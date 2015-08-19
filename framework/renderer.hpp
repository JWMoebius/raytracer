// -----------------------------------------------------------------------------
// Copyright  : (C) 2014 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Renderer
// -----------------------------------------------------------------------------

#ifndef BUW_RENDERER_HPP
#define BUW_RENDERER_HPP

#include "color.hpp"
#include "pixel.hpp"
#include "ppmwriter.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include <string>
#include <glm/glm.hpp>

class Renderer
{
public:
  Renderer();
  Renderer(unsigned w, unsigned h, std::string const& file);
  Renderer(Renderer const& copy_renderer);
  void render();
  void render(Scene const& render_scene);
  void write(Pixel const& p);
  Ray shootRay(int x, int y, Scene const& render_scene);
  inline std::vector<Color> const& colorbuffer() const
  {
    return colorbuffer_;
  }
	unsigned width();
	unsigned height();
	
	
private:
  unsigned width_;
  unsigned height_;
  std::vector<Color> colorbuffer_;
  std::string filename_;
  PpmWriter ppm_;
};

#endif // #ifndef BUW_RENDERER_HPP
