#version 430
 
layout (location = 0) in vec3 position;
 
out vec3 newPos;

void main()
{
	newPos = position;

    gl_Position = vec4(newPos, 1.0);
} 