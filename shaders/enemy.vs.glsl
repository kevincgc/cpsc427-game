// #version 330

// // Input attributes
// in vec3 in_position;
// in vec2 in_color;

// // Passed to fragment shader
// out vec2 texcoord;
// // out vec3 vcolor;
// // out vec2 vpos;
// Application data
// uniform mat3 transform;
// uniform mat3 projection;
// uniform bool dead;
// uniform float time;

// vec2 rand(vec2 v)
// {
//     return vec2(fract(sin(dot(v.xy ,vec2(12.9898,78.233))) * 43758.5453),
//                 fract(cos(dot(v.yx ,vec2(31.6245,22.723))) * 63412.9227)) - 0.5;
// }

// vec2 getDirection(vec2 uv) {
// 	vec2 CENTER = rand(uv);
// 	vec2 direction = CENTER - uv;
// 	return -direction / 2.0;
// }

// void main()
// {
// 	vec3 pos;

// 	// if (dead) {
// 	// 	// texcoord = vec2(clamp(in_texcoord.x + cos(in_texcoord.y * 10 + 10 * time) / 2, 0.f, 1.f ), 
// 	// 	// 				clamp(in_texcoord.y + sin(in_texcoord.x * 10 + 10 * time) / 2, 0.f, 1.f ));

// 	// 	// vec2 distorted =  vec2(clamp(in_position.x + cos(in_position.y * 10 + 10 * time) / 2, 0.f, 1.f ), 
// 	// 	// 				clamp(in_position.y + sin(in_position.x * 10 + 10 * time) / 2, 0.f, 1.f ));

// 	// 	vec2 distorted = in_position.xy + getDirection(in_position.xy) * sin(time * 10) * 0.1;
// 	// 	pos = projection * transform * vec3(distorted, 1.0);
// 	// } else {
// 	// 	// texcoord = in_texcoord;
// 	// 	pos = projection * transform * vec3(in_position.xy, 1.0);
// 	// }
// 	texcoord = in_texcoord;
// 	// vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
// 	gl_Position = vec4(pos.xy, in_position.z, 1.0);
// }

#version 330

// Input attributes
in vec3 in_position;
in vec3 in_color;

out vec3 vcolor;
out vec2 vpos;

// Application data
uniform mat3 transform;
uniform mat3 projection;

void main()
{
	vpos = in_position.xy; // local coordinated before transform
	vcolor = in_color;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0); // why not simply *in_position.xyz ?
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}