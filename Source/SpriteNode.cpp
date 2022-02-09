#include "SpriteNode.h"
#include "DirectX12Application.h"

SpriteNode::SpriteNode()
{
	XMFLOAT3 pos =XMFLOAT3(0, 0, 0);
	XMFLOAT3 rot = XMFLOAT3(0, 0, 0);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	renderItem = new RenderItem(pos, rot, scale,Textures::ID::Desert, 0);
	static_cast<DirectX12Application*>(D3DApp::GetApp())->BuildRenderItem(renderItem);
}

SpriteNode::SpriteNode(RenderItem* renderItem)
{
	this->renderItem = renderItem;
	static_cast<DirectX12Application*>(D3DApp::GetApp())->BuildRenderItem(renderItem);
}

SpriteNode::SpriteNode(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Textures::ID id, UINT objIndex)
{
	renderItem = new RenderItem(pos, rot, scale, id, objIndex);
	static_cast<DirectX12Application*>(D3DApp::GetApp())->BuildRenderItem(renderItem);
}

void SpriteNode::DrawCurrent(const GameTimer& gt) const
{
	//renderItem->Draw(renderItem);
}
