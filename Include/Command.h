#pragma once

#include "Category.h"
#include <functional>
#include <cassert>
#include "../D3DCommon/GameTimer.h"

class SceneNode;

/// <summary>
/// Structure used for executing input commands
/// </summary>
struct Command
{
	/// <summary>
	/// Default constructor for creating a command 
	/// </summary>
	Command();

	std::function<void(SceneNode&, GameTimer)> action;
	unsigned int							   category;
};

/// <summary>
/// Templated function used for binding actions to events
/// </summary>
/// <typeparam name="GameObject">Type parameter used to test if cast is safe</typeparam>
/// <typeparam name="Function">Templated function type</typeparam>
/// <param name="fn">The function that is to be bound to an action</param>
/// <returns>A function that is bound to an event</returns>
template <typename GameObject, typename Function>
std::function<void(SceneNode&, GameTimer)> derivedAction(Function fn)
{
	return[=](SceneNode& node, GameTimer dt)
	{
		assert(dynamic_cast<GameObject*>(&node) != nullptr);
		fn(static_cast<GameObject&>(node), dt);
	};
}