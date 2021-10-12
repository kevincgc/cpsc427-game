#version 330

in vec2 uv;
uniform sampler2D image;
uniform float time;

layout(location = 0) out vec4 out_color;


void main() {
  out_color = texture(image, uv); // Get the pixel colour from the texture
  // out_color.xy *= vec2(uv.x * sin(time / 100), uv.y);
  // out_color = vec4(uv.x, uv.y, 0.0, 0);
}