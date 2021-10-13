#version 330

// Input attributes
in vec3 in_position;
in vec3 in_color;

out vec3 vcolor;
out vec2 vpos;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float time;



void main() {
  // Pass the texture coordinates out to the fragment shader
  float ANIMATION_FRAME_W = 96.f / 844.f;
  float ANIMATION_FRAME_W_PADDING = 20.f / 844.f;
  int NUM_ANIMATION_FRAMES = 3;
  int frame;
  uv = in_uv;
  frame = int(time / 5.f) % NUM_ANIMATION_FRAMES;

  uv.x += ANIMATION_FRAME_W * frame + ANIMATION_FRAME_W_PADDING;
  vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
  gl_Position = vec4(pos.xy, in_position.z, 1.0);
}