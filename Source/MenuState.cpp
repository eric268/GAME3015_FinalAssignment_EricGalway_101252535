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
	mSceneGraph(new SceneNode(game)),
	arrowPosition(Play)

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
	if (event == VK_UP)
	{
		if (arrowPosition != Play)
		{
			arrowPosition = Play;
			mArrowSprite->SetPosition(-43, 0.3f, 9.5);
		}
	}
	else if (event == VK_DOWN)
	{
		if (arrowPosition != Exit)
		{
			arrowPosition = Exit;
			mArrowSprite->SetPosition(-43, 0.3f, -5.5);
		}
	}
	else if (event == VK_RETURN)
	{
		if (arrowPosition == Play)
		{
			requestStackPop();
			requestStackPush(States::Game);
		}
		else if (arrowPosition == Exit)
		{
			requestStackPop();
		}
	}
	return true;
}

void MenuState::BuildScene()
{
	mGame->FlushCommandQueue();
	mGame->GetRenderItems().clear();
	mGame->mOpaqueRitems.clear();
	mGame->mFrameResources.clear();
	mGame->BuildMaterials();
	Game::objCBIndex = 0;

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 1, 1, Textures::ID::MenuScreen));
	mBackgroundSprite = backgroundSprite.get();
	mBackgroundSprite->SetScale(XMFLOAT3(0.75f, 0.5f, 0.55f));
	mBackgroundSprite->SetPosition(0, 0, 0);
	mSceneGraph->AttachChild(std::move(backgroundSprite));

	std::unique_ptr<SpriteNode> option1Sprite(new SpriteNode(mGame, 1, 1, Textures::ID::PlayText));
	mOption1Sprite = option1Sprite.get();
	mOption1Sprite->SetScale(XMFLOAT3(0.08, 0.06, 0.06));
	mOption1Sprite->SetPosition(-29, 0.1f, 10);
	mSceneGraph->AttachChild(std::move(option1Sprite));

	std::unique_ptr<SpriteNode> option2Sprite(new SpriteNode(mGame, 1, 1, Textures::ID::QuitText));
	mOption2Sprite = option2Sprite.get();
	mOption2Sprite->SetScale(XMFLOAT3(0.08, 0.06, 0.06));
	mOption2Sprite->SetPosition(-29, 0.2f, -5);
	mSceneGraph->AttachChild(std::move(option2Sprite));

	std::unique_ptr<SpriteNode> arrowSprite(new SpriteNode(mGame, 1, 1, Textures::ID::Arrow));
	mArrowSprite = arrowSprite.get();
	mArrowSprite->SetScale(XMFLOAT3(0.07f, 0.04f, 0.04f));
	mArrowSprite->SetPosition(-43, 0.3f, 9.5);
	mSceneGraph->AttachChild(std::move(arrowSprite));

	mGame->BuildRenderItems();
	mGame->BuildFrameResources();
	mGame->BuildPSOs();
	mGame->InitalizeCamera(100.0f, 1.5f * XM_PI, 0.1f);
}
