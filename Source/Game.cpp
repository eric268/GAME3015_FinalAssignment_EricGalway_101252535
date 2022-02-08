#include "Game.h"

Game::Game() : mWorld(World())
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
	mWorld.Update(gt);
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
	mWorld.LoadTextures();
	mWorld.BuildScene();
}

void Game::Draw(const GameTimer& gt)
{
	mWorld.Draw(gt);
}

World Game::GetWorld()
{
	return mWorld;
}
