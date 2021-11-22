#version 330

in vec2 uv;
uniform sampler2D image;
uniform float time;
uniform bool flash;
uniform bool used_wall_breaker;

layout(location = 0) out vec4 out_color;


void main() {
  out_color = texture(image, uv); // Get the pixel colour from the texture
  if (flash || used_wall_breaker) {
    out_color.xy *= vec2(uv.x + sin(time), uv.y);
  } else if (used_wall_breaker) {
  // TODO get help with flashing red or smth
    //out_color.xy *= vec2(uv.x + sin(time), uv.y);
    out_color = vec4(1.0, 0.0, 0.0, 0);
  }
  // out_color = vec4(uv.x, uv.y, 0.0, 0);
}