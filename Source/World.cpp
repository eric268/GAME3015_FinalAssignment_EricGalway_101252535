#include "World.h"
#include <string.h>
#include "ResourceManager.h"
#include "DirectX12Application.h"
#include "SpriteNode.h"

World::World() : mSpawnPosition(XMFLOAT3(0,-20,20))
{
	LoadTextures();
}

void World::Update(const GameTimer& gt)
{
	mSceneGraph.Update(gt);
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
	backgroundSprite->SetScale(XMFLOAT3(1.0f, 2.0f, 1.0f));
	backgroundSprite->SetPosition(0, -30, 0);
	mSceneLayers[Background]->AttachChild(std::move(backgroundSprite));


	std::unique_ptr<Aircraft> leader(new Aircraft(Aircraft::Eagle));
	mPlayerAircraft = leader.get();
	mPlayerAircraft->SetPosition(mSpawnPosition);
	mPlayerAircraft->SetVelocity(XMFLOAT3(1.0f,0.f,0.f));
	mSceneLayers[Air]->AttachChild(std::move(leader));

	std::unique_ptr<Aircraft> leftEscort(new Aircraft(Aircraft::Raptor));
	leftEscort->SetPosition(-20.f, -20.f, 0);
	mPlayerAircraft->AttachChild(std::move(leftEscort));

	std::unique_ptr<Aircraft> rightEscort(new Aircraft( Aircraft::Raptor));
	rightEscort->SetPosition(20.f, -20.f, 0);
	mPlayerAircraft->AttachChild(std::move(rightEscort));

}

void World::AddTexture(Textures::ID id, std::wstring fileName)
{
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(id, fileName));
}


