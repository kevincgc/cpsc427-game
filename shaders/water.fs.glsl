#version 330

uniform sampler2D screen_texture;
uniform sampler2D pre_screen;
uniform float time;
uniform bool initial_game;


in vec2 texcoord;

layout(location = 0) out vec4 color;




//  https://www.shadertoy.com/view/Ms3SRf
// void main()
// {
// 	float w = 0.5;
// 	float g = 1.0;
//     vec4 gamma = vec4 (g, g, g, 1.0);
    
//     vec4 color0 = pow(texture(screen_texture, texcoord), gamma);
//     // // vec4 color1 = texture(screen_texture, texcoord);
//     vec4 color1 = vec4(0.949, 0.949, 0.9529, 1.0);
//     float duration = 10;
    

// 	// float t = mod(time, duration) /  duration;
// 	float t = time / duration;
		
// 	float correction = mix(w, -w, t);
		
// 	float choose = smoothstep(t  - w, t + w, texcoord.x + correction); // clamped ramp
		
// 	color = mix(color0, color1, choose); 
// }

// void main() {
// 	color = texture(screen_texture,texcoord);
// }

vec2 rand(vec2 v)
{
    return vec2(fract(sin(dot(v.xy ,vec2(12.9898,78.233))) * 43758.5453),
                fract(cos(dot(v.yx ,vec2(31.6245,22.723))) * 63412.9227)) - 0.5;
}


vec4 explode(vec2 p, float time,  float blast)
{
	float t = clamp(time-2.0, 0.0, 4.0);
    
	p = floor(p / 5.0) + vec2(18.0, 16.0);
	
    vec2 r = rand(p);
	vec2 delta = 2.0*r * vec2(0.4*(3.0-t), 1.0) - vec2(0.0, t*0.9);
    
	p -= blast * delta * t;
    
	// if (clamp(p.x, 0.0, 37.0) != p.x || clamp(p.y, 0.0, 32.0) != p.y) return texture(screen_texture, texcoord);
	if (clamp(p.x, 0.0, 37.0) != p.x || clamp(p.y, 0.0, 32.0) != p.y) return vec4(0.0, 0.0, 0.0, 1.0);
	p.y= 32.0-p.y;
    
	return smoothstep(0.0, 0.2, time) * smoothstep(2.0, 1.2, t) * texture(screen_texture, texcoord);
}

#define dur 5.0
#define spd 1.0




vec2 iResolution = vec2(1200., 800.); 
// void mainImage( out vec4 fragColor, in vec2 fragCoord )
void main()
{
    float blastBarPos = iResolution.y - 8.0;
    float timeBarPos = 12.0;
    
    vec2 fragCoord = texcoord * iResolution;
	vec2 p = fragCoord.xy - iResolution.xy * 0.5;
	// vec2 p = texcoord;
    
    float blast = 100.0;
    float t = spd * time + 2.2;
    vec4 col;
	if (t < dur -1 && !initial_game) {
		col = explode(p, mod(t, dur), blast);
		col = mix(col, explode(p, mod(t - 0.05, dur), blast), 0.3);
		col = mix(col, explode(p, mod(t - 0.10, dur), blast), 0.2);
		col = mix(col, explode(p, mod(t - 0.15, dur), blast), 0.1);
	} else {
		col = texture(screen_texture, texcoord);
	}

	color = col;
}
