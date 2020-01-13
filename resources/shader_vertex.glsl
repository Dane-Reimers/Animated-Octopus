#version 330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 vertex_color;
void main()
{
	vec3 fragNor = vec4(M * vec4(vertPos[0], vertPos[1], vertPos[2], 0.0)).xyz;
	vertex_color = normalize(fragNor);
	gl_Position = P * V * M * vec4(vertPos, 1.0);
}
