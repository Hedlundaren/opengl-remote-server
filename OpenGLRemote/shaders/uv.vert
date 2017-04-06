#version 430
 
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
 
out vec3 newPos;
out vec3 newNormal;
out vec2 newUV;

uniform float time;
uniform mat4 MV;
uniform mat4 P;

void main()
{
	newPos = position;
	newNormal = normal;
	newUV = uv;

    gl_Position = P * MV * vec4(newPos, 1.0);
} 