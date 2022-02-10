#include "Game.h"

Game::Game() : gameWorld(World())
{}

Game::~Game()
{
	static DirectX::XMFLOAT4X4 m1
	(10,2,3,4,
	12,13,1,2,
	19,0,2,1,
	8,9,2,1);

	static DirectX::XMFLOAT4X4 m2
	(1, 12, 3, 4,
		12, 73, 1, 2,
		19, 10, 8, 71,
		8, 9, 14, 1);
	static DirectX::XMFLOAT4X4 ans = MathHelper::MultiplyXMFLOAT4x4(m1, m2);

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
}

void Game::Draw(const GameTimer& gt)
{
	gameWorld.Draw(gt);
}

World& Game::GetWorld()
{
	return gameWorld;
}
