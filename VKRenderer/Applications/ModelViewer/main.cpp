#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../VKRenderer/Core/Application/Application.h"
#include "../../VKRenderer/Core/Camera/Camera.h"
#include "ModelViewer.h"

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void StallTillTargetFPS()
{
	while (glfwGetTime() < lastFrame + 1.0 / TARGET_FPS) {
		// TODO: Put the thread to sleep, yield, or simply do nothing
	}
}

void MainLoop(Application* rendererExample)
{
	
	while (!glfwWindowShouldClose(rendererExample->getRenderer()->getWindow()))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		//Renderer Update
		rendererExample->Update(deltaTime);
		
		rendererExample->Draw(deltaTime);
		
		//Add other updates here
		//======================

		//camera update
		rendererExample->getRenderer()->m_MainCamera->CameraUpdate();

		StallTillTargetFPS();
	}

	vkDeviceWaitIdle(rendererExample->getRenderer()->getDevice());
}


void Destroy(Application* rendererExample)
{
	//Renderer destroy
	rendererExample->Destroy();
}


int main() 
{	
	//Init a base class
	Application* rendererExample = new ModelViewer();

	//Initalize Vulkan and GLFW for window.
	rendererExample->Init(); 

	rendererExample->PrepareApp();

	MainLoop(rendererExample);
	
	Destroy(rendererExample);

	//clean up the pointer
	delete rendererExample;

	return 0;
}