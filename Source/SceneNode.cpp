#include "SceneNode.h"
#include "Command.h"
#include <algorithm>
#include <cassert>

SceneNode::SceneNode() :
	mGame(nullptr),
	mChildren(),
	mParent(nullptr),
	nodePosition(XMFLOAT3(0, 0, 0)),
	nodeRotation(XMFLOAT3(0, 0, 0)),
	nodeScale(XMFLOAT3(1, 1, 1))
{
	renderItem = new RenderItem();
}

SceneNode::SceneNode(Game* game) :
	mGame(game),
	mChildren(),
	mParent(nullptr),
	nodePosition(XMFLOAT3(0, 0, 0)),
	nodeRotation(XMFLOAT3(0, 0, 0)),
	nodeScale(XMFLOAT3(1, 1, 1))
{
	renderItem = new RenderItem();
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

DirectX::XMFLOAT3 SceneNode::GetWorldPosition()
{
	XMFLOAT4X4 transform = GetWorldTransform();
	XMFLOAT3 position = XMFLOAT3(transform._41, transform._42, transform._43);
	return position;
}

DirectX::XMFLOAT4X4 SceneNode::GetWorldTransform()
{
	XMFLOAT4X4 t = MathHelper::Identity4x4();

	for (const SceneNode* node = this; node != nullptr; node = node->mParent)
	{
		t = MathHelper::MultiplyXMFLOAT4x4(node->renderItem->World, t);
	}
	return t;
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

void SceneNode::Draw(const GameTimer& gt)
{
	if (mParent != nullptr && renderItem != nullptr)
	{
		renderItem->UpdateTransform(nodePosition, nodeRotation, nodeScale);
		renderItem->World = MathHelper::MultiplyXMFLOAT4x4(mParent->renderItem->World, renderItem->World);
	}
	
	DrawCurrent(gt);
	DrawChildren(gt);
}

void SceneNode::onCommand(const Command& command, GameTimer dt)
{
	if (command.category & getCategory())
		command.action(*this, dt);

	for (Ptr& child : mChildren)
		child->onCommand(command, dt);
}

unsigned int SceneNode::getCategory() const
{
	return Category::Scene;
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

RenderItem* SceneNode::GetRenderItem() const
{
	return renderItem;
}

void SceneNode::SetRenderItem(RenderItem* renderItem)
{
	this->renderItem = renderItem;
}

SceneNode* SceneNode::GetParent() const
{
	return mParent;
}

void SceneNode::DrawCurrent(const GameTimer& gt) const
{
	//Does nothing as of right now
}

void SceneNode::DrawChildren(const GameTimer& gt) const
{
	for (const Ptr& child : mChildren)
	{
		child->Draw(gt);
	}
}