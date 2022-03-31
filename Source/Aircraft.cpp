#include "Aircraft.h"
#include "Game.h"
#include "ResourceManager.h"

Aircraft::Aircraft(Type type)
{
	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
	SetVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f));

	RenderItem* aircraftRenderItem = new RenderItem();

	XMStoreFloat4x4(&aircraftRenderItem->TexTransform, XMMatrixScaling(1, 1, 1));
	auto tempG = static_cast<Game*>(D3DApp::GetApp())->mGeometries["shapeGeo"].get();
	XMStoreFloat4x4(&aircraftRenderItem->World, XMMatrixScaling(GetScale().x, GetScale().y, GetScale().z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(GetRotation().x),
		XMConvertToRadians(GetRotation().y), XMConvertToRadians(GetRotation().z)) * XMMatrixTranslation(GetPosition().x, GetPosition().y + (0.5 * GetScale().y), GetPosition().z));
	aircraftRenderItem->ObjCBIndex = Game::objCBIndex++;
	aircraftRenderItem->Geo = tempG;
	auto searchedMat = ResourceManager::GetInstance()->GetMaterials().find(ConvertTypeToTexture(type));
	aircraftRenderItem->material = searchedMat->second.get();
	aircraftRenderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	aircraftRenderItem->IndexCount = aircraftRenderItem->Geo->DrawArgs["grid"].IndexCount;
	aircraftRenderItem->StartIndexLocation = aircraftRenderItem->Geo->DrawArgs["grid"].StartIndexLocation;
	aircraftRenderItem->BaseVertexLocation = aircraftRenderItem->Geo->DrawArgs["grid"].BaseVertexLocation;


	SetRenderItem(aircraftRenderItem);
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

unsigned int Aircraft::getCategory() const 
{
	switch (mType)
	{
	case Eagle:
		return Category::PlayerAircraft;
	default: return Category::EnemyAircraft;
	}
}

void Aircraft::DrawCurrent(const GameTimer& gt) const
{
	GetRenderItem()->Draw(GetRenderItem());
}
