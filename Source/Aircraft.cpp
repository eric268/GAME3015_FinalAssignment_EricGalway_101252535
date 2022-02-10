#include "Aircraft.h"
#include "Game.h"

Aircraft::Aircraft(Type type)
{
	nodePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	nodeRotation = XMFLOAT3(0, 0, 0);
	nodeScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	renderItem = new RenderItem(nodePosition, nodeRotation, nodeScale, ConvertTypeToTexture(type),1,1);
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

Aircraft::Aircraft(RenderItem* renderItem)
{
	nodePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	nodeRotation = XMFLOAT3(0, 0, 0);
	nodeScale = XMFLOAT3(1, 1, 1);
	mVelocity = XMFLOAT3(20.0f, 0.0f, 0.0f);
	this->renderItem = renderItem;
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

Aircraft::Aircraft(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Type type)
{
	nodePosition = pos;
	nodeRotation = rot;
	nodeScale = scale;
	renderItem = new RenderItem(nodePosition, nodeRotation, nodeScale, ConvertTypeToTexture(type),1,1);
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

Textures::ID Aircraft::ConvertTypeToTexture(Aircraft::Type type)
{
	switch (type)
	{
	case Aircraft::Eagle:
		return Textures::Eagle;

	case Aircraft::Raptor:
		return Textures::Raptor;
	}
	return Textures::Eagle;
}

void Aircraft::DrawCurrent(const GameTimer& gt) const
{
	renderItem->Draw(renderItem);
}
