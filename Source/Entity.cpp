#include "Entity.h"

Entity::Entity(Game* game) : SceneNode(game)
{
}

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
	//Adds the velocity to the Entities current position and updates the position
	float x = GetPosition().x + mVelocity.x * gt.DeltaTime();
	float y = GetPosition().y + mVelocity.y * gt.DeltaTime();
	float z = GetPosition().z + mVelocity.z * gt.DeltaTime();

	SceneNode::SetPosition(x,y,z);
}

void Entity::accelerate(XMFLOAT2 velocity)
{
	mVelocity.x += velocity.x;
	mVelocity.z += velocity.y;
}

void Entity::accelerate(float vx, float vy)
{
	mVelocity.x += vx;
	mVelocity.z += vy;
}
