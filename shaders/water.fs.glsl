#version 330

uniform sampler2D screen_texture;
uniform sampler2D pre_screen;
uniform float time;
uniform bool initial_game;
uniform bool endGame;


in vec2 texcoord;

layout(location = 0) out vec4 color;




//  https://www.shadertoy.com/view/Ms3SRf

vec2 iResolution = vec2(1200., 800.); 
vec2 rand1(vec2 v)
{
    return vec2(fract(sin(dot(v.xy ,vec2(12.9898,78.233))) * 43758.5453),
                fract(cos(dot(v.yx ,vec2(31.6245,22.723))) * 63412.9227)) - 0.5;
}


float rand(vec2 n) {
    return fract(sin(cos(dot(n, vec2(12.9898,12.1414)))) * 83758.5453);
}

float noise(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

float fbm(vec2 n) {
    float total = 0.0, amplitude = 1.0;
    for (int i = 0; i <5; i++) {
        total += noise(n) * amplitude;
        n += n*1.7;
        amplitude *= 0.47;
    }
    return total;
}
// fire shader attribute to https://www.shadertoy.com/view/4ttGWM
vec4 fire( ) {
	vec2 fragCoord = texcoord * iResolution;
    const vec3 c1 = vec3(0.5, 0.0, 0.1);
    const vec3 c2 = vec3(0.9, 0.1, 0.0);
    const vec3 c3 = vec3(0.2, 0.1, 0.7);
    const vec3 c4 = vec3(1.0, 0.9, 0.1);
    const vec3 c5 = vec3(0.1);
    const vec3 c6 = vec3(0.9);
	float iTime = time;
    vec2 speed = vec2(0.1, 0.9);
    float shift = 1.327+sin(iTime*2.0)/2.4;
    float alpha = 1.0;

	float dist = 3.5-sin(iTime*0.4)/1.89;

    vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 p = fragCoord.xy * dist / iResolution.xx;
    p += sin(p.yx*4.0+vec2(.2,-.3)*iTime)*0.04;
    p += sin(p.yx*8.0+vec2(.6,+.1)*iTime)*0.01;

    p.x -= iTime/1.1;
    float q = fbm(p - iTime * 0.3+1.0*sin(iTime+0.5)/2.0);
    float qb = fbm(p - iTime * 0.4+0.1*cos(iTime)/2.0);
    float q2 = fbm(p - iTime * 0.44 - 5.0*cos(iTime)/2.0) - 6.0;
    float q3 = fbm(p - iTime * 0.9 - 10.0*cos(iTime)/15.0)-4.0;
    float q4 = fbm(p - iTime * 1.4 - 20.0*sin(iTime)/14.0)+2.0;
    q = (q + qb - .4 * q2 -2.0*q3  + .6*q4)/3.8;
    vec2 r = vec2(fbm(p + q /2.0 + iTime * speed.x - p.x - p.y), fbm(p + q - iTime * speed.y));
    vec3 c = mix(c1, c2, fbm(p + r)) + mix(c3, c4, r.x) - mix(c5, c6, r.y);
    vec3 color = vec3(1.0/(pow(c+1.61,vec3(4.0))) * cos(shift * fragCoord.y / iResolution.y));

    color=vec3(1.0,.2,.05)/(pow((r.y+r.y)* max(.0,p.y)+0.1, 4.0));;
    color += (texture(screen_texture,uv).xyz*0.01*pow((r.y+r.y)*.65,5.0)+0.055)*mix( vec3(.9,.4,.3),vec3(.7,.5,.2), uv.y);
    color = color/(1.0+max(vec3(0),color));
    return vec4(color.x, color.y, color.z, alpha);
}

vec4 explode(vec2 p, float time,  float blast)
{
	float t = clamp(time-2.0, 0.0, 4.0);

	p = floor(p / 5.0) + vec2(18.0, 16.0);

    vec2 r = rand1(p);
	vec2 delta = 2.0*r * vec2(0.4*(3.0-t), 1.0) - vec2(0.0, t*0.9);

	p -= blast * delta * t;

	if (clamp(p.x, 0.0, 37.0) != p.x || clamp(p.y, 0.0, 32.0) != p.y) return fire();
	// if (clamp(p.x, 0.0, 37.0) != p.x || clamp(p.y, 0.0, 32.0) != p.y) return vec4(0.0, 0.0, 0.0, 1.0);
	p.y= 32.0-p.y;

	return smoothstep(0.0, 0.2, time) * smoothstep(2.0, 1.2, t) * texture(screen_texture, texcoord);
}

#define dur 5.0
#define spd 1.0

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
		col = mix(col, explode(p, mod(t - 0.1, dur), blast), 0.3);
		col = mix(col, explode(p, mod(t - 0.2, dur), blast), 0.2);
		col = mix(col, explode(p, mod(t - 0.3, dur), blast), 0.1);
	} else {
		col = texture(screen_texture, texcoord);
	}

	color = col;
}
