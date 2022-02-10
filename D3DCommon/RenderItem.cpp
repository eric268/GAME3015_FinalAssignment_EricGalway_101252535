#include "RenderItem.h"
#include "DirectX12Application.h"
#include "ResourceManager.h"

RenderItem::RenderItem() : World(),
Position(),
Rotation(),
Scale()
{
;
}

RenderItem::RenderItem(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, Textures::ID matID)
{
	Position = position;
	Rotation = rotation;
	Scale = scale;
	auto tempG = static_cast<DirectX12Application*>(D3DApp::GetApp())->mGeometries["shapeGeo"].get();
	XMStoreFloat4x4(&World, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
	XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)) * XMMatrixTranslation(position.x, position.y + (0.5 * scale.y), position.z));
	ObjCBIndex = DirectX12Application::objCBIndex++;
	Geo = tempG;

	auto searchedMat = ResourceManager::GetInstance()->GetMaterials().find(matID);
	material = searchedMat->second.get();
	PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	IndexCount = Geo->DrawArgs["grid"].IndexCount;
	StartIndexLocation = Geo->DrawArgs["grid"].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs["grid"].BaseVertexLocation;
}

void RenderItem::Draw(RenderItem* renderItem)
{
	static_cast<DirectX12Application*>(D3DApp::GetApp())->AddRenderItem(this);
}

void RenderItem::UpdateTransform(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	XMStoreFloat4x4(&World, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)) * XMMatrixTranslation(position.x, position.y, position.z));
}