#include "stdafx.h"

#include "vkRenderer.h"


int main() 
{

	vkRenderer::getInstance()->Init();

	vkRenderer::getInstance()->mainloop();

	vkRenderer::getInstance()->Destroy();

	system("pause");

	return 0;
}