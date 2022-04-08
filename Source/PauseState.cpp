#include "PauseState.h"
#include "ResourceHolder.h"
#include "Game.h"

PauseState::PauseState(Game* game, StateStack& stack, Context context) : 
	mGame(game),
	State(game, stack, context),
	mBackgroundSprite(nullptr),
	mSceneGraph(new SceneNode(game))

{
	BuildScene();
}

void PauseState::draw(GameTimer dt)
{
	mSceneGraph->Draw(dt);
}

bool PauseState::update(GameTimer dt)
{
	//mSceneGraph->Update(dt);
	return true;
}

bool PauseState::handleEvent(const WPARAM event)
{
	if (event == VK_ESCAPE)
	{
		requestStackPop();
	}

	if (event == VK_BACK)
	{
		requestStateClear();
		requestStackPush(States::Menu);
	}
	return true;
}

void PauseState::BuildScene()
{
	mGame->mOpaqueRitems.clear();
	mGame->mFrameResources.clear();

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 1, 1, Textures::ID::TitleScreen));
	mBackgroundSprite = backgroundSprite.get();
	mBackgroundSprite->SetScale(XMFLOAT3(0.25f, 0.5f, 0.25f));
	mBackgroundSprite->SetPosition(0, 0, 0);
	mSceneGraph->AttachChild(std::move(backgroundSprite));

	mGame->BuildRenderItems();
	mGame->BuildFrameResources();
	//mGame->BuildPSOs();
}
