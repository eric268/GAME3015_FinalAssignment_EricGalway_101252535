#include "Player.h"
#include "CommandQueue.h"
#include "Aircraft.h"
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

struct AircraftMover
{
	AircraftMover(float vx, float vy)
		: velocity(vx, vy)
	{

	}

	void operator() (Aircraft& aircraft, GameTimer) const
	{
		aircraft.accelerate(velocity);
	}
	XMFLOAT2 velocity;
};

Player::Player()
{
	mKeyBinding[VK_LEFT] = MoveLeft;
	mKeyBinding[VK_RIGHT] = MoveRight;
	mKeyBinding[VK_UP] = MoveUp;
	mKeyBinding[VK_DOWN] = MoveDown;

	mKeyBinding['P'] = GetPosition;

	initalizeActions();

	for (auto& pair : mActionBinding)
	{
		pair.second.category = Category::PlayerAircraft;
	}
}

void Player::handleEvent(WPARAM event, CommandQueue& commands)
{
	auto found = mKeyBinding.find(event);
	if (found != mKeyBinding.end() && !isRealtimeAction(found->second))
	{
		commands.push(mActionBinding[found->second]);
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	for (auto pair : mKeyBinding)
	{
		if (GetAsyncKeyState(pair.first) && isRealtimeAction(pair.second))
			commands.push(mActionBinding[pair.second]);
	}
}

void Player::assignKey(Action action, WPARAM key)
{
	for (auto itr = mKeyBinding.begin(); itr != mKeyBinding.end();)
	{
		if (itr->second == action)
			mKeyBinding.erase(itr++);
		else
			++itr;
	}
	mKeyBinding[key] = action;
}

WPARAM Player::getAssignedKey(Action action) const
{
	for (auto pair : mKeyBinding)
	{
		if (pair.second == action)
		{
			return pair.first;
		}
	}
	return -1;
}

void Player::initalizeActions()
{
	const float playerSpeed = 50.0f;
	mActionBinding[MoveLeft].action = derivedAction<Aircraft>(AircraftMover(-playerSpeed, 0.0f));
	mActionBinding[MoveRight].action = derivedAction<Aircraft>(AircraftMover(playerSpeed, 0.0f));
	mActionBinding[MoveUp].action = derivedAction<Aircraft>(AircraftMover(0.0f, playerSpeed));
	mActionBinding[MoveDown].action = derivedAction<Aircraft>(AircraftMover(0.0f, -playerSpeed));

	mActionBinding[GetPosition].action = [](SceneNode& s, GameTimer) {
		std::wstring pos = std::to_wstring(s.GetPosition().x) + L"," + std::to_wstring(s.GetPosition().z) + L"\n";
		OutputDebugString(pos.c_str());
	};
}

bool Player::isRealtimeAction(Action action)
{
	switch (action)
	{
	case MoveLeft:
	case MoveRight:
	case MoveDown:
	case MoveUp:
		return true;

	default:
		return false;
	}
}