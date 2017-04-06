#version 430
 

in vec3 newPos;
in vec3 newNormal;
in vec2 newUV;

out vec4 outputF;

uniform float time;
uniform float camPos;
uniform vec3 lDir;
uniform sampler2D saveBuffer;

void main()
{
	vec3 lightDir = lDir;
	vec3 normal = newNormal;

	// Colors 
	vec3 ambient_color = vec3(texture(saveBuffer, newUV));
	vec3 diffuse_color = vec3(texture(saveBuffer, newUV));
	vec3 specular_color = vec3(1.0);
	ambient_color = diffuse_color;

	float ka = 0.9;
	vec3 ambient = ambient_color * ka;
	float kd = 0.1;
    vec3 diffuse = clamp( dot( normal, lightDir ), 0.0, 1.0) * diffuse_color;
	float ks = 0.0;
	vec3 eye_pos = normalize(newPos - camPos );
	vec3 R = 2.0*dot(lightDir,normal)*normal - lightDir;
	vec3 specular = ks * pow( clamp(dot(R, eye_pos), 0.0, 1.0), 34.9) * specular_color;	

	vec3 color = diffuse + ambient + specular;

	outputF = vec4(color, 1.0);
	
 }