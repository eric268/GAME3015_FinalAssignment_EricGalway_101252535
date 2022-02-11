#pragma once
#include "Entity.h"

/// Concrete class for Aircraft
/// 
/// This is the concrete class for all aircraft. The current available aircraft options are the Eagle or Raptor.
/// This class inherits from the Entity class.
class Aircraft : public Entity
{
public:

	/// Aircraft type
	/// 
	/// This enumeration is used to distinguish between the different types of aircraft
	enum Type
	{
		Eagle,
		Raptor
	};

public:
	/// Overloaded constructor for Aircraft
	/// 
	/// This constructor initializes and aircraft based on type required. Acts almost like a default constructor initializing 
	/// position, scale, render item all to default values
	/// <param name="type"> Type of aircraft to create</param>
	Aircraft(Type type);
	/// Overloaded constructor for Aircraft
	/// 
	/// This constructor initializes an aircraft object when a render item has already been created.
	/// <param name="renderItem"> The render item to initalize the aircraft object with</param>
	Aircraft(RenderItem* renderItem);
	/// Overloaded constructor for Aircraft
	/// 
	/// This constructor allows for values to be set at initialization.
	/// <param name="pos">The position to initalize the aircraft with</param>
	/// <param name="rot">The rotation to initalize the aircraft</param>
	/// <param name="scale">The scale to initalize the aircraft</param>
	/// <param name="type">The type to initalize the aircraft</param>
	Aircraft(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Type type);
	
	/// Converts the aircraft type enumeration to the appropriate texture enumeration
	/// 
	/// </summary>
	/// <param name="type">Aircraft type</param>
	/// <returns>Texture ID</returns>
	Textures::ID ConvertTypeToTexture(Aircraft::Type type);

	/// Adds the aircraft to be drawn in game.cpp DrawRenderItems() function
	/// 
	/// This function overrides the scene node SceneNode::DrawCurrent(const GameTimer& gt) const function 
	/// <param name="gt">The delta time</param>
	virtual void DrawCurrent(const GameTimer& gt) const;

private:
	Type mType;
};

