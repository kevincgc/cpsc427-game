#version 330 core

layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat3 projection;
uniform mat3 transform;

void main()
{
    vec3 pos = projection * transform * vec3(vertex.xy, 1.0);
    gl_Position = vec4(pos, 1.0);
    TexCoords = vertex.zw;
}  


//Referece: https://learnopengl.com/In-Practice/Text-Rendering