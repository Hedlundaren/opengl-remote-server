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

#include <stdio.h>

#define GLFW_HAND_CURSOR 0x00036004
#define GLFW_CROSSHAIR_CURSOR   0x00036003

#define WIDTH 1920
#define HEIGHT 1080

void StartNetworkThread(DataPackage* d) {
	SocketManager *mSocketManager = new SocketManager(d);
	mSocketManager->StartServer();
}

struct PAINTSTATES {
	bool PAINTING = false;
	bool TEXTURE = false;
	bool SOFT = false;
};

bool save_screenshot(string filename, int w, int h)
{
	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w*h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*)malloc(nSize * sizeof(char));

	if (!dataBuffer) return false;

	// Let's fetch them from the backbuffer	
	// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
	glReadPixels((GLint)0, (GLint)0,
		(GLint)w, (GLint)h,
		GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	//Now the file creation
	FILE *filePtr = fopen(filename.c_str(), "wb");
	if (!filePtr) return false;


	unsigned char TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
	unsigned char header[6] = { w % 256,w / 256,
		h % 256,h / 256,
		24,0 };
	// We write the headers
	fwrite(TGAheader, sizeof(unsigned char), 12, filePtr);
	fwrite(header, sizeof(unsigned char), 6, filePtr);
	// And finally our image data
	fwrite(dataBuffer, sizeof(GLubyte), nSize, filePtr);
	fclose(filePtr);

	free(dataBuffer);

	return true;
}

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

	// Custom cursor
	GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	glfwSetCursor(window, cursor);

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

	Texture canvas_texture("textures/oldbunny.jpg");


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
		if (glfwGetKey(window, GLFW_KEY_X)) { dataPackage->texture_painting = 1.0f; }
		if (glfwGetKey(window, GLFW_KEY_Z)) { dataPackage->texture_painting = 0.0f; }

		// Get mouse position
		glfwGetCursorPos(window, &currentX, &currentY);
		mouseCoord = glm::vec2(currentX, currentY);

		// Render Options
		if (glfwGetKey(window, GLFW_KEY_W)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Send sensor data
		dataPackage->update(deltaTime);

		

		/*====================================================
		===================== PAINTING ======================
		=====================================================*/

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
		if (PS->PAINTING) {
			// Save texture
			saveBuffer.bindBuffer();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			save_program();
			texLoc = glGetUniformLocation(save_program, "textureBuffer");
			glUniform1i(texLoc, 0);
			glActiveTexture(GL_TEXTURE0);
			textureBuffer.bindTexture();
			quad.draw();
		}
		

		// STEP 4 Render canvas - Either 3D or texture
		if (dataPackage->texture_painting == 1.0f) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			save_program();
			texLoc = glGetUniformLocation(save_program, "textureBuffer");
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
			textureBuffer.bindTexture();
			custom_program.updateCommonUniforms(rotator, WIDTH, HEIGHT, currentTime, dataPackage);
			model.draw();
		}
		
		// STEP 6 - Save old mouse coords
		mouseCoordOld = mouseCoord;
		/*====================================================
		===================== END ============================
		=====================================================*/
		// save image
		if (glfwGetKey(window, GLFW_KEY_S) && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			save_program();
			texLoc = glGetUniformLocation(save_program, "textureBuffer");
			glUniform1i(texLoc, 0);
			glActiveTexture(GL_TEXTURE0);
			textureBuffer.bindTexture();
			quad.draw();

			if (save_screenshot("textures/new_texture.tga", WIDTH, HEIGHT)) {
				std::cout << "Texture Saved.\n";
			}
			else {
				std::cout << "Error saving texture.\n";
			}
		}

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