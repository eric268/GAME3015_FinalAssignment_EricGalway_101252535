#pragma once
#include "SceneNode.h"
#include "../D3DCommon/RenderItem.h"
class Entity : public SceneNode
{
public:

	void SetVelocity(XMFLOAT3 vel);
	void SetVelocity(float x, float y, float z);
	XMFLOAT3 GetVelocity() const;

	virtual void UpdateCurrent(const GameTimer& gt);
	virtual void Move(const GameTimer& gt);
	XMFLOAT3 mVelocity;
	RenderItem* renderItem;
private:
};

