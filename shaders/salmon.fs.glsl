// #version 330

// // From Vertex Shader
// in vec3 vcolor;
// in vec2 vpos; // Distance from local origin

// // Application data
// uniform sampler2D sampler0;
// uniform vec3 fcolor;
// uniform int light_up;

// // Output color
// layout(location = 0) out vec4 color;

// void main()
// {
// 	// color = vec4(fcolor * vcolor, 1.0);
// 	color = texture(sampler0 , vpos);
// 	color.xyz *= vec3(0.549, 0.0, 1.0);	// blend color onto the texture
// 	// Salmon mesh is contained in a 1x1 square
// 	float radius = distance(vec2(0.0), vpos);
// 	if (light_up == 1 && radius < 0.3)
// 	{
// 		// 0.8 is just to make it not too strong
// 		color.xyz += (0.3 - radius) * 0.8 * vec3(1.0, 1.0, 0.0);
// 	}
// }
#version 330

in vec2 uv;
uniform sampler2D image;
uniform float time;

layout(location = 0) out vec4 out_color;


void main() {
  out_color = texture(image, uv); // Get the pixel colour from the texture
  out_color.xy *= vec2(uv.x * sin(time / 10), uv.y);
  // out_color = vec4(uv.x, uv.y, 0.0, 0);
}