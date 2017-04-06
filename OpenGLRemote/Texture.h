#pragma once
#include <iostream>
#ifdef _WIN32
#include "GL/glew.h"
#endif

#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>


class Texture
{
public:
	Texture(const char * path);
	Texture(const char * path, int w, int h);
	~Texture();
	void bindTexture();

protected:
	void generateTexture(const char * path);
	void saveTexture(const char * path);
private:
	int width, height;
	unsigned char* image;
	GLuint texture;
};

