#include "ModelViewer.h"



ModelViewer::ModelViewer()
{
}


ModelViewer::~ModelViewer()
{
}

void ModelViewer::LoadAModel(std::string fileName)
{
	ModelInfo modelinfor = 	rsrcLdr.LoadModelResource(fileName);

	//Load Index and Vertex Buffer



}


void ModelViewer::PrepareApp()
{
	//Load Model
	LoadAModel("../../Assets/Models/christmas-ball/source/Christmas_Ball_Sketchfab.fbx");

}

void ModelViewer::Update(float deltaTime)
{
}

void ModelViewer::Draw(float deltaTime)
{
}

void ModelViewer::Destroy()
{
}