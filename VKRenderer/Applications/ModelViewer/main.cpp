#include "ModelViewer.h"
#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"
#include "../../VKRenderer/Core/Application/Application.h"
#include "../../Dependencies/Imgui/IMGUI/Imgui_Impl.h"
#include "../../VKRenderer/Core/Camera/Camera.h"


int i = 0;
std::chrono::time_point<std::chrono::high_resolution_clock>lastTimeStamp;

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

		//camera update
		rendererExample->m_MainCamera->update(deltaTime);
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