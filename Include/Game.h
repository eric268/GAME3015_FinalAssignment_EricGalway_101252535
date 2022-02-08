#pragma once
#include <World.h>
#include "../D3DCommon/d3dApp.h"
#include "../D3DCommon/MathHelper.h"
#include "../D3DCommon/UploadBuffer.h"
#include "../D3DCommon/GeometryGenerator.h"


class Game
{
public:

	World mWorld;
	static const float playerSpeed;
	static const float timePerFrame;

	Game();
	~Game();

	void Initalize();
	void ProcessEvents();
	void UpdateStatistics(const GameTimer& gt);
	void Update(const GameTimer& gt);
	void OnKeyboardInput(const GameTimer& gt);
	void Draw(const GameTimer& gt);
	Texture tex;

	World GetWorld();
private:

};

