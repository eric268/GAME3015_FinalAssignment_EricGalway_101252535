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

private:
	/// <summary>Function that overrides scene node draw current function
	/// 
	/// This adds the current sprite node to the container which draws nodes in the game.cpp</summary>
	/// <param name="gt"></param>
	virtual void DrawCurrent(const GameTimer& gt) const;
};

