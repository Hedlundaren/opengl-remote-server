#version 430
 
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec3 newPos;
out vec2 newUV;

void main()
{
	newUV = uv;
	newPos = vec3(position.x, -position.y, 0.0);
    gl_Position = vec4(newPos, 1.0);
} 