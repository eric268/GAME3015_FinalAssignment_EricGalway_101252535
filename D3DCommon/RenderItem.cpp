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

RenderItem::RenderItem(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, Textures::ID matID, UINT objIndex)
{
	Position = position;
	Rotation = rotation;
	Scale = scale;
	ObjCBIndex = objIndex;
	XMStoreFloat4x4(&World, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)) * XMMatrixTranslation(position.x, position.y + (0.5 * scale.y), position.z));
	
	material = ResourceManager::GetInstance()->GetMaterials()[matID].get();
}

void RenderItem::Draw(RenderItem* renderItem)
{
	static_cast<DirectX12Application*>(D3DApp::GetApp())->DrawRenderItem(this);
}
