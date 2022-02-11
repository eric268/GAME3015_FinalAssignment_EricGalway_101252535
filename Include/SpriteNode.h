#pragma once
#include "SceneNode.h"	

/// <summary>Class that creates sprite nodes
/// 
/// This class is designed to create static nodes in the scene</summary>
class SpriteNode : public SceneNode
{
public:
	/// <summary>Default constructor for Sprite Node
	SpriteNode();

	/// <summary> Overloaded constructor for Scene Node
	/// 
	/// Used for manipulating Render Item properties while keeping default transform values</summary>
	/// <param name="uScaling">Texture U scaling value</param>
	/// <param name="vScaling">Texture V scaling value</param>
	/// <param name="id">Texture ID value</param>
	SpriteNode(float uScaling, float vScaling, Textures::ID id);

	/// <summary>Overloaded Scene Node constructor 
	/// 
	/// Used for when a render item has already been created</summary>
	/// <param name="renderItem">Render Item pointer</param>
	SpriteNode(RenderItem* renderItem);

	/// <summary>Overloaded Scene Node Constructor
	/// 
	/// Useful for when transform data is to be manipulated and UV scaling are 1,1</summary>
	/// <param name="pos">Position</param>
	/// <param name="rot">Rotation</param>
	/// <param name="scale">Scale</param>
	/// <param name="id">Texture ID</param>
	SpriteNode(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Textures::ID id);

private:
	/// <summary>Function that overrides scene node draw current function
	/// 
	/// This adds the current sprite node to the container which draws nodes in the game.cpp</summary>
	/// <param name="gt"></param>
	virtual void DrawCurrent(const GameTimer& gt) const;
};

