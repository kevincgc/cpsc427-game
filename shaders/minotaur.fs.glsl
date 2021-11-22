#version 330

in vec2 uv;
uniform sampler2D image;
uniform float time;
uniform bool flash;

layout(location = 0) out vec4 out_color;


void main() {
  out_color = texture(image, uv); // Get the pixel colour from the texture
  if (flash) {
    out_color.xy *= vec2(uv.x + sin(time), uv.y);
  }
  // out_color = vec4(uv.x, uv.y, 0.0, 0);
}