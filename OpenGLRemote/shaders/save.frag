#version 430
 

in vec3 newPos;
in vec2 newUV;

out vec4 outputF;

uniform sampler2D textureBuffer;

void main()
{
	vec3 color = vec3(texture(textureBuffer, newUV));
	outputF = vec4(color, 1.0);
 }