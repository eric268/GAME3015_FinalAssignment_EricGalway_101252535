#include "Aircraft.h"
#include "Game.h"

Aircraft::Aircraft(Type type)
{
	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
	SetVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRenderItem(new RenderItem(GetPosition(), GetRotation(), GetScale(), ConvertTypeToTexture(type), 1, 1));
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(GetRenderItem());
}

Aircraft::Aircraft(RenderItem* renderItem)
{
	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(1, 1, 1));
	SetVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRenderItem(renderItem);
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

Aircraft::Aircraft(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Type type)
{
	SetPosition(pos);
	SetRotation(rot);
	SetScale(scale);
	SetRenderItem(new RenderItem(GetPosition(), GetRotation(), GetScale(), ConvertTypeToTexture(type), 1, 1));
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(GetRenderItem());
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
	GetRenderItem()->Draw(GetRenderItem());
}
