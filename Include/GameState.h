#pragma once
#include "State.h"
#include "World.h"
#include "Player.h"

class Game;

class GameState : public State
{
public:
	GameState(Game* game, StateStack& stack, Context context);

	virtual void		draw(GameTimer dt);
	virtual bool		update(GameTimer dt);
	virtual bool		handleEvent(const WPARAM event);


private:
	Game* mGame;
	World*				mWorld;
	Player& mPlayer;
	virtual void BuildScene();
};