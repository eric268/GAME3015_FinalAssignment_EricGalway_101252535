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

	// Escape pressed, trigger the pause screen
	//if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
//	requestStackPush(States::Pause);


	return true;
}

void GameState::BuildScene()
{
	mGame->LoadText();
	mGame->GetRenderItems().clear();
	mGame->mOpaqueRitems.clear();
	mGame->mFrameResources.clear();
	Game::objCBIndex = 0;

	mWorld = new World(mGame);

	mGame->BuildRenderItems();
	mGame->BuildFrameResources();
	mGame->InitalizeCamera(150.0f, 1.5f * XM_PI, 0.25f * XM_PI);
}
