
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
uniform int randomInt;

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
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + direction;
} 


float BarrelPower = 0.5;

// Given a vec2 in [-1,+1], generate a texture coord in [0,+1]
vec2 Distort(vec2 p)
{
    float theta  = atan(p.y, p.x);
    float radius = length(p);
    radius = pow(radius, BarrelPower);
    p.x = radius * cos(theta);
    p.y = radius * sin(theta);
    return 0.5 * (p + 1.0);
}

void main()
{
	vec2 distorted = in_position.xy;
	if (dead) {
		if (randomInt == 0) {
			// clapse
			distorted = mix(distorted, (Distort(in_position.xy * sin(time / 100)) - 0.5) * 6, (sin(time)+1)/2);
		}
		
		else if (randomInt == 1) {
		// random break
			distorted = in_position.xy + getDirection(rand(in_position.xy))  * time * 0.1;
			distorted = distorted + getDirection(rand(in_position.xy))  * time * 0.2;
		} 
		else if (randomInt == 2) {
			// out of shape
			distorted = in_position.xy + getDirection(rand(in_position.yx)) * sin(time);
		}
		else {
		// explosion
			distorted = explode(in_position, vec3((in_position.xy+rand(in_position.xy))*0.5 , 0)).xy;
		}
		
	}
	// vpos = in_position.xy; // local coordinated before transform
	vpos = distorted;
	vcolor = in_color;
	// vec3 pos = projection * transform * vec3(in_position.xy, 1.0); 
	vec3 pos = projection * transform * vec3(distorted, 1.0); // why not simply *in_position.xyz ?
	
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}