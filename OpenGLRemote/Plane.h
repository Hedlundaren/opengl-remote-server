#pragma once
#include <vector>
#include <glm/glm.hpp>

#ifdef _WIN32
#include "GL/glew.h"
#endif

#include <GLFW/glfw3.h>

class Plane
{
public:
	Plane();
	Plane(const int rx, const int rz, const int w, const int l);
	~Plane();
	void draw();
	void create_plane();

private:
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	GLuint VAO, IBO, VBO;

	int width, length;
	const int res_x, res_z;
};

