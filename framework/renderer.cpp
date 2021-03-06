// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Renderer
// -----------------------------------------------------------------------------

#define _USE_MATH_DEFINES //pi
#include <math.h>
#include <cmath>
#include "renderer.hpp"
Renderer::Renderer()
  : width_(0)
  , height_(0)
  , colorbuffer_(0, Color(0.0, 0.0, 0.0))
  , filename_("NULL")
  , ppm_(0, 0){}
Renderer::Renderer(unsigned w, unsigned h, std::string const& file)
  : width_(w)
  , height_(h)
  , colorbuffer_(w*h, Color(0.0, 0.0, 0.0))
  , filename_(file)
  , ppm_(width_, height_)
{}
Renderer::Renderer(Renderer const& copy_renderer)
  : width_(copy_renderer.width_)
  , height_(copy_renderer.height_)
  , colorbuffer_(copy_renderer.width_*copy_renderer.height_, 
  Color(0.0, 0.0, 0.0))
  , filename_(copy_renderer.filename_)
  , ppm_(copy_renderer.width_, copy_renderer.height_)
{}

void Renderer::render() {
  const std::size_t checkersize = 20;

  for (unsigned y = 0; y < height_; ++y) {
    for (unsigned x = 0; x < width_; ++x) {
      Pixel p(x,y);
      if ( ((x/checkersize)%2) != ((y/checkersize)%2)) {
        p.color = Color(0.0, 1.0, float(x)/height_);
      } else {
        p.color = Color(1.0, 0.0, float(y)/width_);
      }

      write(p);
    }
  }
  ppm_.save(filename_);
}

void Renderer::render(Scene const& scene) {
 int ray_depth = 3;                      //Tiefe der Strahlen
  for (unsigned y = 0; y < height_; ++y) {
    for (unsigned x = 0; x < width_; ++x) {
      float distance = 0.0;
      float min_distance = std::numeric_limits<float>::max();
			Ray pixel_ray = shootRay(x, y, scene);

      bool pixel_drawn = false;
      Raystructure intersect_struct 
                = scene.composite_ptr_->raystruct_intersect(pixel_ray);

      if (intersect_struct.distance_ > 0 && 
          min_distance > intersect_struct.distance_) {          
							
          pixel_drawn = true;
          min_distance = distance;

					Pixel p(x,y);
          p.color = shade(scene, intersect_struct.material_, intersect_struct);
					write(p);
				} else {
          if (false == pixel_drawn) {
	
            Pixel p(x,y);
            p.color = Color(0.5, 0.5, 0.5);
            write(p);
          }
			  }
		  }
    }
  ppm_.save(filename_);
}

void Renderer::write(Pixel const& p) {
  // flip pixels, because of opengl glDrawPixels
  size_t buf_pos = (width_*p.y + p.x);
  if (buf_pos >= colorbuffer_.size() || (int)buf_pos < 0) {
    std::cerr << "Fatal Error Renderer::write(Pixel p) : "
      << "pixel out of ppm_ : "
      << (int)p.x << "," << (int)p.y
      << std::endl;
  } else {
    colorbuffer_[buf_pos] = p.color;
  }

  ppm_.write(p);
}

