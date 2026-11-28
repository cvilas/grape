//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

// Splat fragment shader: evaluates the gaussian at each pixel of the quad from splat.vert.
//
// v_offset is the pixel's position in standard deviations along the ellipse axes, so the
// elliptical 2D gaussian becomes the unit circular one: G = exp(-½·r²), where r = |v_offset| (r
// is the Mahalanobis distance from the centre).
//
// The output colour is 'premultiplied' (rgb·alpha) to suit the blending set up in viewer.cpp:
//   framebuffer = this_colour·alpha + (1 - alpha)·framebuffer

#version 450

layout(location = 0) in vec4 v_color;   // rgb and peak opacity of the splat
layout(location = 1) in vec2 v_offset;  // position relative to centre, in standard deviations

layout(location = 0) out vec4 out_color;

const float EXTENT_SQ = 9.0;           // (3σ)². Must match EXTENT in splat.vert
const float MAX_ALPHA = 0.99;          // never fully opaque, as in the reference implementation
const float MIN_ALPHA = 1.0 / 255.0;   // below this, the splat cannot change an 8-bit pixel

void main() {
  const float r2 = dot(v_offset, v_offset);
  if (r2 > EXTENT_SQ) {
    discard;  // corners of the quad, outside the 3σ ellipse
  }
  const float alpha = min(MAX_ALPHA, v_color.a * exp(-0.5 * r2));
  if (alpha < MIN_ALPHA) {
    discard;
  }
  out_color = vec4(v_color.rgb * alpha, alpha);
}
