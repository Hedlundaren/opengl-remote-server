#pragma once

#include <iostream>
#ifdef _WIN32
#include "GL/glew.h"
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class DisplayWindow
{
public:
	DisplayWindow(GLFWwindow* &window, const unsigned width, const unsigned height, const char* title);
	~DisplayWindow();

	void initFrame(glm::vec3 clear_color);


};

inline void DisplayWindow::initFrame(glm::vec3 clear_color) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
}

