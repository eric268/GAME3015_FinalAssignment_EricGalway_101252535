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
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(Textures::ID::Desert, L"Media/Desert.dds"));
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(Textures::ID::Eagle, L"Media/Eagle.dds"));
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(Textures::ID::Raptor, L"Media/Raptor.dds"));

}


void World::BuildScene()
{
}


