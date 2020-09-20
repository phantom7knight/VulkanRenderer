#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../VKRenderer/Core/Application/Application.h"
#include "../../VKRenderer/Core/Camera/Camera.h"
#include "ModelViewer.h"


int i = 0;
std::chrono::time_point<std::chrono::high_resolution_clock>lastTimeStamp;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void MainLoop(Application* rendererExample)
{
	
	while (!glfwWindowShouldClose(rendererExample->getRenderer()->getWindow()))
	{
		//Frame Rate Manager Init
		//float deltaTime = 0.0f;// vkTimer::getInstance()->FrameStart(true) / 1000.0f;
		auto currentTime = glfwGetTime();// std::chrono::high_resolution_clock::now();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		/*if (deltaTime > 0.15f)
		{
			deltaTime = 0.1f;
		}*/

		glfwPollEvents();

		//Renderer Update
		rendererExample->Update(deltaTime);
		
		rendererExample->Draw(deltaTime);
		//auto timeEnd = glfwGetTime();// std::chrono::high_resolution_clock::now();

		//float deltaTimeDiff = (float)(std::chrono::duration<double, std::milli>(timeEnd - lastTimeStamp).count());
		//deltaTime = timeStart - timeEnd;// (float)deltaTimeDiff / 1000.0f;
		
		//Add other updates here

		//camera update
		rendererExample->getRenderer()->m_MainCamera->update(deltaTime);
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