#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
in vec3 Normal;
in vec3 Tangent;

// Passed to fragment shader
out vec2 texcoord;
out vec3 fragPos;
out vec3 TangentLightPosition;
out vec3 TangentViewPosition;
out vec3 TangentFragPosition;


// Application data
uniform mat3 transform;
uniform mat3 projection;




void main()
{

	
	
	vec3 pos = projection * transform *  mat3(1.0f) * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);


	mat3 model = mat3(0.1f);
	// passing to fragment shader
	fragPos = vec3 (model * in_position);
	texcoord = in_texcoord;

	mat3 normalMatrix =model;
	vec3 T = normalize(normalMatrix * Tangent);
	vec3 N = normalize(normalMatrix * Normal);
	T = normalize(T - dot (T,N) * N);
	vec3 B = cross (N,T);

	mat3 TBN = transpose(mat3(T,B,N));

	vec3 lightPosition = vec3 (0.f,0.f,3.f);
	vec3 viewPosition = vec3 (0.f,0.f,12.f);

	TangentLightPosition = TBN * lightPosition;
	TangentViewPosition = TBN * viewPosition;
	TangentFragPosition = TBN * fragPos;
}


// reference : https://learnopengl.com/Advanced-Lighting/Normal-Mapping