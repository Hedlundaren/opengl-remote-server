#pragma once
#include <glm/glm.hpp>
#include <iostream>

class DataPackage
{
public:

	glm::vec3 acceleration;
	glm::vec3 a1;
	glm::vec3 a2;
	glm::vec3 a3;
	glm::vec3 rotation;
	glm::vec3 velocity;
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec2 drag;
	glm::vec2 rot_drag;

	DataPackage();
	
	// Controls
	float resistance = 5.0f;
	float rotation_resistance = 3.0f;
	float drag_resistance = 0.005f;
	float scale_factor = 0.003f;

	// Painting
	float texture_painting = 0.0f; // bool for shader 0-1
	glm::vec3 painting_color = glm::vec3(0.5, 0.2, 0.0);
	float brush_size = 2.5f;
	float biggest_brush_size = 10.0f;
	float brush_stiffness = 0.0f;
	float opacity = 0.3f;

	void update(float dt);

	void inline print() {
		std::cout << "acceleration: (" << acceleration.x << ", " << acceleration.y << ", " << acceleration.z << ")\n";
		std::cout << "rotation: (" << rotation.x << ", " << rotation.y << ", " << rotation.z << ")\n";
		std::cout << "scale: (" << scale.x << ", " << scale.y << ", " << scale.z << ")\n";
	}

	void resetPosition();
};

