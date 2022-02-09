#include "Game.h"

Game::Game() : gameWorld(World())
{}

Game::~Game()
{
}

void Game::ProcessEvents()
{
}

void Game::UpdateStatistics(const GameTimer& gt)
{
}

void Game::Update(const GameTimer& gt)
{
	gameWorld.Update(gt);
	UpdateStatistics(gt);
}


void Game::OnKeyboardInput(const GameTimer& gt)
{
	if (GetAsyncKeyState('1') & 0x8000)
	{
		
	}

}

void Game::Initalize()
{
	gameWorld.LoadTextures();
	gameWorld.BuildScene();
}

void Game::Draw(const GameTimer& gt)
{
	gameWorld.Draw(gt);
}

World Game::GetWorld()
{
	return gameWorld;
}
