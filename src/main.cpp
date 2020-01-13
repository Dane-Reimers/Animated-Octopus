/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;
double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = glm::vec3(0, 0, -15);
		rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double frametime)
	{
		double ftime = frametime;
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -1*ftime;
		else if(d==1)
			yangle = 1*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:
	int kn = 0, ka = 0, kd = 0;
	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, shapeprog, bubbleProg, globeProg;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox, IndexBufferIDBox;

	Shape shape, icoshape;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			ka = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			ka = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			kd = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			kd = 0;
		}
		if (key == GLFW_KEY_N && action == GLFW_PRESS) kn = 1;
		if (key == GLFW_KEY_N && action == GLFW_RELEASE) kn = 0;
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../resources";
		//try t800.obj or F18.obj ...
		shape.loadMesh(resourceDirectory + "/sphere.obj");
		shape.resize();
		shape.init();

		icoshape.loadMesh(resourceDirectory + "/icosphere.obj");
		icoshape.resize();
		icoshape.init();


		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		vec3 cylinder_vertices[81 * 4];
		float angle1, angle2;
		for (int i = 0; i < 81; i++) {
			angle1 = 2 * 3.141596 / (80 * 2) * (i * 2 + 0);
			angle2 = 2 * 3.141596 / (80 * 2) * (i * 2 + 1);

			cylinder_vertices[i * 4 + 0][0] = sin(angle1);
			cylinder_vertices[i * 4 + 0][1] = 1.0;
			cylinder_vertices[i * 4 + 0][2] = cos(angle1);

			cylinder_vertices[i * 4 + 1][0] = sin(angle2);
			cylinder_vertices[i * 4 + 1][1] = 1.0;
			cylinder_vertices[i * 4 + 1][2] = cos(angle2);

			cylinder_vertices[i * 4 + 2][0] = sin(angle1);
			cylinder_vertices[i * 4 + 2][1] = -1.0;
			cylinder_vertices[i * 4 + 2][2] = cos(angle1);

			cylinder_vertices[i * 4 + 3][0] = sin(angle2);
			cylinder_vertices[i * 4 + 3][1] = -1.0;
			cylinder_vertices[i * 4 + 3][2] = cos(angle2);
		}

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cylinder_vertices), cylinder_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cylinder_elements[81 * 12];

		for (int i = 0; i < 81; i++) {
			cylinder_elements[i * 12 + 0] = i * 4 + 2;
			cylinder_elements[i * 12 + 1] = i * 4 + 1;
			cylinder_elements[i * 12 + 2] = i * 4 + 0;

			cylinder_elements[i * 12 + 3] = i * 4 + 2;
			cylinder_elements[i * 12 + 4] = i * 4 + 3;
			cylinder_elements[i * 12 + 5] = i * 4 + 1;

			cylinder_elements[i * 12 + 6] = i * 4 + 3;
			cylinder_elements[i * 12 + 7] = i * 4 + 4;
			cylinder_elements[i * 12 + 8] = i * 4 + 1;

			cylinder_elements[i * 12 + 9] = i * 4 + 3;
			cylinder_elements[i * 12 + 10] = i * 4 + 6;
			cylinder_elements[i * 12 + 11] = i * 4 + 4;
		}

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cylinder_elements), cylinder_elements, GL_STATIC_DRAW);




		glBindVertexArray(0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// Enable blending/transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
			}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertColor");

		// Initialize the GLSL program.
		shapeprog = std::make_shared<Program>();
		shapeprog->setVerbose(true);
		shapeprog->setShaderNames(resourceDirectory + "/shape_vertex.glsl", resourceDirectory + "/shape_fragment.glsl");
		if (!shapeprog->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
			}
		shapeprog->addUniform("P");
		shapeprog->addUniform("V");
		shapeprog->addUniform("M");
		shapeprog->addAttribute("vertPos");
		shapeprog->addAttribute("vertTex");
		shapeprog->addAttribute("vertNor");

		// Prog for bubbles.
		bubbleProg = std::make_shared<Program>();
		bubbleProg->setVerbose(true);
		bubbleProg->setShaderNames(resourceDirectory + "/bubble_vertex.glsl", resourceDirectory + "/bubble_fragment.glsl");
		if (!bubbleProg->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
			}
		bubbleProg->addUniform("P");
		bubbleProg->addUniform("V");
		bubbleProg->addUniform("M");
		bubbleProg->addAttribute("vertPos");
		bubbleProg->addAttribute("vertNor");
		bubbleProg->addAttribute("vertTex");

		// Prog for bubbles.
		globeProg = std::make_shared<Program>();
		globeProg->setVerbose(true);
		globeProg->setShaderNames(resourceDirectory + "/globe_vertex.glsl", resourceDirectory + "/globe_fragment.glsl");
		if (!globeProg->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
			}
		globeProg->addUniform("P");
		globeProg->addUniform("V");
		globeProg->addUniform("M");
		globeProg->addAttribute("vertPos");
		globeProg->addAttribute("vertNor");
		globeProg->addAttribute("vertTex");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{

		double frametime = get_last_elapsed_time();
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = glm::mat4(1);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		V = mycam.process(frametime);

		static float w = 0.0;
		w += 0.075;

		static float rot = 0.0;
		if (ka == 1)
			rot += 0.02;
		if (kd == 1)
			rot -= 0.02;

		shapeprog->bind();
		glUniformMatrix4fv(shapeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(shapeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		// Bottom of head
		mat4 Shead1 = glm::scale(glm::mat4(1.0f), glm::vec3(1.25, 1.0, 1.25));
		mat4 Rhead1 = glm::rotate(glm::mat4(1.0f), rot, glm::vec3(0.0, 1.0, 0.0));
		mat4 Thead1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.75 + sin(w + 3.141596), -0.75));
		mat4 Mhead1 = Thead1 * Rhead1;
		M = Mhead1 * Shead1;
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);

		// Eyes
		mat4 Seye = glm::scale(glm::mat4(1.0f), glm::vec3(0.25, 0.25, 0.25));
		mat4 Teye1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7, -0.25, 0.9));
		mat4 Meye1 = Mhead1 * Teye1;
		M = Meye1 * Seye;
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);
		mat4 Teye2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.7, -0.25, 0.9));
		mat4 Meye2 = Mhead1 * Teye2;
		M = Meye2 * Seye;
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);

		// Top of head
		mat4 Shead2 = glm::scale(glm::mat4(1.0f), glm::vec3(1.0, 2.25, 1.0));
		mat4 Thead2 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.0, -0.75));
		mat4 Rhead2 = glm::rotate(glm::mat4(1.0f), (float)-3.141596 / 4, glm::vec3(1.0, 0.0, 0.0));
		mat4 Mhead2 = Mhead1 * Thead2 * Rhead2;
		M = Mhead2 * Shead2;
		glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape.draw(shapeprog);

		shapeprog->unbind();
		
		// Draw the cylinders using GLSL.
		prog->bind();
		glBindVertexArray(VertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);


		for (int i = 0; i < 8; i++) {
			float angle = ((float)i) / 8 * 3.141596 * 2;
			mat4 TransRoot = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -0.0125, 0.0));
			mat4 initialRot = glm::rotate(glm::mat4(1.0f), (float)3.141596/6, glm::vec3(-cos(angle), 0.0, sin(angle)));
			mat4 initialTrans = glm::translate(glm::mat4(1.0f), glm::vec3(sin(angle)*0.7, -0.75, cos(angle)*0.7));
			mat4 PrevM = Mhead1 * initialTrans * initialRot * TransRoot;

			mat4 Rot, Scale;
			mat4 TransAttach = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -0.02, 0.0));

			for (int j = 0; j < 150; j++) {
				float scaleFact = 0.2 * (1.0 - ((float)j) / 250);
				Scale = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFact, 0.025, scaleFact));
				Rot = glm::rotate(glm::mat4(1.0), (float)sin(w)/112 + (float)sin(2.35619 + w - (float)j/150 * 3.141596)/64 + 0.01f, glm::vec3(-cos(angle), 0.0, sin(angle)));
				PrevM = PrevM * TransAttach * Rot * TransRoot;
				M = PrevM * Scale;
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glDrawElements(GL_TRIANGLES, 81 * 12, GL_UNSIGNED_SHORT, (void*)0);
				if (j % 20 == 0) {
					shapeprog->bind();
					glUniformMatrix4fv(shapeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
					glUniformMatrix4fv(shapeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
					mat4 Ssuct = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFact, scaleFact, scaleFact));
					mat4 Tsuct = glm::translate(glm::mat4(1.0f), glm::vec3(-sin(angle) * 0.5 * scaleFact, 0.0, -cos(angle) * 0.5 * scaleFact));
					M = PrevM * Tsuct;
					M = M * Ssuct;
					glUniformMatrix4fv(shapeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					shape.draw(shapeprog);
					shapeprog->unbind();
					prog->bind();
					glBindVertexArray(VertexArrayID);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
					glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
				}
			}
		}


		// Draw the bubbles
		static bool bubbleStartsGen = false;
		static glm::mat4 bubbleStarts[24];

		if (bubbleStartsGen == false) {
			mat4 TransBubble;
			for (int i = 0; i < 24; i++) {
				float angle = (float)(i % 9) / 8 * 3.141596 * 2 + 3.141596 / 8;
				mat4 TransBubble = glm::translate(glm::mat4(1.0f), glm::vec3(sin(angle)*((float)(rand() % 100) / 5 + 24)/9, (float)(rand() % 100) / 100.0 * 3 - 1.5, cos(angle)*((float)(rand() % 100) / 5 + 24)/9));
				bubbleStarts[i] = TransBubble;
			}
			bubbleStartsGen = true;
		}

		bubbleProg->bind();
		glUniformMatrix4fv(bubbleProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(bubbleProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		mat4 ScaleBubble = glm::scale(glm::mat4(1.0f), vec3(0.1, 0.1, 0.1));
		for (int i = 0; i < 24; i++) {
			float angle = (float)(i % 9) / 8 * 3.141596 * 2 + 3.141596 / 8;
			mat4 RotBubble = glm::rotate(glm::mat4(1.0f), w + (float)3.141596, glm::vec3(cos(angle), 0.0, -sin(angle)));
			M = Mhead1 * RotBubble * bubbleStarts[i];
			M = M * ScaleBubble;
			glUniformMatrix4fv(bubbleProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			shape.draw(bubbleProg);
		}
		bubbleProg->unbind();

		// Globe
		globeProg->bind();
		glUniformMatrix4fv(globeProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(globeProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		mat4 Sglobe = glm::scale(glm::mat4(1.0f), glm::vec3(6.0, 6.0, 6.0));
		mat4 Tglobe = glm::translate(glm::mat4(1.0f), vec3(0.0, -0.75 - sin(w + 3.141596), 0.0));
		M = Mhead1 * Tglobe * Sglobe;
		glUniformMatrix4fv(globeProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		icoshape.draw(globeProg);

		glBindVertexArray(0);
		globeProg->unbind();

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
