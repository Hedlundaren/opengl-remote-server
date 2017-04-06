#version 430
 

in vec3 newPos;

out vec4 outputF;

uniform sampler2D uvCoordBuffer;
uniform sampler2D saveBuffer;
uniform vec2 mouseCoord;
uniform vec2 mouseCoordOld;
uniform float texture_painting;
uniform float brush_stiffness;
uniform float opacity;
uniform vec3 painting_color;
uniform float brush_size;

float cross_product(vec2 u, vec2 v){
	return u.x*v.y-u.y*v.x;
}

float distance_to_line(vec2 line_start, vec2 line_end, vec2 point){
	float dist = abs(cross_product( line_end-line_start, line_start-point)) / length(line_end-line_start);
	return dist;
}

vec2 ortho_vec(vec2 vector){
    return vec2(-vector.y, vector.x);
}

void main()
{
	float line_thickness = 0.005 * brush_size;
	float intensity = 0;
	float width = 1920;
	float height = 1080;
	float pi = 3.1415;

	// The two points
	vec2 p1 = vec2(mouseCoord.x / width, -mouseCoord.y / height + 1.0);
	vec2 p2 = vec2(mouseCoordOld.x / width, -mouseCoordOld.y / height + 1.0 );

	vec2 screen_coord = vec2(gl_FragCoord.x / width, gl_FragCoord.y / height );
	vec3 color = vec3(texture(saveBuffer, screen_coord));
	vec2 uv_p1 = vec2(texture(uvCoordBuffer, p1));
	vec2 uv_p2 = vec2(texture(uvCoordBuffer, p2));
	
	float point_distance = 0.0f;
	float line_dist = 0.0f;
	float od1 = 0.0f;
	float od2 = 0.0f;

	if(texture_painting > 0.5){ // paint on texture
		
		vec2 line_dir = p2 - p1;
		uv_p1 = p1; 
		// Get distance to line
		vec2 ortho_vec = normalize(ortho_vec(line_dir));
		point_distance = length(line_dir);

		line_dist = distance_to_line(p1, p2, screen_coord);
		od1 = distance_to_line(p1, (p1 + ortho_vec), screen_coord);
		od2 = distance_to_line(p2, (p2 + ortho_vec), screen_coord);
	}else{ // paint on mesh

		vec2 line_dir = uv_p2 - uv_p1;
		vec2 ortho_vec = normalize(ortho_vec(line_dir));
		point_distance = length(line_dir);

		line_dist = distance_to_line(uv_p1, uv_p2, screen_coord);
		od1 = distance_to_line(uv_p1, (uv_p1 + ortho_vec), screen_coord);
		od2 = distance_to_line(uv_p2, (uv_p2 + ortho_vec), screen_coord);
	}

	// Distance to p1
	vec2 d = uv_p1 - screen_coord;
	float dist = length(d);
	intensity = pow(intensity, 5.0);
	vec4 paint;
	if(point_distance < 0.2 && line_dist < line_thickness && od1 < point_distance && od2 < point_distance){
		intensity = cos(line_dist * pi/line_thickness)*0.5 + 0.5 * (1 - brush_stiffness) + brush_stiffness * (1);
		paint = opacity * intensity * vec4(painting_color, intensity);
	}

	if( dist < line_thickness){
		intensity = cos(dist * pi/line_thickness)*0.5 + 0.5 * (1 - brush_stiffness) + brush_stiffness * (1);
		paint += opacity * intensity * vec4(painting_color, intensity);
	}

	outputF = vec4(color, 1) + paint;
 }