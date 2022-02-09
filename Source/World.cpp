#include "World.h"
#include <string.h>
#include "ResourceManager.h"
#include "DirectX12Application.h"

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
	AddTexture(Textures::ID::Desert, L"Media/Desert.dds");
	AddTexture(Textures::ID::Eagle, L"Media/Eagle.dds");
	AddTexture(Textures::ID::Raptor, L"Media/Raptor.dds");
}


void World::BuildScene()
{
	//static_cast<DirectX12Application*>(D3DApp::GetApp())->createShapeInWorld(objIndex, XMFLOAT3(5.0f, 5.0f, 5.0f), XMFLOAT3(0.0, -10.0, -20.0), XMFLOAT3(), "grid", Textures::ID::Eagle);
}

void World::AddTexture(Textures::ID id, std::wstring fileName)
{
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(id, fileName));
}


