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



// void main()
// {
// 	vec3 pos;

	// if (dead) {
	// 	texcoord = vec2(clamp(in_texcoord.x + cos(in_texcoord.y * 10 + 10 * time) / 2, 0.f, 1.f ), 
	// 					clamp(in_texcoord.y + sin(in_texcoord.x * 10 + 10 * time) / 2, 0.f, 1.f ));

	// 	vec2 distorted =  vec2(clamp(in_position.x + cos(in_position.y * 10 + 10 * time) / 2, 0.f, 1.f ), 
	// 					clamp(in_position.y + sin(in_position.x * 10 + 10 * time) / 2, 0.f, 1.f ));

	// 	vec2 distorted = in_position.xy + getDirection(in_position.xy) * sin(time * 10) * 0.1;
	// 	pos = projection * transform * vec3(distorted, 1.0);
	// } else {
	// 	texcoord = in_texcoord;
	// 	pos = projection * transform * vec3(in_position.xy, 1.0);
	// }
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
uniform bool dead;
uniform float time;


const vec2 TOP_LEFT = vec2 (0.25, 0.75);
const vec2 TOP_RIGHT = vec2 (0.75, 0.75);
const vec2 BOTTOM_LEFT = vec2 (0.25, 0.25);
const vec2 BOTTOM_RIGHT = vec2 (0.75, 0.25);

vec2 rand(vec2 v)
{
    return vec2(fract(sin(dot(v.xy ,vec2(12.9898,78.233))) * 43758.5453),
                fract(cos(dot(v.yx ,vec2(31.6245,22.723))) * 63412.9227)) - 0.5;
}

vec2 getDirection(vec2 uv) {
	vec2 CENTER = vec2(0, 0);
	vec2 direction = CENTER - uv;
	return direction/10;
}



vec3 explode(vec3 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time * 2) + 1.0) / 2.0) * magnitude; 
    return position + direction;
} 


// vec2 randomEffect()
// {
// 	select = int (time) % 3;
// 	if (select == 0) return 
// }
void main()
{
	vec2 distorted = in_position.xy;
	if (dead) {
		// out of shape
		// distorted = in_position.xy + getDirection(rand(in_position.xy)) * sin(time * 10) * 0.1;

		// random break
		// distorted = in_position.xy + getDirection(rand(in_position.xy))  * time * 0.01;
		// distorted = distorted + getDirection(rand(in_position.xy))  * time * 0.02;

		// explosion
		distorted = explode(in_position, vec3((in_position.xy+rand(in_position.xy))*0.5 , 0)).xy;
		// clapse
		// distorted +=   getDirection(distorted) * time;
		// distorted.x += sin(distorted.x + distorted.y + time) * 0.5; 
		// distorted.y += ( - distorted.y ) * 0.1*time;
	}
	// vpos = in_position.xy; // local coordinated before transform
	vpos = distorted;
	vcolor = in_color;
	// vec3 pos = projection * transform * vec3(in_position.xy, 1.0); 
	vec3 pos = projection * transform * vec3(distorted, 1.0); // why not simply *in_position.xyz ?
	
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}