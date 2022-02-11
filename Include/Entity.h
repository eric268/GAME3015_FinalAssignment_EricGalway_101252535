#pragma once
#include "SceneNode.h"

/// Parent class for all game objects with velocity
/// 
/// This is the parent class that all objects with movement components will inherit from.
/// It inherits from the SceneNode base class
class Entity : public SceneNode
{
public:
	/// Sets the velocity to a XMFLOAT3 value
	/// 
	/// <param name="vel">Value that velocity is to be set to</param>
	void SetVelocity(XMFLOAT3 vel);
	/// Sets the velocity to three float values
	/// 
	/// Acts as an additional helper function so that users do not have to use XMFLOAT3</summary>
	/// <param name="x">Velocity x value</param>
	/// <param name="y">Velocity y value</param>
	/// <param name="z">Velocity z value</param>
	void SetVelocity(float x, float y, float z);

	/// Gets the velocity of the Entity
	/// 
	/// <returns>XMFLOAT3 value representing velocity</returns>
	XMFLOAT3 GetVelocity() const;

	/// <summary> Updates the Entity
	/// 
	/// Overrides the scene node function and updates the Entity based on Delta Time</summary>
	/// <param name="gt">Delta time</param>
	virtual void UpdateCurrent(const GameTimer& gt);

	/// <summary>Function responsible for moving Entities
	/// 
	/// This function updates the Entity position based on Velocity and Delta Time</summary>
	/// <param name="gt">Delta Time</param>
	virtual void Move(const GameTimer& gt);

private:
	XMFLOAT3 mVelocity;
};

