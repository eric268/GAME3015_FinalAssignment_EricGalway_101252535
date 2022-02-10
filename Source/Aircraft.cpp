#include "Aircraft.h"
#include "DirectX12Application.h"

Aircraft::Aircraft(Type type)
{
	XMFLOAT3 pos = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rot = XMFLOAT3(0, 0, 0);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	renderItem = new RenderItem(pos, rot, scale, Textures::ID::Eagle);
	static_cast<DirectX12Application*>(D3DApp::GetApp())->BuildRenderItem(renderItem);
}

Aircraft::Aircraft(RenderItem* renderItem)
{
	this->renderItem = renderItem;
	static_cast<DirectX12Application*>(D3DApp::GetApp())->BuildRenderItem(renderItem);
}

Aircraft::Aircraft(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Type type)
{
	renderItem = new RenderItem(pos, rot, scale, ConvertTypeToTexture(type));
	static_cast<DirectX12Application*>(D3DApp::GetApp())->BuildRenderItem(renderItem);
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
