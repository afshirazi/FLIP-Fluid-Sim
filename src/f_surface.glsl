#version 330 core

in vec3 norm;

out vec4 FragColor;

uniform vec3 vertColor;

//uniform vec3 lightPos;
const vec3 light = normalize(vec3(0.3, 0.3, 0.8));

void main()
{
	vec3 ambient = vec3(0.4, 0.4, 0.4);
	vec3 normal = normalize(norm);
	float diff = max(dot(norm, light), 0.0);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

	vec3 color = (ambient + diffuse) * vertColor;

	FragColor = vec4(color, 1.0);
}