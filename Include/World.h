#pragma once

#include "../D3DCommon/d3dApp.h"
#include "../D3DCommon/MathHelper.h"
#include "../D3DCommon/UploadBuffer.h"
#include "../D3DCommon/GeometryGenerator.h"
#include "ResourceIdentifiers.h"

using Microsoft::WRL::ComPtr;

class World
{
public:
	World();

	void Update(const GameTimer& gt);
	void Draw(const GameTimer& gt);

	void BuildScene();
	void LoadTextures();
	UINT objIndex;
private:

};


