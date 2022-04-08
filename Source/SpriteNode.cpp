
#include "SpriteNode.h"
#include "Game.h"
#include "ResourceManager.h"


SpriteNode::SpriteNode(Game* game)
{

}

SpriteNode::SpriteNode(Game* game, float uScaling, float vScaling, Textures::ID id) : mGame(game)
{
	SetPosition(XMFLOAT3(0, -10, 0));
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(1, 1, 1));

	auto render = std::make_unique<RenderItem>();
	renderItem = render.get();

	XMStoreFloat4x4(&renderItem->TexTransform, XMMatrixScaling(uScaling, vScaling, 1));
	XMStoreFloat4x4(&renderItem->World, XMMatrixScaling(GetScale().x, GetScale().y, GetScale().z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(GetRotation().x),
		XMConvertToRadians(GetRotation().y), XMConvertToRadians(GetRotation().z)) * XMMatrixTranslation(GetPosition().x, GetPosition().y + (0.5 * GetScale().y), GetPosition().z));
	
	renderItem->ObjCBIndex = Game::objCBIndex++;
	renderItem->Geo = game->mGeometries["shapeGeo"].get();;
	auto searchedMat = ResourceManager::GetInstance()->GetMaterials().find(id);
	renderItem->material = searchedMat->second.get();
	
	renderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	renderItem->IndexCount = renderItem->Geo->DrawArgs["grid"].IndexCount;
	renderItem->StartIndexLocation = renderItem->Geo->DrawArgs["grid"].StartIndexLocation;
	renderItem->BaseVertexLocation = renderItem->Geo->DrawArgs["grid"].BaseVertexLocation;
	
	mSpriteNodeRenderItem = render.get();
	mGame->GetRenderItems().push_back(std::move(render));
}

void SpriteNode::DrawCurrent(const GameTimer& gt) const
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mGame->mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mGame->mCurrFrameResource->MaterialCB->Resource();

	if (mSpriteNodeRenderItem)
	{

		mGame->GetCMDList()->IASetVertexBuffers(0, 1, &mSpriteNodeRenderItem->Geo->VertexBufferView());
		mGame->GetCMDList()->IASetIndexBuffer(&mSpriteNodeRenderItem->Geo->IndexBufferView());
		mGame->GetCMDList()->IASetPrimitiveTopology(mSpriteNodeRenderItem->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mGame->mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(mSpriteNodeRenderItem->material->DiffuseSrvHeapIndex, mGame->mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + mSpriteNodeRenderItem->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + mSpriteNodeRenderItem->material->MatCBIndex * matCBByteSize;

		mGame->GetCMDList()->SetGraphicsRootDescriptorTable(0, tex);
		mGame->GetCMDList()->SetGraphicsRootConstantBufferView(1, objCBAddress);
		mGame->GetCMDList()->SetGraphicsRootConstantBufferView(3, matCBAddress);

		mGame->GetCMDList()->DrawIndexedInstanced(mSpriteNodeRenderItem->IndexCount, 1, mSpriteNodeRenderItem->StartIndexLocation, mSpriteNodeRenderItem->BaseVertexLocation, 0);
		GetRenderItem()->NumFramesDirty++;
	}
}
