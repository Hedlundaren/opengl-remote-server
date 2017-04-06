#version 430
 

in vec3 newPos;
in vec3 newNormal;
in vec2 newUV;

out vec4 outputF;

uniform float time;
uniform float camPos;
uniform vec3 lDir;

void main()
{
	outputF = vec4(vec3(newUV.x, newUV.y, 0.0), 1.0);
 }