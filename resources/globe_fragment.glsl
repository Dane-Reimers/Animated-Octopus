#version 330 core
out vec4 color;
in vec3 fragNor;
vec3 normal;
void main()
{
	normal = normalize(fragNor);
	color.a = 0.5;	//transparency: 1 .. 100% NOT transparent
	color.rgb = vec3(0.0, 0.0, 0.5) + normal * 0.5;
}
