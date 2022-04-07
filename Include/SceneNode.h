#pragma once
#include <vector>
#include <memory>
#include "../D3DCommon/d3dApp.h"
#include "../D3DCommon/MathHelper.h"
#include "../D3DCommon/UploadBuffer.h"
#include "../D3DCommon/GeometryGenerator.h"
#include "../D3DCommon/RenderItem.h"
#include "Category.h"

struct Command;
class Game;

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

/// Class which all scene objects will inherit from 
/// 
/// Class that contains the scene node which allows for all objects to be added as children
/// This allows for objects to be drawn and updated correctly
/// This class will be overridden by child classes the majority of the time</summary>
class SceneNode
{
public:
	typedef std::shared_ptr<SceneNode> Ptr;
	Game* mGame;

	SceneNode();
	/// <summary>Default constructor for Scene Node
	SceneNode(Game* game);

	/// <summary>Attaches scene nodes as children
	/// 
	/// Allows for scene nodes of classes that inherit from scene node to be attached as children
	/// This allows for them to be updated appropriately</summary>
	/// <param name="child">Scene node pointer</param>
	void AttachChild(Ptr child);

	/// <summary> Detached child from scene node

	/// <param name="node">Reference to scene node</param>
	/// <returns>Scene node pointer</returns>
	Ptr detachChild(const SceneNode& node);

	/// Updates the Scene Node based on delta time
	/// 
	/// <param name="gt">Delta Time</param>
	void Update(const GameTimer& gt);

	/// <summary>Function that will add the node to the game.cpp container which draws all render items
	/// 
	/// <param name="gt">Delta Time</param>
	virtual void Draw(const GameTimer& gt);

	/// <summary>
	/// Performs action if command correctly matches the category
	/// </summary>
	/// <param name="command">Command to check if category matches and if true performs respective action</param>
	/// <param name="dt">Delta time</param>
	void onCommand(const Command& command, GameTimer dt);

	/// <summary>
	/// Returns the current category
	/// </summary>
	/// <returns>A Type from the Category namespace</returns>
	virtual unsigned int getCategory() const;
	
	
	
	//Getters


	/// <summary> Returns the World Position of the node
	/// <returns>XMFLOAT3 representing world position</returns>
	DirectX::XMFLOAT3 GetWorldPosition();

	/// <summary> Returns the transform of the Scene Node
	/// <returns>XMFLOAT4X4 Representing the transform of the scene node</returns>
	DirectX::XMFLOAT4X4 GetWorldTransform();

	/// <summary> Returns the position
	/// <returns>XMFLOAT3 representing position</returns>
	XMFLOAT3 GetPosition();

	/// <summary> Returns the rotation
/// <returns>XMFLOAT3 representing rotation</returns>
	XMFLOAT3 GetRotation();

	/// <summary> Returns the scale
	/// <returns>XMFLOAT3 representing scale</returns>
	XMFLOAT3 GetScale();

	/// <summary> Returns the Render Item
	/// <returns>Render Item pointer which representing the objects draw properties</returns>
	RenderItem* GetRenderItem() const;

	/// <summary>Returns the parent of the current scene node
	/// <returns>Scene node pointer</returns>
	SceneNode* GetParent() const;


	//Setters


	/// <summary>Sets the position of the scene node
	/// 
	/// Sets the position to a value of XMFLOAT3</summary>
	/// <param name="position"> XMFLOAT3</param>
	void SetPosition(XMFLOAT3 position);

	/// <summary>Sets the roation of the scene node
	/// 
	/// Sets the rotation to a value of XMFLOAT3</summary>
	/// <param name="rotation"> XMFLOAT3</param>
	void SetRotation(XMFLOAT3 rotation);

	/// <summary>Sets the scale of the scene node
	/// 
	/// Sets the scale to a value of XMFLOAT3</summary>
	/// <param name="scale"> XMFLOAT3</param>
	void SetScale(XMFLOAT3 scale);

	/// <summary>Sets the position of the scene node
	/// 
	/// Helper function so that users do not have to use XMFLOAT3</summary>
	/// <param name="x">x positon</param>
	/// <param name="y">y position</param>
	/// <param name="z">z position</param>
	void SetPosition(float x, float y, float z);

	/// <summary>Sets the rotation of the scene node
	/// 
	/// Helper function so that users do not have to use XMFLOAT3</summary>
	/// <param name="x">x positon</param>
	/// <param name="y">y position</param>
	/// <param name="z">z position</param>
	void SetRotation(float x, float y, float z);

	/// <summary>Sets the scale of the scene node
	/// 
	/// Helper function so that users do not have to use XMFLOAT3</summary>
	/// <param name="x">x positon</param>
	/// <param name="y">y position</param>
	/// <param name="z">z position</param>
	void SetScale(float x, float y, float z);

	/// <summary> Sets the render item to the paramter
	/// 
	/// </summary>
	/// <param name="renderItem">Render Item pointer</param>
	void SetRenderItem(RenderItem* renderItem);

private:
	/// <summary> Virtual function that updates the current node
	/// 
	/// Will be primarily overridden by child classes</summary>
	/// <param name="gt">Delta time</param>
	virtual void UpdateCurrent(const GameTimer& gt);

	/// <summary> Virtual function that updates the children of the scene node
	/// 
	/// Iterates through all children calling their respective draw functions</summary>
	/// <param name="gt">Delta time</param>
	void UpdateChildren(const GameTimer& gt);

	/// <summary> Virtual function that draws the current node
	/// 
	/// Will be primarily overridden by child classes</summary>
	/// <param name="gt">Delta time</param>
	virtual void DrawCurrent(const GameTimer& gt) const;

	/// <summary> Virtual function that updates the children of the scene node
	/// 
	/// Iterates through all children calling their respective update functions</summary>
	/// <param name="gt">Delta time</param>
	void DrawChildren(const GameTimer& gt) const;

public:
		RenderItem* renderItem;

private:
	std::vector<Ptr> mChildren;

	SceneNode* mParent;

	XMFLOAT3 nodePosition;
	XMFLOAT3 nodeRotation;
	XMFLOAT3 nodeScale;
};

