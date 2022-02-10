#pragma once
#include <vector>
#include <memory>
#include "../D3DCommon/d3dApp.h"
#include "../D3DCommon/MathHelper.h"
#include "../D3DCommon/UploadBuffer.h"
#include "../D3DCommon/GeometryGenerator.h"
#include "../D3DCommon/RenderItem.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class SceneNode
{
public:
	typedef std::shared_ptr<SceneNode> Ptr;

	SceneNode();

	void AttachChild(Ptr child);
	Ptr detachChild(const SceneNode& node);

	void Update(const GameTimer& gt);

	DirectX::XMFLOAT3 GetWorldPosition();
	DirectX::XMFLOAT4X4 GetWorldTransform();
	virtual void Draw(const GameTimer& gt);


	void SetPosition(XMFLOAT3 position);
	void SetRotation(XMFLOAT3 rotation);
	void SetScale(XMFLOAT3 scale);

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();
	XMFLOAT3 GetScale();

	RenderItem* renderItem;
	XMFLOAT3 nodePosition;
	XMFLOAT3 nodeRotation;
	XMFLOAT3 nodeScale;
	SceneNode* mParent;

	void SetTransform(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale);
	XMFLOAT4X4 GetTransform();
	XMFLOAT4X4 transform;


private:
	virtual void UpdateCurrent(const GameTimer& gt);
	void UpdateChildren(const GameTimer& gt);

	virtual void DrawCurrent(const GameTimer& gt) const;
	void DrawChildren(const GameTimer& gt) const;

	std::vector<Ptr> mChildren;

};

