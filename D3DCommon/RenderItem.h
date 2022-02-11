#pragma once

#include "../D3DCommon/d3dApp.h"
#include "../D3DCommon/MathHelper.h"
#include "../D3DCommon/UploadBuffer.h"
#include "../D3DCommon/GeometryGenerator.h"
#include "ResourceIdentifiers.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class RenderItem
{
public:
	RenderItem();
	RenderItem(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 Scale, Textures::ID matID, float uScaling, float vScaling);
	void Draw(RenderItem* renderItem);
	void UpdateTransform(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale);

	UINT ObjCBIndex;
	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	// DrawIndexedInstanced parameters.
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform;
	int NumFramesDirty = gNumFrameResources;
	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT IndexCount = 0;
	Material* material;
	MeshGeometry* Geo;

};
