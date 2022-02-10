
#include "SpriteNode.h"
#include "Game.h"


SpriteNode::SpriteNode()
{
	nodePosition = XMFLOAT3(0, 0, 0);
	nodeRotation = XMFLOAT3(0, 0, 0);
	nodeScale = XMFLOAT3(1, 1, 1);
	renderItem = new RenderItem(nodePosition, nodeRotation, nodeScale,Textures::ID::Desert);
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

SpriteNode::SpriteNode(RenderItem* renderItem)
{
	this->renderItem = renderItem;
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

SpriteNode::SpriteNode(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Textures::ID id)
{
	nodePosition = pos;
	nodeRotation = rot;
	nodeScale = scale;
	renderItem = new RenderItem(nodePosition, nodeRotation, nodeScale, id);
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

void SpriteNode::DrawCurrent(const GameTimer& gt) const
{
	renderItem->Draw(renderItem);
}
