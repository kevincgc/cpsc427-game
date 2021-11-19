#version 330

uniform sampler2D screen_texture;
uniform sampler2D pre_screen;
uniform float time;


in vec2 texcoord;

layout(location = 0) out vec4 color;


//  https://www.shadertoy.com/view/Ms3SRf
void main()
{
	float w = 0.5;
	float g = 1.0;
    vec4 gamma = vec4 (g, g, g, 1.0);
    
    vec4 color0 = pow(texture(screen_texture, texcoord), gamma);
    // // vec4 color1 = texture(screen_texture, texcoord);
    vec4 color1 = vec4(0.949, 0.949, 0.9529, 1.0);
    float duration = 10;
    

	// float t = mod(time, duration) /  duration;
	float t = time / duration;
		
	float correction = mix(w, -w, t);
		
	float choose = smoothstep(t  - w, t + w, texcoord.x + correction); // clamped ramp
		
	color = mix(color0, color1, choose); 
	
    
}

