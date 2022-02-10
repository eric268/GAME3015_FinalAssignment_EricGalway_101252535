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
	//auto var = mParent;
	nodePosition.x = mParent->GetPosition().x + mVelocity.x * gt.DeltaTime();
	nodePosition.y = mParent->GetPosition().x + mVelocity.y * gt.DeltaTime();
	nodePosition.z = mParent->GetPosition().x + mVelocity.y * gt.DeltaTime();
	SetPosition(nodePosition);
}
