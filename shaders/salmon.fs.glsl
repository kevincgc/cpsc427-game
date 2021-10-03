#version 330

// From Vertex Shader
in vec3 vcolor;
in vec2 vpos; // Distance from local origin

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int light_up;

// Output color
layout(location = 0) out vec4 color;
// Fake projection matrix, which maps game world coordinates into OpenGL's [-1, 1] range
// float left = 0.f;
// float top = 0.f;
// float right = window_size_in_game_units.x;
// float bottom = window_size_in_game_units.y;
// float sx = 2.f / (right - left);
// float sy = 2.f / (top - bottom);
// float tx = -(right + left) / (right - left);
// float ty = -(top + bottom) / (top - bottom);
// mat3 projection_2D = {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};


// mat3 transform = {
//     {50., 0., 0.},    // Leftmost column      scale x
//     {0., 50., 0.},    // Middle column        scale y
//     {200., 200., 1.},  // Rightmost column    translate
// };

void main()
{
	// color = vec4(fcolor * vcolor, 1.0);
	color = texture(sampler0, vpos);
	// Salmon mesh is contained in a 1x1 square
	float radius = distance(vec2(0.0), vpos);
	if (light_up == 1 && radius < 0.3)
	{
		// 0.8 is just to make it not too strong
		color.xyz += (0.3 - radius) * 0.8 * vec3(1.0, 1.0, 0.0);
	}
}