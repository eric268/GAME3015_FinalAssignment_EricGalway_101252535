#pragma once
#include "ResourceIdentifiers.h"
#include "SceneNode.h"
#include "Aircraft.h"
#include <array>


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class World
{
public:
	World();
	World(float width, float height);
	//World(XMFLOAT4X4& mWorldView);

	void Update(const GameTimer& gt);
	void Draw(const GameTimer& gt);

	void BuildScene();
	void AddTexture(Textures::ID id, std::wstring fileName);
	void LoadTextures();
	UINT objIndex;
	XMFLOAT4X4 mWorldView;
private:
	enum Layer
	{
		Background,
		Air,
		LayerCount
	};

private:
	SceneNode mSceneGraph;
	std::array<SceneNode*, LayerCount>	mSceneLayers;
	XMFLOAT4 mWorldBounds;
	XMFLOAT3 mSpawnPosition;
	float mScrollSpeed;
	Aircraft* mPlayerAircraft;
	XMFLOAT3 worldViewPosition;

	void ManagePlayerPosition();

	float screenWidth;
	float screenHeight;
};


