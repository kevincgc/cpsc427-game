#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform bool initial_game;
uniform bool endGame;


in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE THE WATER WAVE DISTORTION HERE (you may want to try sin/cos)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return uv;
}

vec4 color_shift(vec4 in_color) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE THE COLOR SHIFTING HERE (you may want to make it blue-ish)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return in_color;
}

vec4 fade_color(vec4 in_color) 
{
	return in_color;
}

void main()
{
    float blastBarPos = iResolution.y - 8.0;
    float timeBarPos = 12.0;
    
    vec2 fragCoord = texcoord * iResolution;
	vec2 p = fragCoord.xy - iResolution.xy * 0.5;
    
    float blast = 100.0;
    float t = spd * time + 2.2;
    vec4 col;
	if (!initial_game && endGame) {
		col = explode(p, mod(t, dur), blast);
		col = mix(col, explode(p, mod(t - 0.05, dur), blast), 0.3);
		col = mix(col, explode(p, mod(t - 0.10, dur), blast), 0.2);
		col = mix(col, explode(p, mod(t - 0.15, dur), blast), 0.1);
	} else {
		col = texture(screen_texture, texcoord);
	}

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
    color = fade_color(color);
}