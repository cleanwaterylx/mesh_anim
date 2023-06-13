#define GLM_FORCE_RADIANS
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "TestMD5GPU.h"
#include "Shader.h"
Camera camera;
float lastFrame = 0;
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Model Anim", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader shader("../shader/modelGPU.vert", "../shader/modelGPU.frag");
	std::string path = "../Model/boblampclean.md5mesh";
	std::string animPath = "../Model/boblampclean.md5anim";
	TestMD5 newModel;
	newModel.LoadModel(path);
    newModel.animation.LoadAnimation(animPath);

	//	newModel.LoadAnim(animPath);
	while (!glfwWindowShouldClose(window))
	{
		float deltaTime = glfwGetTime() - lastFrame;
		lastFrame += deltaTime;
		glClearColor(0.1f, 0.3f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 proj;
		proj = glm::perspective(glm::radians(camera.Zoom), 8.0f / 6.0f, 0.1f, 100.0f);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -5.0f, -17.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		shader.setMat4("project", proj);
		newModel.Update(deltaTime);
		newModel.Render(shader);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	glDeleteProgram(shader.ID);
	return 0;
}
