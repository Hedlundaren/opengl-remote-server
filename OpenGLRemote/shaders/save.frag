#version 430
 

in vec3 newPos;

out vec4 outputF;

uniform sampler2D textureBuffer;

void main()
{
	float width = 1920;
	float height = 1080;
	vec3 color = vec3(0,0,0);

	vec2 screen_coord = vec2(gl_FragCoord.x / width, gl_FragCoord.y / height );
	
	color = vec3(texture(textureBuffer, screen_coord));

	outputF = vec4(color, 1.0);
 }