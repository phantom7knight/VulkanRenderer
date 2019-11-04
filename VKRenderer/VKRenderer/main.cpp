#include "stdafx.h"

#include "vkTimer.h"
#include "vkRenderer.h"


void Initialize()
{
	//Renderer Init
	vkRenderer::getInstance()->Init();
}




void MainLoop()
{
	
	while (!glfwWindowShouldClose(vkRenderer::getInstance()->getWindow()))
	{
		//Frame Rate Manager Init
		float deltaTime = vkTimer::getInstance()->FrameStart(true) / 1000.0f;

		if (deltaTime > 0.15f)
		{
			deltaTime = 0.5f;
		}

		glfwPollEvents();


		//Renderer Update
		vkRenderer::getInstance()->RenderLoop(deltaTime);
		

		//Add other updates here




	}

	vkDeviceWaitIdle(vkRenderer::getInstance()->getDevice());




}


void Destroy()
{
	//Renderer destroy
	vkRenderer::getInstance()->Destroy();
}


int main() 
{

	Initialize();

	MainLoop();

	Destroy();

	system("pause");

	return 0;
}