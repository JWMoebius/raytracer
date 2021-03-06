#include "sdfloader.hpp"
#include "scene.hpp"
#include <regex>
#include <memory>
#include <vector>
#include <map>
//
Scene SDFloader::load(std::string const& filename) {
  //std::ifstream file{"/home/moka3156/Dokumente/raytracer/framework/res/material_data.sdf"};
  std::ifstream file{filename};
  std::vector<std::shared_ptr<Shape>> shapes_;	
	std::map<std::string, Material> materials_;
			 
	std::string out_rendcam_name;
	std::string out_file_name;
	unsigned out_x_res;//USED INT AS RESOLUTION
	unsigned out_y_res;

	std::string out_camera_name;
	double out_fov_x;
	bool read_camera = false;	//Test for correct declaration
	glm::vec3 out_eye_pos;
	glm::vec3 out_dir;
	glm::vec3 out_up;

  std::string composite_name;
  std::unordered_map<std::string,  std::shared_ptr<Shape>> composite_;
  std::unordered_map<std::string, std::shared_ptr<Shape>> map_children;

  std::string transform_name;
  std::string transform_kind;
  unsigned trans_x, trans_y, trans_z;

  std::vector<Light> lights_;
  if (!file) {
    std::cout << "file not open \n";
  }

  std::string curr_line;
  while (std::getline(file, curr_line)) {
    std::cout << curr_line << "\n";
    std::stringstream test(curr_line);
		std::string curr_word;
		while(test >> curr_word)
		{
			//std::cout << "Lese neue Zeile:"<< std::endl; //TESTZEILE
			if (curr_word =="define")
			{
				//std::cout << "Define what?"; 
				test >> curr_word;
			
				if (curr_word =="material")
				{
					//std::cout << " MATERIAL " << std::endl; //TEST
					test >> curr_word;
					std::string material_name = curr_word;
					float ka_f;
					test >> ka_f;
					float ka_s;
					test >> ka_s;
					float ka_t;
					test >> ka_t;				
					float kd_f;
					test >> kd_f;
					float kd_s;
					test >> kd_s;
					float kd_t;
					test >> kd_t;				
					float ks_f;
					test >> ks_f;
					float ks_s;
					test >> ks_s;
					float ks_t;
					test >> ks_t;									
					float exp_m;
					test >> exp_m;	
					
					Material temp_mat
						{material_name, Color{ka_f,ka_s,ka_t}, Color{kd_f,kd_s,kd_t},Color{ks_f,ks_s,ks_t}, exp_m};
					materials_.insert(std::make_pair(material_name,temp_mat));
					//std::cout << "Habe "<< temp_mat.name()<< " erschaffen." << std::endl;
					auto shared_material = std::make_shared<Material>(Material
						{material_name, Color{ka_f,ka_s,ka_t}, Color{kd_f,kd_s,kd_t},Color{ks_f,ks_s,ks_t}, exp_m});
				}
				if (curr_word =="shape")
				{
						test >> curr_word;
					if (curr_word =="sphere"){
						//std::cout << " SPHERE " << std::endl; //TESTZEILE
						test >> curr_word;
						std::string sphere_name = curr_word;
						double center_x;
						test >> center_x;
						double center_y;
						test >> center_y;
						double center_z;
						test >> center_z;				
						double radius;
						test >> radius;		
						std::string sphmaterial_name;
						test >> sphmaterial_name;
		

						std::map<std::string, Material>::iterator it;
						it = materials_.find(sphmaterial_name);
						Sphere temp_sphere
							{it->second, sphere_name, glm::vec3{center_x,center_y,center_z},radius};
							//std::cout << "Habe "<< temp_sphere.name()<< " erschaffen." << std::endl;//TESTZEILE
						auto shared_sphere = std::make_shared<Sphere>(Sphere
							{it->second, sphere_name, glm::vec3{center_x,center_y,center_z},radius});
						shapes_.push_back(shared_sphere);
						map_children.insert(std::make_pair(sphere_name, shared_sphere));
					}
					
					if (curr_word =="box"){
						//std::cout << " BOX " << std::endl; //TESTZEILE
						test >> curr_word;
						std::string box_name = curr_word;
						double min_x;
						test >> min_x;
						double min_y;
						test >> min_y;
						double min_z;
						test >> min_z;
						double max_x;
						test >> max_x;
						double max_y;
						test >> max_y;
						double max_z;
						test >> max_z;						
						std::string boxmaterial_name;
						test >> boxmaterial_name;

						std::map<std::string, Material>::iterator it;
						it = materials_.find(boxmaterial_name);
						Box temp_box
							{it->second, box_name, glm::vec3{min_x, min_y, min_z}, 
																		 glm::vec3{max_x, max_y, max_z}};
						//std::cout << "Habe "<< temp_box.name()<< " erschaffen." << std::endl;//TESTZEILE
						auto shared_box = std::make_shared<Box>(Box
							{it->second, box_name, glm::vec3{min_x, min_y, min_z}, 
																		 glm::vec3{max_x, max_y, max_z}});
						shapes_.push_back(shared_box); //push Pointer statt Box	
						map_children.insert(std::make_pair(box_name, shared_box)); //Add to 
						//std::cout <<"NUMBER OF ELEMENTS:" << shapes_.size() << std::endl;									
					}
					if (curr_word =="composite"){
  						std::vector<std::string> parsed_children;
            std::unordered_map<std::string, std::shared_ptr<Shape>> copy_temp;	
						//std::cout << " COMPOSITE " << std::endl; //TESTZEILE
						test >> curr_word;
						  //std::cout<<"NAME des compy: " << curr_word << " " << std::endl;
						composite_name = curr_word;
						std::string temp;
						test >> temp;
						std::string previous_string;
						while (false == temp.empty() && temp != previous_string) {
							previous_string = temp;
							parsed_children.push_back(temp);	
						  //std::cout<<"Children: " << temp << " " << std::endl;
						  test >> temp;  
					  }
					  std::string shape_name;
					  for (auto const& it : parsed_children) {
              auto it_child = map_children.find((it));
              shape_name = it_child->second->name();				
						  copy_temp.insert(std::make_pair(
						    it_child->second->name(), 
						    it_child->second));
						}
		        Material temp_mat
						{"Composite", Color{0,0,0}, Color{0,0,0},Color{0,0,0}, 0};
						Composite temp_composite
							{temp_mat, composite_name, copy_temp};
						//std::cout << "Habe "<< temp_composite.name()<< " erschaffen." << std::endl;//TESTZEILE
						auto shared_composite = std::make_shared<Composite>(temp_composite);
						shapes_.push_back(shared_composite); //	
						// Add to Componenten Database
            map_children.insert(std::make_pair(composite_name, shared_composite));
            composite_.insert(std::make_pair(composite_name, shared_composite));

						//std::cout <<"NUMBER OF ELEMENTS:" << shapes_.size() << std::endl;									
					}
				}	
				if(curr_word== "light"){
					test >> curr_word;
					std::string light_name = curr_word;
					double pos_x;
					test >> pos_x;
					double pos_y;
					test >> pos_y;		
					double pos_z;
					test >> pos_z;
					float int_amb_r;
					test >> int_amb_r;
					float int_amb_b;
					test >> int_amb_b;		
					float int_amb_g;
					test >> int_amb_g;
					float int_dif_r;
					test >> int_dif_r;
					float int_dif_b;
					test >> int_dif_b;		
					float int_dif_g;
					test >> int_dif_g;
					Light temp_light
							{light_name, glm::vec3{pos_x, pos_y, pos_z}, 
							 Color{int_amb_r, int_amb_b, int_amb_g}, 
							 Color{int_dif_r,int_dif_b,int_dif_g}};
					//std::cout << "Habe "<< temp_light.name_<< " erschaffen." << std::endl;
					lights_.push_back(temp_light);
				}
			
					if(curr_word == "camera"){ 
					//std::cout << " CAMERA " << std::endl; //TESTZEILE
					test >> curr_word;
					out_camera_name = curr_word;
					test >> out_fov_x;					
					test >> out_eye_pos.x;
					test >> out_eye_pos.y;
					test >> out_eye_pos.z;
					test >> out_dir.x;
					test >> out_dir.y;
					test >> out_dir.z;
					test >> out_up.x;
					test >> out_up.y;
					test >> out_up.z;
					read_camera = true;
				}
			}

			if (curr_word == "transform") {
				//test >> curr_word;
				test >> transform_name;
				test >> transform_kind; 
				test >> trans_x;
				test >> trans_y;
				test >> trans_z;

				auto sh = map_children.find(transform_name);
				if ("translate" == transform_kind) {
					glm::vec3 v{trans_x, trans_y, trans_z};
					sh->second->translate(v);
				}
			}			

			if(curr_word == "render") {

				//std::cout << " RENDER " << std::endl; //TESTZEILE
				test >> curr_word;
				out_rendcam_name = curr_word;
				test >> out_file_name;
				test >> out_x_res;
				test >> out_y_res;

				Camera cam(
		      out_camera_name, 
		      out_fov_x, 
		      out_eye_pos, 
		      out_dir, 
		      out_up);
		    Material root_mat
						{"Composite", Color{0,0,0}, Color{0,0,0},Color{0,0,0}, 0};
				Composite root{root_mat, "root", composite_, map_children};
				std::shared_ptr<Composite> shared_composite = 
				    std::make_shared<Composite>(root);
				Renderer rend{out_x_res, out_y_res, out_file_name};
				rend.render(Scene(materials_, shapes_, lights_, shared_composite, cam));
			}
			if(curr_word== "#"){
			}		

	
		}	
/*	  std::cout << "Materialmenge: " << materials_.size();
	  for (auto map : materials_){ 
	    std::cout << map.second << std::endl;
	  	  std::cout << "Shapes: " << shapes_.size();
	  }
	  for (auto it : shapes_){ 
	    std::cout << "Name: " << it->name() << "      Material: " << it->material() << std::endl;
    }*/	
  }
  
	if (read_camera) {
		Camera camera_scene(
      out_camera_name, 
      out_fov_x, 
      out_eye_pos, 
      out_dir, 
      out_up);
    Material root_mat
						{"Composite", Color{0,0,0}, Color{0,0,0},Color{0,0,0}, 0};
    Composite root{root_mat, "root", composite_, map_children};
		std::shared_ptr<Composite> shared_composite = 
				    std::make_shared<Composite>(root);
		Scene read_scene{materials_, shapes_, lights_, shared_composite, camera_scene};
		return read_scene;	
	}
	Material root_mat
						{"Composite", Color{0,0,0}, Color{0,0,0},Color{0,0,0}, 0};
  Composite root{root_mat, "root", composite_, map_children};
	std::shared_ptr<Composite> shared_composite = 
				    std::make_shared<Composite>(root);
	Scene default_read{materials_, shapes_, lights_, shared_composite, 
	  Camera("Default", 4.0, {0,0,0},{0,0,-1.0},{0,1.0,0})};
	return default_read;
}
