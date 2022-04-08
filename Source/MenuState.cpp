#include "MenuState.h"
#include "Utility.h"
#include "ResourceHolder.h"
#include "Game.h"
#include "SceneNode.h"


MenuState::MenuState(Game* game, StateStack& stack, Context context)
	: mGame(game),
	State(game, stack, context),
	mOption1Sprite(nullptr),
	mOption2Sprite(nullptr),
	mBackgroundSprite(nullptr),
	mSceneGraph(new SceneNode(game))

{
	BuildScene();
}

void MenuState::draw(GameTimer dt)
{
	mSceneGraph->Draw(dt);
}

bool MenuState::update(GameTimer dt)
{
	return true;
}

bool MenuState::handleEvent(const WPARAM event)
{
	if (GetAsyncKeyState('S'))
	{
		requestStackPop();
		requestStackPush(States::Title);
	}
	else if (GetAsyncKeyState('Q'))
	{
		// The exit option was chosen, by removing itself, the stack will be empty, and the game will know it is time to close.
		requestStackPop();
	}
	return true;
}

void MenuState::BuildScene()
{
	mGame->LoadText();
	mGame->GetRenderItems().clear();
	mGame->mOpaqueRitems.clear();
	mGame->mFrameResources.clear();
	Game::objCBIndex = 0;

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 1, 1, Textures::ID::TitleScreen));
	mBackgroundSprite = backgroundSprite.get();
	mBackgroundSprite->SetScale(XMFLOAT3(0.75f, 0.5f, 0.55f));
	mBackgroundSprite->SetPosition(0, 0, 0);
	mSceneGraph->AttachChild(std::move(backgroundSprite));

	std::unique_ptr<SpriteNode> option1Sprite(new SpriteNode(mGame, 1, 1, Textures::ID::Raptor));
	mOption1Sprite = option1Sprite.get();
	mOption1Sprite->SetScale(XMFLOAT3(0.15f, 0.5f, 0.15f));
	mOption1Sprite->SetPosition(25, 0.15f, 0);
	mSceneGraph->AttachChild(std::move(option1Sprite));

	std::unique_ptr<SpriteNode> option2Sprite(new SpriteNode(mGame, 1, 1, Textures::ID::Eagle));
	mOption2Sprite = option2Sprite.get();
	mOption2Sprite->SetScale(XMFLOAT3(0.15f, 0.5f, 0.15f));
	mOption2Sprite->SetPosition(25, 0.1f, -20);
	mSceneGraph->AttachChild(std::move(option2Sprite));

	mGame->BuildRenderItems();
	mGame->BuildFrameResources();
	mGame->InitalizeCamera();
}
