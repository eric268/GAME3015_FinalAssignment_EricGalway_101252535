#pragma once
#include "SceneNode.h"
class Entity : public SceneNode
{
public:
	void SetVelocity(XMFLOAT3 vel);
	void SetVelocity(float x, float y, float z);
	XMFLOAT3 GetVelocity() const;

	virtual void UpdateCurrent(const GameTimer& gt) override;
	virtual void Move(const GameTimer& gt);
	XMFLOAT3 mVelocity;
private:
};

