#include "World.h"
#include <string.h>
#include "ResourceManager.h"

World::World()
{
	LoadTextures();
	BuildScene();
}

void World::Update(const GameTimer& gt)
{
}

void World::Draw(const GameTimer& gt)
{
}

void World::LoadTextures()
{
	ResourceManager::GetInstance()->GetTextureHolder().load(Textures::ID::Desert, L"Media/Desert.dds");
	ResourceManager::GetInstance()->GetTextureHolder().load(Textures::ID::Eagle,  L"Media/Eagle.dds");
	ResourceManager::GetInstance()->GetTextureHolder().load(Textures::ID::Desert, L"Media/Raptor.dds");


	//auto desertTex = std::make_unique<Texture>();
	//desertTex->Name = "desertTex";
	//desertTex->Filename = L"Media/Desert.dds";

	//auto eagleTex = std::make_unique<Texture>();
	//eagleTex->Name = "eagleTex";
	//eagleTex->Filename = L"Media/Eagle.dds";

	//auto raptorTex = std::make_unique<Texture>();
	//raptorTex->Name = "raptorTex";
	//raptorTex->Filename = L"Media/Raptor.dds";
}


void World::BuildScene()
{
}


