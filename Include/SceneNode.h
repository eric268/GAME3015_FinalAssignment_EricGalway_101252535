#pragma once
#include <vector>
#include <memory>
#include "../D3DCommon/d3dApp.h"
#include "../D3DCommon/MathHelper.h"
#include "../D3DCommon/UploadBuffer.h"
#include "../D3DCommon/GeometryGenerator.h"
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

	DirectX::XMFLOAT3 GetWorldPosition()const;
	DirectX::XMFLOAT4X4 GetWorldTransform() const;

private:
	virtual void UpdateCurrent(const GameTimer& gt);
	void UpdateChildren(const GameTimer& gt);
	virtual void Draw(const GameTimer& gt) const;
	virtual void DrawCurrent(const GameTimer& gt);


	void DrawChildren(const GameTimer& gt);

	std::vector<Ptr> mChildren;
	SceneNode* mParent;
};

