#version 330 core
out vec4 color;
in vec3 fragNor;
void main()
{
	vec3 normal = normalize(fragNor);
	color.rgb = normal;
	color.a = 1.0;
}
