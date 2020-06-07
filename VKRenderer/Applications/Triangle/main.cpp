#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"
#include "../../VKRenderer/Core/Application/Application.h"
#include "Triangle.h"


void MainLoop(Application* rendererExample)
{
	
	while (!glfwWindowShouldClose(rendererExample->getRenderer()->getWindow()))
	{
		//Frame Rate Manager Init
		float deltaTime = 0.0f;// vkTimer::getInstance()->FrameStart(true) / 1000.0f;

		if (deltaTime > 0.15f)
		{
			deltaTime = 0.5f;
		}

		glfwPollEvents();


		//Renderer Update
		rendererExample->Update(deltaTime);
		
		rendererExample->Draw(deltaTime);
		
		//Add other updates here


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
	Application* rendererExample = new Triangle();

	//Initalize Vulkan and GLFW for window.
	rendererExample->Init();

	rendererExample->PrepareApp();

	MainLoop(rendererExample);
	
	Destroy(rendererExample);

	//clean up the pointer
	delete rendererExample;

	return 0;
}