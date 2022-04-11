#include "GameState.h"
#include "Game.h"

GameState::GameState(Game* game, StateStack& stack, Context context) :
	mGame(game),
	State(game, stack, context),
	mPlayer(game->mPlayer)
{
	BuildScene();
}

void GameState::draw(GameTimer dt)
{
	mWorld->Draw(dt);
}

bool GameState::update(GameTimer dt)
{
	mWorld->Update(dt);

	CommandQueue& commands = mWorld->getCommandQueue();
	mPlayer.handleRealtimeInput(commands);
	return true;
}

bool GameState::handleEvent(const WPARAM event)
{
	CommandQueue& commands = mWorld->getCommandQueue();
	mPlayer.handleEvent(event, commands);

	if (event == 'P')
	{
		requestStackPush(States::Pause);
	}
	return true;
}

void GameState::BuildScene()
{
	mGame->FlushCommandQueue();
	mGame->GetRenderItems().clear();
	mGame->mOpaqueRitems.clear();
	mGame->mFrameResources.clear();
	mGame->BuildMaterials();
	Game::objCBIndex = 0;
	
	mGame->InitalizeCamera(150.0f, 1.5f * XM_PI, 0.25f * XM_PI);
	
	mWorld = new World(mGame);

	mGame->BuildRenderItems();
	mGame->BuildFrameResources();
}
