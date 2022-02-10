#include "SceneNode.h"

SceneNode::SceneNode() : mChildren(), mParent(nullptr), nodePosition()
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

void SceneNode::TranslateWorldPos(XMFLOAT3 position, XMFLOAT3 scale, XMFLOAT3 rotation)
{

		XMStoreFloat4x4(&renderItem->World, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
			XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)) * XMMatrixTranslation(position.x, position.y + (0.5 * scale.y), position.z));
	
}
