#include "../../VKRenderer/stdafx.h"
#include "../../VKRenderer/vkRenderer.h"
#include "ModelViewer.h"



void MainLoop(vkRenderer* rendererExample)
{
	
	while (!glfwWindowShouldClose(rendererExample->getWindow()))
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


	vkDeviceWaitIdle(rendererExample->getDevice());

}


void Destroy(vkRenderer * rendererExample)
{
	//Renderer destroy
	rendererExample->Destroy();
	
}


int main() 
{

	
	//Init a base class
	vkRenderer* rendererExample = new ModelViewer();

	//Initalize Vulkan and GLFW for window.
	rendererExample->Init(); 

	rendererExample->PrepareApp();

	MainLoop(rendererExample);
	
	Destroy(rendererExample);

	//clean up the pointer
	delete rendererExample;

	return 0;
}