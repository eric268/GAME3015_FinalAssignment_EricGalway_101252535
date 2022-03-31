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