Color Renderer::shade (
  Scene const& scene, 
  Material const& material, 
  Raystructure const& raystructure) {
    
  Color shade_color = {0.0,0.0,0.0};
  shade_color = material.color_kd();
  if (scene.lights_.empty()) {
    return shade_color;
  } else {
    Color diffuse = {0.0,0.0,0.0};
    Color specular = {0.0,0.0,0.0};
    Color ambient = {0.0,0.0,0.0};

    bool in_shadow = false;

    for (int i = 0; i < scene.lights_.size(); ++i) {      

      //sum up ambient light:
      ambient.r += scene.lights_[i].intensity_amb_.r;
      ambient.g += scene.lights_[i].intensity_amb_.g;
      ambient.b += scene.lights_[i].intensity_amb_.b;  
   
      /**
        DIFFUSE LIGHTING
        modeled after Lambert's law
        diffuse_intensity = 
            (dot product of surface normal and incident vector) 
          * (brightness of light source)
          * (diffuse coefficient of material)
      */
      
      //respects calculation error, if intersection lies within Shape
      glm::vec3 hit = { 
        (raystructure.intersection_.x + 
          (raystructure.normal_ - raystructure.intersection_).x * 0.000001),
        (raystructure.intersection_.y + 
          (raystructure.normal_ - raystructure.intersection_).y * 0.000001),
        (raystructure.intersection_.z + 
          (raystructure.normal_ - raystructure.intersection_).z * 0.000001)};

      glm::vec3 incidence = glm::normalize(scene.lights_[i].position_ - raystructure.intersection_);
      Ray light_ray = {hit, incidence}; 
/*
      if (raystructure.shape_ptr_ != nullptr) {
        Ray_T light_ray_T = 
          light_ray.transform(raystructure.shape_ptr_->world_transformation_inv());
        Ray light = {glm::vec3(light_ray_T.origin), glm::vec3(light_ray_T.direction)};
        light_ray = light;
      } 
*/      
      Raystructure shadow_point = scene.composite_ptr_->raystruct_intersect(light_ray);
      in_shadow = shadow_point.is_hit_;

      glm::vec3 normal = glm::normalize(raystructure.normal_);
      
      float dotProd_dif = glm::dot(incidence, normal);
 
      diffuse.r += scene.lights_[i].intensity_dif_.r * dotProd_dif/**material.color_kd().r*/;
      diffuse.g += scene.lights_[i].intensity_dif_.g * dotProd_dif/**material.color_kd().g*/;
      diffuse.b += scene.lights_[i].intensity_dif_.b * dotProd_dif/**material.color_kd().b*/;

      diffuse.r = std::fmax(0.0, std::fmin(1.0, diffuse.r));
      diffuse.g = std::fmax(0.0, std::fmin(1.0, diffuse.g));
      diffuse.b = std::fmax(0.0, std::fmin(1.0, diffuse.b));

      /**
        SPECULAR LIGHTING
        diffuse_intensity = 
            (dot product of normalized reflection and spectator vector) 
          ^ (specular coefficient of material)
      */

      float dot_temp = glm::dot(normal, glm::normalize(scene.lights_[i].position_ - raystructure.intersection_));
      glm::vec3 minuend = 
        {(2 * dot_temp * normal.x), (2 * dot_temp * normal.y),(2 * dot_temp * normal.z)};
      glm::vec3 reflection = minuend - glm::normalize(raystructure.intersection_ - scene.lights_[i].position_);
      reflection = glm::normalize(reflection);

      glm::vec3 eyeray_direction = glm::normalize(raystructure.intersection_-scene.camera_.position());
      float dotProd_spec = glm::dot(reflection, eyeray_direction);
      
      specular.r += scene.lights_[i].intensity_dif_.r * material.color_ks().r 
                  * pow(dotProd_spec, material.m());
      specular.g += scene.lights_[i].intensity_dif_.g * material.color_ks().g 
                  * pow(dotProd_spec, material.m());
      specular.b += scene.lights_[i].intensity_dif_.b * material.color_ks().b 
                  * pow(dotProd_spec, material.m());

      specular.r = std::fmax(0.0, std::fmin(1.0, specular.r));
      specular.g = std::fmax(0.0, std::fmin(1.0, specular.g));
      specular.b = std::fmax(0.0, std::fmin(1.0, specular.b));
    }
    
    ambient.r = std::fmax(0.0, std::fmin(1.0, material.color_ka().r * ambient.r));
    ambient.g = std::fmax(0.0, std::fmin(1.0, material.color_ka().g * ambient.g));
    ambient.b = std::fmax(0.0, std::fmin(1.0, material.color_ka().b * ambient.b));

    //only apply coefficient once, since it's the same material:
    diffuse *= material.color_kd(); 

    shade_color = /*diffuse + specular +*/ ambient;

    return shade_color;
  }
}

Ray Renderer::shootRay(int x, int y, Scene const& scene)	{
  glm::vec3 position = scene.camera_.position();
  glm::vec3 dir = glm::normalize(scene.camera_.direction());
  glm::vec3 up = glm::normalize(scene.camera_.up());
  double fov_hor = scene.camera_.horFOV() * (M_PI/180); //radians!
  double ratio = static_cast<double>(width_)
                /static_cast<double>(height_);
  double ratio_inv = static_cast<double>(height_) 
                    /static_cast<double>(width_);
  double fov_ver = ratio * fov_hor; //radians!

  //cross product of dir and up: 
  glm::vec3 right = 
   { (dir.y*up.z - dir.z*up.y),
      (dir.z*up.x - dir.x*up.z),
      (dir.x*up.y - dir.y*up.z)
   };
  
  glm::normalize(right);

  float dist = 0.5 / std::tan(0.5*fov_hor);
  glm::vec3 dist_vec = dir * dist;

  float x_pos = static_cast<float>(x) / static_cast<float>(width_) -0.5;
  float y_pos = ratio_inv * (static_cast<float>(y) / static_cast<float>(height_)) -0.5;
  glm::vec3 pos_on_plane = right*x_pos + up*y_pos;

  glm::vec3 image_point = position + dist_vec + pos_on_plane;
  glm::vec3 direc_shot = glm::normalize(image_point - position);
  
  return Ray(position, direc_shot);
}

unsigned Renderer::width(){
  return width_;
}
unsigned Renderer::height(){
  return height_;
}
