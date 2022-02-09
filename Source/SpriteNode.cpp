#include "SpriteNode.h"

SpriteNode::SpriteNode()
{
	XMFLOAT3 pos =XMFLOAT3(0, 0, 0);
	XMFLOAT3 rot = XMFLOAT3(0, 0, 0);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	renderItem = new RenderItem(pos, rot, scale, Textures::Desert, 0);
}

SpriteNode::SpriteNode(RenderItem* renderItem)
{
	this->renderItem = renderItem;
}

SpriteNode::SpriteNode(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Textures::ID id, UINT objIndex)
{
	renderItem = new RenderItem(pos, rot, scale, id, objIndex);
}

void SpriteNode::DrawCurrent(const GameTimer& gt) const
{
	renderItem->Draw(renderItem);
}
