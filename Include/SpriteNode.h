#pragma once
#include "SceneNode.h"	

class SpriteNode : public SceneNode
{
public:
	SpriteNode();
	SpriteNode(RenderItem* renderItem);
	SpriteNode(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Textures::ID id);

private:
	virtual void DrawCurrent(const GameTimer& gt)const;
};

