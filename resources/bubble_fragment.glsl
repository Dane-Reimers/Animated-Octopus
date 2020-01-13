#version 330 core
in vec3 fragNor;
out vec4 color;
vec3 normal;
void main()
{
	normal = normalize(fragNor);
	color.rgb = vec3(0.0, 0.0, 0.5) + 0.5 * normal;
	color.a = 0.8;
}
