// #version 330

// // Input attributes
// in vec3 in_position;
// in vec3 in_color;

// out vec3 vcolor;
// out vec2 vpos;

// // Application data
// uniform mat3 transform;
// uniform mat3 projection;

// void main()
// {
// 	vpos = in_position.xy; // local coordinated before transform
// 	vcolor = in_color;
// 	vec3 pos = projection * transform * vec3(in_position.xy, 1.0); // why not simply *in_position.xyz ?
// 	gl_Position = vec4(pos.xy, in_position.z, 1.0);
// }
#version 330 

in vec3 in_position;
in vec2 in_uv;
out vec2 uv;

uniform mat3 transform;
uniform mat3 projection;
uniform int in_frame;
uniform float time;



void main() {
  // Pass the texture coordinates out to the fragment shader
  float ANIMATION_FRAME_W = 96.f / 844.f;
  int NUM_ANIMATION_FRAMES = 3;
  int frame = 0;

  uv = in_uv;
  if (sin(time/4)<0.1 && sin(time/4)>-0.1) {
    frame = (frame + 1) % NUM_ANIMATION_FRAMES;
  }

  uv.x += ANIMATION_FRAME_W * frame;
  vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
  gl_Position = vec4(pos.xy, in_position.z, 1.0);
}