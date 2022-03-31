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

	virtual unsigned int getCategory() const;
	
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

