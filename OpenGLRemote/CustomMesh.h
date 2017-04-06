#pragma once
#include <vector>
#include <glm/glm.hpp>

#ifdef _WIN32
#include "GL/glew.h"
#endif

#include <GLFW/glfw3.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>


class CustomMesh
{
public:
	CustomMesh();
	~CustomMesh();
	void load(const std::string path);
	void center_mesh();
	void normalize();
	void uvMapSphere();
	void calcNormals();
	void draw();

private:

	void readFile(const std::string path);
	std::vector<std::string> CustomMesh::split(const std::string &s, char delim);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;

	GLuint VAO, IBO, VBO, NBO, UVBO;

	glm::vec3 center_point;
	float longest_distance;
	bool uvs_found = false;
	bool normals_found = false;
};

