#include "Entity.h"

void Entity::SetVelocity(XMFLOAT3 vel)
{
	mVelocity = vel;
}

void Entity::SetVelocity(float x, float y, float z)
{
	mVelocity = XMFLOAT3(x, y, z);
}

XMFLOAT3 Entity::GetVelocity() const
{
	return mVelocity;
}

void Entity::UpdateCurrent(const GameTimer& gt)
{
	Move(gt);
}

void Entity::Move(const GameTimer& gt)
{
	nodePosition.x += mVelocity.x * gt.DeltaTime();
	nodePosition.y += mVelocity.y * gt.DeltaTime();

	//renderItem->TranslateWorldPos(nodePosition);
	TranslateWorldPos(XMFLOAT3(nodePosition.x, nodePosition.y, 0), XMFLOAT3(0.005, 0.005, 0.005), XMFLOAT3(1, 0, 0));
	


	//IDirect3DDevice9::SetTransform()
}
