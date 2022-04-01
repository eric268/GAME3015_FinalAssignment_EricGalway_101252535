#pragma once
#include "Command.h"

#include <queue>

/// <summary>
/// Class containing queue of all input commands and helper functions
/// </summary>
class CommandQueue
{
public:
	/// <summary>
	/// Adds a command to the command queue
	/// </summary>
	/// <param name="command">Takes a constant Command reference as a parameter</param>
	void push(const Command& command);
	
	/// <summary>
	///  Returns the top Command on the Queue then removes it from the queue
	/// </summary>
	/// <returns>Returns the top Command from the queue</returns>
	Command pop();

	/// <summary>
	/// Checks if the Command Queue is empty
	/// </summary>
	/// <returns>Returns true if empty and false if not empty</returns>
	bool isEmpty() const;
private:
	std::queue<Command> mQueue;
};