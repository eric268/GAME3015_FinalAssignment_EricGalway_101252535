#include "World.h"
#include <string.h>
#include "ResourceManager.h"
#include "DirectX12Application.h"
#include "SpriteNode.h"

World::World()
{
	LoadTextures();
}

void World::Update(const GameTimer& gt)
{
}

void World::Draw(const GameTimer& gt)
{
	mSceneGraph.Draw(gt);
}

void World::LoadTextures()
{
	AddTexture(Textures::ID::Desert, L"Media/Desert.dds");
	AddTexture(Textures::ID::Eagle, L"Media/Eagle.dds");
	AddTexture(Textures::ID::Raptor, L"Media/Raptor.dds");
}


void World::BuildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		SceneNode::Ptr layer(new SceneNode());
		mSceneLayers[i] = layer.get();
		mSceneGraph.AttachChild(std::move(layer));
	}
	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode());
	mSceneLayers[Background]->AttachChild(std::move(backgroundSprite));

	std::unique_ptr<SpriteNode> testSprite(new SpriteNode(XMFLOAT3(3.0f,3.0f,3.0f), XMFLOAT3(0.0f,0.0f,0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), Textures::ID::Raptor));
	mSceneLayers[Background]->AttachChild(std::move(testSprite));

	//std::unique_ptr<Aircraft>


}

void World::AddTexture(Textures::ID id, std::wstring fileName)
{
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(id, fileName));
}


