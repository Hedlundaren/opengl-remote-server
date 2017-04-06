#include "CustomMesh.h"
#include <iostream>


CustomMesh::CustomMesh()
{
	
}

CustomMesh::~CustomMesh()
{
}

// Method functions
void CustomMesh::draw() {
	glLineWidth(2.0);
	glPointSize(2.0);

	//Draw Object
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBindVertexArray(VAO);

	glDrawElements(
		GL_TRIANGLES,      // mode
		indices.size(),    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

	glBindVertexArray(0);

}


void CustomMesh::load(const std::string path) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	readFile(path);
	if (!uvs_found) {
		uvMapSphere(); // only needed if no uvs along with model
	}

	if (!normals_found) {
		calcNormals(); // only needed if no normals along with model
	}

	// Model vertices
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

	// Model normals
	glGenBuffers(1, &NBO);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float) * 3, &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);


	// Model uv coords
	glGenBuffers(1, &UVBO);
	glBindBuffer(GL_ARRAY_BUFFER, UVBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float) * 2, &uvs[0], GL_STATIC_DRAW); // Give our uvs to OpenGL.
	glEnableVertexAttribArray(2); 
	glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, sizeof(float) * 2,(void*)0);

	// Model indices
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

void CustomMesh::readFile(std::string path) {
	
	center_point = glm::vec3(0);
	longest_distance = 0.0f;

		
	// Used to normalize
	float BIG = 999999999.0f;
	float minX = BIG,
		minY = BIG,
		minZ = BIG,
		maxX = -BIG,
		maxY = -BIG,
		maxZ = -BIG;
	
	std::ifstream in_file;
	in_file.open(path);
	std::string line;

	if (in_file.is_open())
	{
		while (getline(in_file, line))
		{
			std::istringstream iss(line); // Open string stream
			std::vector<std::string> elements; // Store elements of each row
			while (iss)
			{
				std::string sub;
				iss >> sub;
				elements.push_back(sub);
			}
			iss.str(std::string());

			// Treat line
			if (elements.size() > 0) {
				
				if (elements[0] == "Vertex") { // String format: Vertex id  vx vy vz {normal=(nx ny nz)}
					

					glm::vec3 vertex = glm::vec3();
					glm::vec3 normal = glm::vec3();
					glm::vec2 uv = glm::vec2();
					glm::vec3 color = glm::vec3();

					// extract vertex
					if (elements[2] == "0") elements[2] = "0.0";
					if (elements[3] == "0") elements[3] = "0.0";	
					if (elements[4] == "0") elements[4] = "0.0";
					vertex.x = atof(elements[2].c_str());
					vertex.y = atof(elements[3].c_str());
					vertex.z = atof(elements[4].c_str());
					if (vertex.x < minX) minX = vertex.x;
					if (vertex.y < minY) minY = vertex.y;
					if (vertex.z < minZ) minZ = vertex.z;
					if (vertex.x > maxX) maxX = vertex.x;
					if (vertex.y > maxX) maxY = vertex.y;
					if (vertex.z > maxX) maxZ = vertex.z;
					center_point += vertex;
					float current_distance = sqrt(pow(vertex.x, 2.0f) + pow(vertex.y, 2.0f) + pow(vertex.z, 2.0f));
					if (current_distance > longest_distance){ longest_distance = current_distance;  }
					vertices.push_back(vertex);

					
					// extract attributes
					size_t s = line.find("{");
					size_t e = line.find("}", s);
					std::string attr_string = line.substr(s + 1, e - s - 1);

					std::vector<std::string> div_string = split(attr_string, '='); // every second elmt is name and every other second is value



					for (int i = 1; i < div_string.size(); i++) {

						std::vector<std::string> last_elements = split(div_string[i-1], ' ');
						std::string attribute = last_elements[last_elements.size() - 1];

						if (attribute == "normal") { // extract normal
							normals_found = true;
							size_t s2 = div_string[i].find("(");
							size_t e2 = div_string[i].find(")", s2);
							std::string normal_string = div_string[i].substr(s2 + 1, e2 - s2 - 1);
							std::vector<std::string> normal_elems = split(normal_string, ' ');

							normal.x = atof(normal_elems[0].c_str());
							normal.y = atof(normal_elems[1].c_str());
							normal.z = atof(normal_elems[2].c_str());
							normals.push_back(normal);
						}

						if (attribute == "uv") { // extract uv
							uvs_found = true;
							size_t s2 = div_string[i].find("(");
							size_t e2 = div_string[i].find(")", s2);
							std::string normal_string = div_string[i].substr(s2 + 1, e2 - s2 - 1);
							std::vector<std::string> normal_elems = split(normal_string, ' ');

							uv.x = atof(normal_elems[0].c_str());
							uv.y = atof(normal_elems[1].c_str());
							uvs.push_back(uv);
						}

					}

				}
				else if (elements[0] == "Face") { // String format: Face id  index_1 index_2 index_3

					// Convert string to integers
					int index_1 = atoi(elements[2].c_str()) - 1;
					int index_2 = atoi(elements[3].c_str()) - 1;
					int index_3 = atoi(elements[4].c_str()) - 1;

					// Push indices
					indices.push_back(index_1);
					indices.push_back(index_2);
					indices.push_back(index_3);
				}
			}
		}
		in_file.close();
		center_point /= vertices.size();
	}
}

std::vector<std::string> CustomMesh::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


void CustomMesh::center_mesh() {
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].x -= center_point.x;
		vertices[i].y -= center_point.y;
		vertices[i].z -= center_point.z;
	}
	center_point = glm::vec3(0);

	// Model vertices
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}

void CustomMesh::normalize() {

	float inv_dist = 1.0f / longest_distance;
	std::cout << longest_distance;
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].x *= inv_dist;
		vertices[i].y *= inv_dist;
		vertices[i].z *= inv_dist;
	}

	// Model vertices
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

}

void CustomMesh::uvMapSphere() {
	for (int i = 0; i < vertices.size(); i++) {
		// uv coords
		const float PI = 3.1415;

		// u coord
		float alpha = atan(vertices[i].z / vertices[i].x);
		float u = alpha / (2.0f * PI);

		// v coord
		float d = sqrt(pow(vertices[i].x, 2.0f) + pow(vertices[i].z, 2.0f));
		float beta = atan(d / abs(vertices[i].z));
		if (vertices[i].y > 0) {
			beta = PI - beta;
		}
		float v = beta / PI;
		uvs.push_back(glm::vec2(u, v));
	}
}

void CustomMesh::calcNormals() {
	for (int i = 0; i < vertices.size(); i++) {
	
		glm::vec3 normal = glm::vec3(0, 1, 0);
		normals.push_back(normal);
	}
}

