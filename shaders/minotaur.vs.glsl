#version 330 

in vec3 in_position;
in vec2 in_uv;
out vec2 uv;

uniform mat3 transform;
uniform mat3 projection;
uniform float time;
uniform int gesture;

// motion:
// enum | gesture | #frames|
// ---------------------------
// 0 -- idle -- 5
// 1 -- walk -- 8
// 2 -- taunt -- 5
// 3 -- attack -- 9
// 4 -- stab -- 5
// 5 -- ready -- 6
// 6 -- turn -- 9
// 7 -- idle -- 3
// 8 -- blink -- 3
// 9 -- die -- 6


const float SHEET_WIDTH = 844.f;
const float SHEET_HEIGHT = 1868;

const float ANIMATION_FRAME_H = 95.f / SHEET_HEIGHT;

const float ANIMATION_FRAME_W = 96.f / SHEET_WIDTH;
const float ANIMATION_FRAME_W_PADDING = 20.f / SHEET_WIDTH;

const float FRAME_SPEED = 13.f;



void main() {
  // Pass the texture coordinates out to the fragment shader
  int num_animation_frames;
  switch (gesture) {
    case 0:
      num_animation_frames = 5;
      break;
    case 1:
      num_animation_frames = 8;
      break;
    case 2:
      num_animation_frames = 5;
      break;
    case 3:
      num_animation_frames = 9;
      break;
    case 4:
      num_animation_frames = 5;
      break;
    case 5:
      num_animation_frames = 6;
      break;
    case 6: 
      num_animation_frames = 9;
      break;
    case 7: 
      num_animation_frames = 3;
      break;
    case 8: 
      num_animation_frames = 3;
      break;
    case 9: 
      num_animation_frames = 6;
      break;
    default:
      num_animation_frames = 9;
  }

  int frame;
  uv = in_uv;
  
  if (gesture == 9 && int(time * FRAME_SPEED) >= 6) { // if death, do not iterate the frames
    frame = 5;
  } else {
    frame = int(time * FRAME_SPEED) % num_animation_frames;
  }
  
  uv.x += ANIMATION_FRAME_W * frame + ANIMATION_FRAME_W_PADDING;
  uv.y += ANIMATION_FRAME_H * gesture;

  vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
  gl_Position = vec4(pos.xy, in_position.z, 1.0);
}