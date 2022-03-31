
#include "SpriteNode.h"
#include "Game.h"
#include "ResourceManager.h"


SpriteNode::SpriteNode()
{

}

SpriteNode::SpriteNode(float uScaling, float vScaling, Textures::ID id)
{
	SetPosition(XMFLOAT3(0, 0, 0));
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(1, 1, 1));

	RenderItem* spriteNodeRenderItem = new RenderItem();

	XMStoreFloat4x4(&spriteNodeRenderItem->TexTransform, XMMatrixScaling(uScaling, vScaling, 1));
	auto tempG = static_cast<Game*>(D3DApp::GetApp())->mGeometries["shapeGeo"].get();
	XMStoreFloat4x4(&spriteNodeRenderItem->World, XMMatrixScaling(GetScale().x, GetScale().y, GetScale().z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(GetRotation().x),
		XMConvertToRadians(GetRotation().y), XMConvertToRadians(GetRotation().z)) * XMMatrixTranslation(GetPosition().x, GetPosition().y + (0.5 * GetScale().y), GetPosition().z));
	spriteNodeRenderItem->ObjCBIndex = Game::objCBIndex++;
	spriteNodeRenderItem->Geo = tempG;
	auto searchedMat = ResourceManager::GetInstance()->GetMaterials().find(id);
	spriteNodeRenderItem->material = searchedMat->second.get();
	spriteNodeRenderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	spriteNodeRenderItem->IndexCount = spriteNodeRenderItem->Geo->DrawArgs["grid"].IndexCount;
	spriteNodeRenderItem->StartIndexLocation = spriteNodeRenderItem->Geo->DrawArgs["grid"].StartIndexLocation;
	spriteNodeRenderItem->BaseVertexLocation = spriteNodeRenderItem->Geo->DrawArgs["grid"].BaseVertexLocation;
	
	SetRenderItem(spriteNodeRenderItem);
	static_cast<Game*>(D3DApp::GetApp())->AddRenderItem(GetRenderItem());
}

void SpriteNode::DrawCurrent(const GameTimer& gt) const
{
	GetRenderItem()->Draw(GetRenderItem());
}
