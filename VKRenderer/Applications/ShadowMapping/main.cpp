#include "../../VKRenderer/stdafx.h"
#include "../../VKRenderer/vkRenderer.h"
#include "../../VKRenderer/vkTimer.h"
#include "ShadowMapping.h"
#include "../../VKRenderer/Camera.h"
#include "../../Dependencies/Imgui/IMGUI/Imgui_Impl.h"


int i = 0;
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

std::chrono::time_point<std::chrono::high_resolution_clock>lastTimeStamp;

void MainLoop(vkRenderer* rendererExample)
{
	
	while (!glfwWindowShouldClose(rendererExample->getWindow()))
	{
		//auto timeStart = std::chrono::high_resolution_clock::now();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		//Renderer Update
		rendererExample->Update(deltaTime);
		
		rendererExample->Draw(deltaTime);

		auto timeEnd = std::chrono::high_resolution_clock::now();

		float deltaTimeDiff = (float)(std::chrono::duration<double, std::milli>(timeEnd - lastTimeStamp).count());

		//Frame Rate Manager Init
		//float deltaTime = (float)deltaTimeDiff / 1000.0f;

		
		//Add other updates here
		//camera update
		//rendererExample->m_MainCamera->update(deltaTime);

		//if (deltaTimeDiff > 1000.0f)
		//{
		//	lastTimeStamp = timeEnd;
		//}

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
	vkRenderer* rendererExample = new ShadowMapping();

	//Initalize Vulkan and GLFW for window.
	rendererExample->Init(); 

	rendererExample->PrepareApp();

	MainLoop(rendererExample);
	
	Destroy(rendererExample);

	//clean up the pointer
	delete rendererExample;

	return 0;
}