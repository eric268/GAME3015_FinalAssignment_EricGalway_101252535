#include "SceneNode.h"

SceneNode::SceneNode() : 
	mChildren(),
	mParent(nullptr), 
	nodePosition(XMFLOAT3(0,0,0)),
	nodeRotation(XMFLOAT3(0,0,0)),
	nodeScale(XMFLOAT3(1,1,1))
{
}

void SceneNode::AttachChild(Ptr child)
{
	child->mParent = this;
	mChildren.push_back(std::move(child));
}

SceneNode::Ptr SceneNode::detachChild(const SceneNode& node)
{
	auto found = std::find_if(mChildren.begin(), mChildren.end(), [&](Ptr& p) { return p.get() == &node; });
	assert(found != mChildren.end());

	Ptr result = std::move(*found);
	result->mParent = nullptr;
	mChildren.erase(found);
	return result;
}

void SceneNode::Update(const GameTimer& gt)
{
	UpdateCurrent(gt);
	UpdateChildren(gt);
}

DirectX::XMFLOAT3 SceneNode::GetWorldPosition() const
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT4X4 SceneNode::GetWorldTransform()const
{
	return DirectX::XMFLOAT4X4();
}

void SceneNode::UpdateCurrent(const GameTimer& gt)
{
}

void SceneNode::UpdateChildren(const GameTimer& gt)
{
	for (Ptr& child : mChildren)
	{
		child->Update(gt);
	}
}

void SceneNode::Draw(const GameTimer& gt) const
{
	DrawCurrent(gt);
	DrawChildren(gt);
}

void SceneNode::SetPosition(XMFLOAT3 position)
{
	nodePosition = position;
	renderItem->UpdateTransform(nodePosition, nodeRotation, nodeScale);
}

void SceneNode::SetRotation(XMFLOAT3 rotation)
{
	nodeRotation = rotation;
	renderItem->UpdateTransform(nodePosition, nodeRotation, nodeScale);
}

void SceneNode::SetScale(XMFLOAT3 scale)
{
	nodeScale = scale;
	renderItem->UpdateTransform(nodePosition, nodeRotation, nodeScale);
}

void SceneNode::SetPosition(float x, float y, float z)
{
	XMFLOAT3 pos = XMFLOAT3(x, y, z);
	nodePosition = pos;
	renderItem->UpdateTransform(nodePosition, nodeRotation, nodeScale);
}

void SceneNode::SetRotation(float x, float y, float z)
{
	XMFLOAT3 rot = XMFLOAT3(x, y, z);
	nodeRotation = rot;
	renderItem->UpdateTransform(nodePosition, nodeRotation, nodeScale);
}

void SceneNode::SetScale(float x, float y, float z)
{
	XMFLOAT3 scale = XMFLOAT3(x, y, z);
	nodeScale = scale;
	renderItem->UpdateTransform(nodePosition, nodeRotation, nodeScale);
}

XMFLOAT3 SceneNode::GetPosition()
{
	return nodePosition;
}

XMFLOAT3 SceneNode::GetRotation()
{
	return nodeRotation;
}

XMFLOAT3 SceneNode::GetScale()
{
	return nodeScale;
}

void SceneNode::DrawCurrent(const GameTimer& gt) const
{
}

void SceneNode::DrawChildren(const GameTimer& gt) const
{
	for (const Ptr& child : mChildren)
	{
		child->Draw(gt);
	}
}