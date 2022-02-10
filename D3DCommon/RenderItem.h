#pragma once
//#include "DirectX12Application.h"
#include "Game.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class RenderItem
{
public:
	RenderItem();
	RenderItem(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 Scale, Textures::ID matID);
	void Draw(RenderItem* renderItem);

	UINT ObjCBIndex = -1;
	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT3 Position = XMFLOAT3(World._11, World._12, World._13);
	XMFLOAT3 Rotation = XMFLOAT3(World._21, World._22, World._23);
	XMFLOAT3 Scale = XMFLOAT3(World._31, World._32, World._33);

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT IndexCount = 0;
	Material* material;
	MeshGeometry* Geo;
};
