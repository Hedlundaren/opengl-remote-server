#include "DataPackage.h"

DataPackage::DataPackage() {
	acceleration = glm::vec3(0);
	a1 = glm::vec3(0);
	a2 = glm::vec3(0);
	a3 = glm::vec3(0);

	rotation = glm::vec3(0);
	velocity = glm::vec3(0);
	position = glm::vec3(0);
	scale = glm::vec3(0);
}

void DataPackage::update(float dt) {

	// Smooth acceleration
	a2 = a1;
	a1 = acceleration;
	position = resistance * (a1 + a2) / 2.0f;

}

void DataPackage::resetPosition() {
	position = glm::vec3(0);
	rotation = glm::vec3(0);
	velocity = glm::vec3(0);
	acceleration = glm::vec3(0);
}