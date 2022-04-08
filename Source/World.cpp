#include "World.h"
#include <string.h>
#include "ResourceManager.h"
#include "SpriteNode.h"
#include "Game.h"

//World::World() : 
//	screenWidth(0),
//	screenHeight(0),
//	screenWidthBuffer(0.0f),
//	screenHeightBuffer(0.0f),
//	mSpawnPosition(XMFLOAT3(0,0,0)),
//	mScrollSpeed(0.0f),
//	mWorldView(XMFLOAT4X4()),
//	worldViewPosition(),
//	screenToWorldRatio(0),
//	cameraPosition(XMFLOAT3()),
//	changeInPlayerRotation(0.0f),
//	maxSpeed(0.0f),
//	mPhi(0.0f)
//{
//	BuildScene();
//}

World::World(Game* gameWorld) :
	mGame(gameWorld),
	screenWidthBuffer(-10.0f),
	screenHeightBuffer(-75.0f),
	mSpawnPosition(XMFLOAT3(0, -20, -25)),
	mScrollSpeed(5.0f),
	worldViewPosition(),
	screenToWorldRatio(10),
	cameraPosition(XMFLOAT3()),
	changeInPlayerRotation(-30.0f),
	maxSpeed(50.0f),
	mPhi(0.25f * XM_PI),
	mWorldView(mGame->mView),
	mSceneGraph(SceneNode())
{
	BuildScene();
}

void World::Update(const GameTimer& gt)
{
	UpdateCamera(gt);
	mPlayerAircraft->SetVelocity(0.0f, 0.0f, 0.0f);

	while (!mCommandQueue.isEmpty())
	{
		mSceneGraph.onCommand(mCommandQueue.pop(), gt);
	}

	adaptPlayerVelocity();
	mSceneGraph.Update(gt);
	adaptPlayerPosition();
}

void World::Draw(const GameTimer& gt)
{
	mSceneGraph.Draw(gt);
}

void World::LoadTextures()
{
	//AddTexture(Textures::ID::Desert, L"Media/Desert.dds");
	//AddTexture(Textures::ID::Eagle, L"Media/Eagle3.dds");
	//AddTexture(Textures::ID::Raptor, L"Media/Raptor.dds");
	//AddTexture(Textures::ID::TitleScreen, L"Media/TitleScreen.dds");
}

XMFLOAT4X4 World::GetWorldView()
{
	return mWorldView;
}

void World::SetWorldView(XMFLOAT4X4& view)
{
	mWorldView = view;
	cameraPosition = MathHelper::GetPosition(mWorldView);
}

void World::adaptPlayerPosition()
{
	XMFLOAT3 position = mPlayerAircraft->GetPosition();

	position.x = min(position.x, mGame->GetClientWidth() / (2.0f * screenToWorldRatio) - screenWidthBuffer);
	position.x = max(position.x, -mGame->GetClientWidth() / (2.0f * screenToWorldRatio) + screenWidthBuffer);
	position.z = max(position.z, (-cameraPosition.y *1.4f)  - screenHeight / (2.f * screenToWorldRatio));
	position.z = min(position.z, -(cameraPosition.y * 1.4f) + screenHeight / (2.f * screenToWorldRatio) - screenHeightBuffer);

	mPlayerAircraft->SetPosition(position);
}

void World::adaptPlayerVelocity()
{
	XMFLOAT3 velocity = mPlayerAircraft->GetVelocity();

	if (velocity.x != 0.0f && velocity.z != 0.0f)
	{
		mPlayerAircraft->SetVelocity(velocity.x / std::sqrt(2.0f), 0.0f, velocity.z / std::sqrt(2.0f));
		velocity = mPlayerAircraft->GetVelocity();
	}

	mPlayerAircraft->accelerate(0.0f, mScrollSpeed);
	mPlayerAircraft->SetRotation(mPlayerAircraft->GetVelocity().z / 3.0f, 0, -mPlayerAircraft->GetVelocity().x / 3.0f);
}

Game* World::GetGame()
{
	return mGame;
}

void World::UpdateCamera(const GameTimer& gt)
{
	cameraPosition.y -= mScrollSpeed * sin(mPhi) *gt.DeltaTime();
	cameraPosition.z -= mScrollSpeed * sin(mPhi) * gt.DeltaTime();
	MathHelper::UpdatePosition(mWorldView, cameraPosition);
}

void World::BuildScene()
{
	//// Initialize the different layers
	//for (std::size_t i = 0; i < SceneNode::LayerCount; ++i)
	//{
	//	SceneNode::Ptr layer(new SceneNode());
	//	mSceneLayers[i] = layer.get();
	//	mSceneGraph.AttachChild(std::move(layer));
	//}
	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 3, 1500, Textures::ID::Desert));
	backgroundSprite->SetScale(XMFLOAT3(10.0f, 1.0f, 2000.0f));
	backgroundSprite->SetPosition(0, -100, 0);
	mSceneGraph.AttachChild(std::move(backgroundSprite));
	//mSceneLayers[SceneNode::Background]->AttachChild(std::move(backgroundSprite));

	std::unique_ptr<Aircraft> leader(new Aircraft(mGame, Aircraft::Eagle));
	mPlayerAircraft = leader.get();
	mPlayerAircraft->SetPosition(0,0,0);
	mPlayerAircraft->SetScale(0.1f, 0.1f, 0.1f);
	mPlayerAircraft->SetVelocity(XMFLOAT3(0.0f, 0, mScrollSpeed));
	mSceneGraph.AttachChild(std::move(leader));
	//mSceneLayers[SceneNode::Air]->AttachChild(std::move(leader));

	std::unique_ptr<Aircraft> leftEscort(new Aircraft(mGame, Aircraft::Raptor));
	leftEscort->SetPosition(-25.f, 0.0f, -25);
	leftEscort->SetScale(1, 1, 1);
	mPlayerAircraft->AttachChild(std::move(leftEscort));

	std::unique_ptr<Aircraft> rightEscort(new Aircraft(mGame, Aircraft::Raptor));
	rightEscort->SetPosition(25.f, 0.0f, -25);
	rightEscort->SetScale(1, 1, 1);
	mPlayerAircraft->AttachChild(std::move(rightEscort));
}

void World::AddTexture(Textures::ID id, std::wstring fileName)
{
	ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(id, fileName));
}

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}


