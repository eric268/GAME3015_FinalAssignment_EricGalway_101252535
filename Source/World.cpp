#include "World.h"
#include <string.h>
#include "ResourceManager.h"
#include "DirectX12Application.h"

World::World(): objIndex(0)
{
	LoadTextures();
	BuildScene();
}

void World::Update(const GameTimer& gt)
{
}

void World::Draw(const GameTimer& gt)
{
	//static_cast<DirectX12Application*>(D3DApp::GetApp())->createShapeInWorld(objIndex, XMFLOAT3(5.0f, 5.0f, 5.0f), XMFLOAT3(0.0, -10.0, -20.0), XMFLOAT3(), "grid", Textures::ID::Eagle);
}

void World::LoadTextures()
{
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(Textures::ID::Desert, L"Media/Desert.dds"));
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(Textures::ID::Eagle, L"Media/Eagle.dds"));
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(Textures::ID::Raptor, L"Media/Raptor.dds"));
}


void World::BuildScene()
{
}


