#include "State.h"
#include "StateStack.h"
#include  "Game.h"

State::Context::Context(TextureHolder& textures, Player& player)
	: textures(&textures)
	, player(&player)
{
}

State::State(Game* game, StateStack& stack, Context context)
	: mGame(game),
	  mStack(&stack),
	  mContext(context)
{
}

State::~State()
{
}

void State::requestStackPush(States::ID stateID)
{
	mStack->pushState(stateID);
}

void State::requestStackPop()
{
	mStack->popState();
}

void State::requestStateClear()
{
	mStack->clearStates();
}

State::Context State::getContext() const
{
	return mContext;
}