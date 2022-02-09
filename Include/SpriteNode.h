#pragma once
#include "SceneNode.h"
#include "../D3DCommon/RenderItem.h"
class SpriteNode : public SceneNode
{
public:
	SpriteNode();
	SpriteNode(RenderItem* renderItem);
	SpriteNode(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Textures::ID id, UINT objIndex);

private:
	virtual void DrawCurrent(const GameTimer& gt)const;
	RenderItem* renderItem;

};

