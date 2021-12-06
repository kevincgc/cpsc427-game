#version 330

// From vertex shader
in vec2 texcoord;
in vec3 fragPos;
in vec3 TangentLightPosition;
in vec3 TangentViewPosition;
in vec3 TangentFragPosition;


// Application data
uniform sampler2D sampler0;
uniform sampler2D normalMap;


uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	
	// obtain normal from normal map
	vec3 normal = texture (normalMap, texcoord).rgb;
	// transform normal vector
	normal = normalize (normal * 2.0 - 1.0);
	//  color
	vec3 original_color = texture(sampler0, texcoord).rgb;
	// ambient
	vec3 ambient = 0.6* original_color;
	// diffuse_color
	vec3 lightDirection = normalize(TangentLightPosition - TangentFragPosition);
	float diffuse_value = max(dot(lightDirection, normal), 0.0);
	vec3 diffuse = 1.3 *  diffuse_value * original_color;
	// specular
	vec3 viewDirection = normalize(TangentViewPosition - TangentFragPosition);
	vec3 reflectDir = reflect(-lightDirection, normal);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specular_value = pow(max(dot(normal, halfwayDir),0.0), 32.0);

	vec3 specular = vec3(0.1) * specular_value;

	color = vec4(ambient + diffuse + specular , 1.0);
}

// reference : https://learnopengl.com/Advanced-Lighting/Normal-Mapping;