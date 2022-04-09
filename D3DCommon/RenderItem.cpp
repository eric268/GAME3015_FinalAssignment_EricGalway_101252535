#include "RenderItem.h"
#include "Game.h"
#include "ResourceManager.h"

RenderItem::RenderItem()
{
}

void RenderItem::UpdateTransform(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	XMStoreFloat4x4(&World, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)) * XMMatrixTranslation(position.x, position.y, position.z));
}

void RenderItem::UpdatePosition(XMFLOAT3 pos)
{
	XMStoreFloat4x4(&World, XMMatrixTranslation(pos.x, pos.y, pos.z));
}

void RenderItem::UpdateRotation(XMFLOAT3 rot)
{
	XMStoreFloat4x4(&World,XMMatrixRotationRollPitchYaw(XMConvertToRadians(rot.x),
		XMConvertToRadians(rot.y), XMConvertToRadians(rot.z)) );
}

void RenderItem::UpdateScale(XMFLOAT3 scale)
{
	XMStoreFloat4x4(&World, XMMatrixScaling(scale.x, scale.y, scale.z));
}

