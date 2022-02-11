
#include "SpriteNode.h"
#include "Game.h"


SpriteNode::SpriteNode()
{
	SetPosition(0, 0, 0);
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(1, 1, 1));
	SetRenderItem(new RenderItem(GetPosition(), GetRotation(), GetScale(), Textures::ID::Desert, 1, 1));
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(GetRenderItem());
}

SpriteNode::SpriteNode(float uScaling, float vScaling, Textures::ID id)
{
	SetPosition(XMFLOAT3(0, 0, 0));
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(1, 1, 1));
	SetRenderItem( new RenderItem(GetPosition(), GetRotation(), GetScale(), id, uScaling, vScaling));
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(GetRenderItem());
}

SpriteNode::SpriteNode(RenderItem* renderItem)
{
	SetRenderItem(renderItem);
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(renderItem);
}

SpriteNode::SpriteNode(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Textures::ID id)
{
	SetPosition(pos);
	SetRotation(rot);
	SetScale(scale);
	SetRenderItem(new RenderItem(GetPosition(), GetRotation(), GetScale(), id,1,1));
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(GetRenderItem());
}

void SpriteNode::DrawCurrent(const GameTimer& gt) const
{
	GetRenderItem()->Draw(GetRenderItem());
}
