#include "Aircraft.h"
#include "Game.h"
#include "ResourceManager.h"

Aircraft::Aircraft(Game* game, Type type) : Entity(game)
{
	mGame = game;

	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRotation(XMFLOAT3(0, 0, 0));
	SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));
	SetVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	mType = type;

	auto render = std::make_unique<RenderItem>();
	renderItem = render.get();

	XMStoreFloat4x4(&renderItem->TexTransform, XMMatrixScaling(1, 1, 1));
	XMStoreFloat4x4(&renderItem->World, XMMatrixScaling(GetScale().x, GetScale().y, GetScale().z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(GetRotation().x),
		XMConvertToRadians(GetRotation().y), XMConvertToRadians(GetRotation().z)) * XMMatrixTranslation(GetPosition().x, GetPosition().y + (0.5 * GetScale().y), GetPosition().z));
	
	renderItem->ObjCBIndex = Game::objCBIndex++;
	renderItem->Geo = game->mGeometries["shapeGeo"].get();
	auto searchedMat = ResourceManager::GetInstance()->GetMaterials().find(ConvertTypeToTexture(type));
	renderItem->material = searchedMat->second.get();


	renderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	renderItem->IndexCount = renderItem->Geo->DrawArgs["grid"].IndexCount;
	renderItem->StartIndexLocation = renderItem->Geo->DrawArgs["grid"].StartIndexLocation;
	renderItem->BaseVertexLocation = renderItem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mAircraftRenderItem = render.get();
	game->GetRenderItems().push_back(std::move(render));
}

Textures::ID Aircraft::ConvertTypeToTexture(Aircraft::Type type)
{
	switch (type)
	{
	case Aircraft::Eagle:
		return Textures::Eagle;

	case Aircraft::Raptor:
		return Textures::Raptor;
	}
	return Textures::Eagle;
}

unsigned int Aircraft::getCategory() const 
{
	switch (mType)
	{
	case Eagle:
		return Category::PlayerAircraft;
	default: 
		return Category::EnemyAircraft;
	}
}

void Aircraft::DrawCurrent(const GameTimer& gt) const
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mGame->mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mGame->mCurrFrameResource->MaterialCB->Resource();

	// For each render item...
	if (mAircraftRenderItem)
	{
		mGame->GetCMDList()->SetPipelineState(mGame->mPSOs["transparent"].Get());

		mGame->GetCMDList()->IASetVertexBuffers(0, 1, &mAircraftRenderItem->Geo->VertexBufferView());
		mGame->GetCMDList()->IASetIndexBuffer(&mAircraftRenderItem->Geo->IndexBufferView());
		mGame->GetCMDList()->IASetPrimitiveTopology(mAircraftRenderItem->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mGame->mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(mAircraftRenderItem->material->DiffuseSrvHeapIndex, mGame->mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + mAircraftRenderItem->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + mAircraftRenderItem->material->MatCBIndex * matCBByteSize;

		mGame->GetCMDList()->SetGraphicsRootDescriptorTable(0, tex);
		mGame->GetCMDList()->SetGraphicsRootConstantBufferView(1, objCBAddress);
		mGame->GetCMDList()->SetGraphicsRootConstantBufferView(3, matCBAddress);

		mGame->GetCMDList()->DrawIndexedInstanced(mAircraftRenderItem->IndexCount, 1, mAircraftRenderItem->StartIndexLocation, mAircraftRenderItem->BaseVertexLocation, 0);
	}
}
