#include "Aircraft.h"
#include "DirectX12Application.h"

Aircraft::Aircraft(Type type)
{
	nodePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 pos = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rot = XMFLOAT3(0, 0, 0);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	mVelocity = XMFLOAT3(2.0f, 0.0f, 0.0f);
	renderItem = new RenderItem(pos, rot, scale, Textures::ID::Eagle);
	static_cast<DirectX12Application*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

Aircraft::Aircraft(RenderItem* renderItem)
{
	nodePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mVelocity = XMFLOAT3(20.0f, 0.0f, 0.0f);
	this->renderItem = renderItem;
	static_cast<DirectX12Application*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

Aircraft::Aircraft(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Type type)
{
	nodePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mVelocity = XMFLOAT3(20.0f, 0.0f, 0.0f);
	renderItem = new RenderItem(pos, rot, scale, ConvertTypeToTexture(type));
	static_cast<DirectX12Application*>(D3DApp::GetApp())->AddRenderItem(renderItem);
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
