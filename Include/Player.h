#pragma once
#include "Command.h"
#include <map>
#include "../D3DCommon/d3dApp.h"

class CommandQueue;

/// <summary>
/// Class used for handling player inputs and their corresponding actions/events
/// </summary>
class Player
{
public:

	/// <summary>
	/// Default Player Constructor. Populates key binding map with event keys
	/// </summary>
	Player();
	/// <summary>
	/// Check if pressed key appears in key binding, trigger command if so if this has
	/// not occurred during real time
	/// </summary>
	/// <param name="event">A parameter using Microsofts typedef for event int values</param>
	/// <param name="commands">Takes in a reference to a command queue instances</param>
	void handleEvent(WPARAM event, CommandQueue& commands);
	/// <summary>
	/// Checks if an event has already been pushed to the command queue and if not pushes that event if this has
	/// not occurred during real time
	/// </summary>
	/// <param name="event">A parameter using Microsofts typedef for event int values</param>
	/// <param name="commands">Takes in a reference to a command queue instances</param>
	void handleRealtimeInput(CommandQueue& commands);

	/// <summary>
	/// Enumeration for actions that can be bound to keys
	/// </summary>
	enum Action
	{
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		GetPosition,
		ActionCount
	};
	/// <summary>
	/// Removes all keys that area already mapped to an action and inserts a new binding
	/// </summary>
	/// <param name="action">Action used for handeling input action</param>
	/// <param name="key">Integer used to get corresponding Microsoft WParam</param>
	void assignKey(Action action, WPARAM key);

	/// <summary>
	/// Returns the key to a corresponding action
	/// </summary>
	/// <param name="action">Action that requires key</param>
	/// <returns></returns>
	WPARAM getAssignedKey(Action action) const;

private:
	/// <summary>
	/// Binds all action bindings to their respective derived actions
	/// </summary>
	void initalizeActions();

	/// <summary>
	/// Checks if the passes in Action is a valid bound action
	/// </summary>
	/// <param name="action">Action to check if bound</param>
	/// <returns>Returns true if action is bound and false if not</returns>
	static bool isRealtimeAction(Action action);

private:
	std::map<int, Action> mKeyBinding;
	std::map<Action, Command> mActionBinding;
};