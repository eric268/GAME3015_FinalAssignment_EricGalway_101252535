#include "Player.h"
#include "CommandQueue.h"
#include "Aircraft.h"
#include "World.h"
#include "Game.h"
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

/// <summary>
/// Struct used for changing the velocity of Aircraft
/// </summary>
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

Player::Player(World* world)
	: mWorld(world)
{
	mKeyBinding[VK_LEFT] = MoveLeft;
	mKeyBinding[VK_RIGHT] = MoveRight;
	mKeyBinding[VK_UP] = MoveUp;
	mKeyBinding[VK_DOWN] = MoveDown;
	mKeyBinding['P'] = GetPosition;

	mKeyIsPressed[MoveLeft] = false;
	mKeyIsPressed[MoveRight] = false;
	mKeyIsPressed[MoveUp] = false;
	mKeyIsPressed[MoveDown] = false;
	mKeyIsPressed[GetPosition] = false;

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
		mKeyIsPressed[found->second] = true;
		commands.push(mActionBinding[found->second]);
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	for (auto pair : mKeyBinding)
	{
		if (GetAsyncKeyState(pair.first))
		{
			if (isRealtimeAction(pair.second))
			{
				commands.push(mActionBinding[pair.second]);
			}
		}
		else
		{
			mKeyIsPressed[pair.second] = false;
		}
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

void Player::processEvents(CommandQueue& commands)
{
	if (mWorld->GetGame()->GetKeyIsPressed())
	{
		handleEvent(mWorld->GetGame()->GetKeyPressed(), commands);
		handleRealtimeInput(commands);
	}
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
		return mKeyIsPressed[MoveLeft];
	case MoveRight:
		return mKeyIsPressed[MoveRight];
	case MoveDown:
		return mKeyIsPressed[MoveDown];
	case MoveUp:
		return mKeyIsPressed[MoveUp];
	case GetPosition:
		return mKeyIsPressed[GetPosition];
	default:
		return false;
	}
}