#pragma once
#include "ResourceIdentifiers.h"
#include "SceneNode.h"
#include <array>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class World
{
public:
	World();

	void Update(const GameTimer& gt);
	void Draw(const GameTimer& gt);

	void BuildScene();
	void AddTexture(Textures::ID id, std::wstring fileName);
	void LoadTextures();
	UINT objIndex;

private:
	enum Layer
	{
		Background,
		Air,
		LayerCount
	};

private:
	XMFLOAT3 mWorldView;
	SceneNode mSceneGraph;
	std::array<SceneNode*, LayerCount>	mSceneLayers;
	XMFLOAT4 mWorldBounds;
	XMFLOAT3 mSpawnPosition;
	float mScrollSpeed;
};


