#include "../../VKRenderer/stdafx.h"
#include "../../VKRenderer/vkRenderer.h"
#include "../../VKRenderer/vkTimer.h"
#include "ModelViewer.h"
#include "../../VKRenderer/Camera.h"


int i = 0;
std::chrono::time_point<std::chrono::high_resolution_clock>lastTimeStamp;

void MainLoop(vkRenderer* rendererExample)
{
	
	while (!glfwWindowShouldClose(rendererExample->getWindow()))
	{
		auto timeStart = std::chrono::high_resolution_clock::now();

		glfwPollEvents();

		//Renderer Update
		rendererExample->Update(0.0f);
		
		rendererExample->Draw(0.0f);

		auto timeEnd = std::chrono::high_resolution_clock::now();

		float deltaTimeDiff = (float)(std::chrono::duration<double, std::milli>(timeEnd - lastTimeStamp).count());

		//Frame Rate Manager Init
		float deltaTime = (float)deltaTimeDiff / 1000.0f;//vkTimer::getInstance()->FrameStart(true) / 1000.0f;

		
		//Add other updates here
		//camera update
		rendererExample->m_MainCamera->update(deltaTime);

		//print camera position

		if (i % 500 == 0)
		{
			std::cout << "The position is " << " X: " << rendererExample->m_MainCamera->camProperties.position.x << " Y: " << rendererExample->m_MainCamera->camProperties.position.y << " Z: "
				<< rendererExample->m_MainCamera->camProperties.position.z << std::endl;
			i = 0;
		}
		else
		{
			++i;
		}

		if (deltaTimeDiff > 1000.0f)
		{
			lastTimeStamp = timeEnd;
		}

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