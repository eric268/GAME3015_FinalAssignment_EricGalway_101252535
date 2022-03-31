#include "World.h"
#include <string.h>
#include "ResourceManager.h"
#include "SpriteNode.h"

World::World() : 
	screenWidth(0),
	screenHeight(0),
	screenWidthBuffer(0.0f),
	screenHeightBuffer(0.0f),
	mSpawnPosition(XMFLOAT3(0,0,0)),
	mScrollSpeed(0.0f),
	mWorldView(XMFLOAT4X4()),
	worldViewPosition(),
	screenToWorldRatio(0),
	cameraPosition(XMFLOAT3()),
	changeInPlayerRotation(0.0f)
{
	LoadTextures();
}

World::World( float width, float height) :
	//mWorldView(mWorldView),
	screenWidth(width),
	screenHeight(height),
	screenWidthBuffer(25.0f),
	screenHeightBuffer(25.0f),
	mSpawnPosition(XMFLOAT3(0, -20, -25)),
	mScrollSpeed(5.0f),
	worldViewPosition(),
	screenToWorldRatio(10),
	cameraPosition(XMFLOAT3()),
	changeInPlayerRotation(-30.0f)
{
		LoadTextures();
}

void World::Update(const GameTimer& gt)
{

	//UpdateCamera(gt);
	mPlayerAircraft->SetVelocity(0.0f, 0.0f, 0.0f);
	
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), gt);
	
	//ManagePlayerPosition();
	//adaptPlayerVelocity();
	mSceneGraph.Update(gt);
	//adaptPlayerPosition();
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
	//position.x = max(position.x * screenToWorldRatio, screenWidth/2.0f - screenWidthBuffer);
	//position.x = min(position.x * screenToWorldRatio, -screenWidth/2.0f + screenWidthBuffer);
	position.y = min(position.y * screenToWorldRatio, screenHeight/2.0f - screenHeightBuffer);
	position.y = max(position.y * screenToWorldRatio, -screenHeight/2.0f  + screenHeightBuffer);

	mPlayerAircraft->SetPosition(position);
}

void World::adaptPlayerVelocity()
{
	XMFLOAT3 velocity = mPlayerAircraft->GetVelocity();
	
	if (velocity.x != 0.0f && velocity.y != 0.0f)
	{
		mPlayerAircraft->SetVelocity(velocity.x / std::sqrt(2.0f), velocity.y / std::sqrt(2.0f), 0.0f);
	}

	mPlayerAircraft->accelerate(0.0f, mScrollSpeed);
}

void World::ManagePlayerPosition()
{
	XMFLOAT3 position = mPlayerAircraft->GetPosition();
	XMFLOAT3 velocity = mPlayerAircraft->GetVelocity();

	if (position.x * screenToWorldRatio <= -screenWidth/2.0f + screenWidthBuffer || position.x * screenToWorldRatio >=  screenWidth/2.0f - screenWidthBuffer)
	{
		changeInPlayerRotation *= -1;
		velocity.x = -velocity.x;
		mPlayerAircraft->SetVelocity(velocity);
		mPlayerAircraft->SetRotation(0, 0, changeInPlayerRotation);

		//Calculates how far outside of the bounds the aircraft is and updates its position appropriately 
		int sign = (velocity.x < 0) ? -1 : 1;
		float offset = abs(position.x * screenToWorldRatio - (-screenWidth / 2.0f + screenWidthBuffer) * sign);
		float positionOffset = offset * sign;
		mPlayerAircraft->SetPosition(position.x + positionOffset, position.y, position.z);
	}
}

void World::UpdateCamera(const GameTimer& gt)
{
	cameraPosition.y -= mScrollSpeed * gt.DeltaTime();
	MathHelper::UpdatePosition(mWorldView, cameraPosition);
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
	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(5,1500, Textures::ID::Desert));
	backgroundSprite->SetScale(XMFLOAT3(10.0f, 1.0f, 2000.0f));
	backgroundSprite->SetPosition(0, -100, 0);
	mSceneLayers[Background]->AttachChild(std::move(backgroundSprite));

	std::unique_ptr<Aircraft> leader(new Aircraft(Aircraft::Eagle));
	mPlayerAircraft = leader.get();
	mPlayerAircraft->SetPosition(mSpawnPosition);
	mPlayerAircraft->SetScale(0.1f, 0.1f, 0.1f);
	mPlayerAircraft->SetVelocity(XMFLOAT3(15.0f, 0, mScrollSpeed));
	mSceneLayers[Air]->AttachChild(std::move(leader));

	std::unique_ptr<Aircraft> leftEscort(new Aircraft(Aircraft::Raptor));
	leftEscort->SetPosition(-25.f, 0.0f, -10);
	mPlayerAircraft->AttachChild(std::move(leftEscort));

	std::unique_ptr<Aircraft> rightEscort(new Aircraft(Aircraft::Raptor));
	rightEscort->SetPosition(25.f, 0.0f, -10);
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


