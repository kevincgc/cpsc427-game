#version 330

in vec2 uv;
uniform sampler2D image;
uniform float time;
uniform bool flash;
uniform bool used_wall_breaker;
uniform bool used_speed_boost;

layout(location = 0) out vec4 out_color;


void main() {
  out_color = texture(image, uv); // Get the pixel colour from the texture
  if (flash) {
    out_color.xy *= vec2(uv.x + sin(time), uv.y);
  } else if (used_wall_breaker) {
    if (int (time) % 2 != 0) {
      out_color *= vec4(0.9137, 0.0745, 0.0745, 1.0);
    } 
  } else if (used_speed_boost) {
      out_color *= vec4(uv.x + sin(time), abs(2*sin(time + 2)), 0.0, 1.0);
  }
  // out_color = vec4(uv.x, uv.y, 0.0, 0);
}