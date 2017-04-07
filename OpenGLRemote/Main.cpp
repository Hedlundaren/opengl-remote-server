#include "SocketManager.h"
#include "DataPackage.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>


#include <iostream>
#include <algorithm>
#include <vector>

#ifdef _WIN32
#include "GL/glew.h"
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "DisplayWindow.h"
#include "ShaderProgram.h"
#include "Rotator.h"
#include "Framebuffer.h"
#include "Texture.h"

#include "Plane.h"
#include "Sphere.h"
#include "Quad.h"
#include "CustomMesh.h"

#define HEIGHT 1080
#define WIDTH 1920

void StartNetworkThread(DataPackage* d) {
	SocketManager *mSocketManager = new SocketManager(d);
	mSocketManager->StartServer();
}

struct PAINTSTATES {
	bool PAINTING = false;
	bool TEXTURE = false;
	bool SOFT = false;
};

int main()
{
	std::cout << "====== Welcome ======\n";
	
	PAINTSTATES *PS = new PAINTSTATES;

	// Initiate connection
	DataPackage *dataPackage = new DataPackage;
	boost::thread* networkThread;
	networkThread = new boost::thread(StartNetworkThread, dataPackage);

	// Time
	float currentTime, deltaTime, lastTime = 0.0f;
	GLFWwindow* window = nullptr;
	DisplayWindow myWindow = DisplayWindow(window, WIDTH, HEIGHT, "Magic phone.");
	glm::vec3 clear_color = glm::vec3(0.7, 0.65, 0.6);

	// Mouse controls
	MouseRotator rotator;
	rotator.init(window);

	glUseProgram(0);
	ShaderProgram uv_program("shaders/uv.vert", "", "", "", "shaders/uv.frag");
	ShaderProgram paint_program("shaders/paint.vert", "", "", "", "shaders/paint.frag");
	ShaderProgram save_program("shaders/save.vert", "", "", "", "shaders/save.frag");
	ShaderProgram custom_program("shaders/custom.vert", "", "", "", "shaders/custom.frag");

	custom_program();
	uv_program();
	paint_program();
	glUseProgram(0);

	Sphere sphere = Sphere(20, 20, 1);
	Quad quad;
	CustomMesh model = CustomMesh();
	model.load("models/bunnyuv.m");
	model.center_mesh();
	model.normalize();

	Framebuffer uvCoordBuffer(WIDTH, HEIGHT); // Render uv coords to this
	Framebuffer textureBuffer(WIDTH, HEIGHT); // Render texture of object to this
	Framebuffer saveBuffer(WIDTH, HEIGHT); // Render texture of object to this

	Texture canvas_texture("textures/canvas2.jpg");


	GLint texLoc;
	glViewport(0, 0, WIDTH, HEIGHT);

	double currentX, currentY;
	glm::vec2 mouseCoord;
	glm::vec2 mouseCoordOld;

	// Initialize canvas
	saveBuffer.bindBuffer();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	save_program();
	texLoc = glGetUniformLocation(paint_program, "textureBuffer");
	glUniform1i(texLoc, 0);
	glActiveTexture(GL_TEXTURE0);
	canvas_texture.bindTexture();
	quad.draw();

	do {
		// Init frame
		myWindow.initFrame(clear_color);
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		rotator.poll(window);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) { PS->PAINTING = true; }
		else  PS->PAINTING = false;
		if (glfwGetKey(window, GLFW_KEY_T)) { PS->TEXTURE = true; }
		else  PS->TEXTURE = false;

		// Get mouse position
		glfwGetCursorPos(window, &currentX, &currentY);
		mouseCoord = glm::vec2(currentX, currentY);

		// Render Options
		if (glfwGetKey(window, GLFW_KEY_W)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);;

		// Send sensor data
		dataPackage->update(deltaTime);

		// save image
		if (glfwGetKey(window, GLFW_KEY_S)) {
			//saveBuffer.sa
		}
		/*====================================================
		===================== PAINTING ======================
		=====================================================*/

		if (PS->PAINTING) {
			// STEP 1 - Render UV coords
			uvCoordBuffer.bindBuffer();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			uv_program();
			uv_program.updateCommonUniforms(rotator, WIDTH, HEIGHT, currentTime, dataPackage);
			model.draw();

			// STEP 2 - Paint texture
			textureBuffer.bindBuffer();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			paint_program();
			// Send textures
			texLoc = glGetUniformLocation(paint_program, "uvCoordBuffer");
			glUniform1i(texLoc, 0);
			texLoc = glGetUniformLocation(paint_program, "saveBuffer");
			glUniform1i(texLoc, 1);
			glActiveTexture(GL_TEXTURE0);
			uvCoordBuffer.bindTexture();
			glActiveTexture(GL_TEXTURE1);
			saveBuffer.bindTexture();
			// Send mouse coords
			texLoc = glGetUniformLocation(paint_program, "mouseCoord");
			glUniform2fv(texLoc, 1, &mouseCoord[0]);
			texLoc = glGetUniformLocation(paint_program, "mouseCoordOld");
			glUniform2fv(texLoc, 1, &mouseCoordOld[0]);
			// Send states
			if (PS->TEXTURE) dataPackage->texture_painting = 1.0f;
			else dataPackage->texture_painting = 0.0f;
			texLoc = glGetUniformLocation(paint_program, "texture_painting");
			glProgramUniform1f(paint_program, texLoc, dataPackage->texture_painting);
			texLoc = glGetUniformLocation(paint_program, "brush_size");
			glProgramUniform1f(paint_program, texLoc, dataPackage->brush_size);
			texLoc = glGetUniformLocation(paint_program, "brush_stiffness");
			glProgramUniform1f(paint_program, texLoc, dataPackage->brush_stiffness);
			texLoc = glGetUniformLocation(paint_program, "opacity");
			glProgramUniform1f(paint_program, texLoc, dataPackage->opacity);
			texLoc = glGetUniformLocation(paint_program, "painting_color");
			glUniform3fv(texLoc, 1, &dataPackage->painting_color[0]);

			quad.draw();

			// Save texture
			saveBuffer.bindBuffer();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			save_program();
			texLoc = glGetUniformLocation(paint_program, "textureBuffer");
			glUniform1i(texLoc, 0);
			glActiveTexture(GL_TEXTURE0);
			textureBuffer.bindTexture();
			quad.draw();
		}
		

		// STEP 4 Render canvas - Either 3D or texture
		if (PS->TEXTURE) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			save_program();
			texLoc = glGetUniformLocation(paint_program, "textureBuffer");
			glUniform1i(texLoc, 0);
			glActiveTexture(GL_TEXTURE0);
			textureBuffer.bindTexture();
			quad.draw();
		}
		else{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			custom_program();
			texLoc = glGetUniformLocation(custom_program, "saveBuffer");
			glUniform1i(texLoc, 0);
			glActiveTexture(GL_TEXTURE0);
			saveBuffer.bindTexture();
			custom_program.updateCommonUniforms(rotator, WIDTH, HEIGHT, currentTime, dataPackage);
			model.draw();
		}
		


		// STEP 6 - Save old mouse coords
		mouseCoordOld = mouseCoord;
		/*====================================================
		===================== END ============================
		=====================================================*/

		// Finish frame
		glfwSwapInterval(0);
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glDisableVertexAttribArray(0);
	FreeConsole();

	return 0;
}