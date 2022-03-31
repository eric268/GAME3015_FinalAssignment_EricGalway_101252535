#pragma once
#include "Command.h"
#include <map>

#include "Game.h"

class CommandQueue;
class Player
{
public:
	//typedef int Key;

	Player();
	void handleEvent(UINT event, CommandQueue& commands);
	void handleRealtimeInput(CommandQueue& commands);

	enum Action
	{
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		GetPosition,
		ActionCount
	};

	void assignKey(Action action, int key);
	int getAssignedKey(Action action) const;

private:
	void initalizeActions();
	static bool isRealtimeAction(Action action);

private:
	std::map<int, Action> mKeyBinding;
	std::map<Action, Command> mActionBinding;
};