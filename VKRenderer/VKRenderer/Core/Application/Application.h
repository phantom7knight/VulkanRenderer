#pragma once

#include "../PCH/stdafx.h"

class Application
{
public:

	//Functions
	Application();
	virtual ~Application();
	
	
protected:
	
	//Setup Renderer, GUI, Camera Properties
	virtual void Init() = 0;

	//Setup FBO's, Render Passes, Pipelines,.... etc
	virtual void PrepareApp() = 0;
	
	//Draw Function for App
	virtual void Draw(float deltaTime) = 0;
	
	//Update Function for App
	virtual void Update(float deltaTime) = 0;
	
	//De-Init all the resource allocated
	virtual void Destroy() = 0;

};

