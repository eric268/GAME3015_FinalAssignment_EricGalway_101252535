#pragma once
#include "StateIdentifiers.h"
#include "ResourceIdentifiers.h"
#include "../D3DCommon/GameTimer.h"
#include "../D3DCommon/d3dApp.h"

#include <memory>


class StateStack;
class Player;
class Game;

class State
{
public:
	typedef std::unique_ptr<State> Ptr;

	struct Context
	{
		Context(Player& player);
		Player* player;
	};

public:
	State(Game* game, StateStack& stack, Context context);
	virtual ~State();

	virtual void draw(GameTimer gt) = 0;
	virtual bool update(GameTimer gt) = 0;
	virtual bool handleEvent(WPARAM event) = 0;
	virtual void BuildScene() = 0;



	void requestStackPush(States::ID stateID);
	void requestStackPop();
	void requestStateClear();

	Context getContext() const;

	StateStack* mStack;
	Context mContext;
	Game* mGame;

};