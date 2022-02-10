#include "World.h"
#include <string.h>
#include "ResourceManager.h"
#include "SpriteNode.h"

World::World() : 
	mSpawnPosition(XMFLOAT3(0,-20,-20)),
	mScrollSpeed(2.0f),
	mWorldView(XMFLOAT4X4()),
	worldViewPosition()
{
	LoadTextures();
}

World::World(XMFLOAT4X4 mWorldView) : 
	mWorldView(mWorldView),
	mSpawnPosition(XMFLOAT3(30,20,20)),
	mScrollSpeed(2.0f),
	worldViewPosition()
{
	LoadTextures();
}

void World::Update(const GameTimer& gt)
{
	mSceneGraph.Update(gt);
	UpdateCameraViewPosition(gt);
}

void World::Draw(const GameTimer& gt)
{
	mSceneGraph.Draw(gt);
}

void World::LoadTextures()
{
	AddTexture(Textures::ID::Desert, L"Media/Desert.dds");
	AddTexture(Textures::ID::Eagle, L"Media/Eagle3.dds");
	AddTexture(Textures::ID::Raptor, L"Media/Raptor.dds");
}

void World::UpdateCameraViewPosition(const GameTimer& gt)
{
	//worldViewPosition.y += mScrollSpeed * gt.DeltaTime();
	//mWorldView._42 -= worldViewPosition.y;
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
	backgroundSprite->SetScale(XMFLOAT3(10.0f, 1.0f, 200.0f));
	backgroundSprite->SetPosition(0, -100, 0);
	mSceneLayers[Background]->AttachChild(std::move(backgroundSprite));
	//XMFLOAT4X4 temp = backgroundSprite->renderItem->TexTransform;


	std::unique_ptr<Aircraft> leader(new Aircraft(Aircraft::Eagle));
	mPlayerAircraft = leader.get();
	mPlayerAircraft->SetPosition(mSpawnPosition);
	mPlayerAircraft->SetScale(0.1f, 0.1f, 0.1f);
	mPlayerAircraft->SetVelocity(XMFLOAT3(1.0f, 0, mScrollSpeed));
	mSceneLayers[Air]->AttachChild(std::move(leader));

	std::unique_ptr<Aircraft> leftEscort(new Aircraft(Aircraft::Raptor));
	leftEscort->SetPosition(-25.f, 0.f, -10);
	mPlayerAircraft->AttachChild(std::move(leftEscort));

	std::unique_ptr<Aircraft> rightEscort(new Aircraft( Aircraft::Raptor));
	rightEscort->SetPosition(25.f, 0.f, -10);
	mPlayerAircraft->AttachChild(std::move(rightEscort));

}

void World::AddTexture(Textures::ID id, std::wstring fileName)
{
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(id, fileName));
}


