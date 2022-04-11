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
	return false;
}

bool PauseState::handleEvent(const WPARAM event)
{
	if (event == VK_RETURN)
	{
		mGame->FlushCommandQueue();
		requestStackPop();
		mGame->GetRenderItems().pop_back();
		mGame->GetRenderItems().shrink_to_fit();
		mGame->mOpaqueRitems.pop_back();
		mGame->mOpaqueRitems.shrink_to_fit();
		Game::objCBIndex--;

		mGame->mFrameResources.clear();
		mGame->BuildFrameResources();
		mGame->BuildMaterials();
		mGame->BuildPSOs();
		mGame->ReAddMaterials();

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
	mGame->FlushCommandQueue();
	mGame->mFrameResources.clear();
	mGame->BuildMaterials();

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 1, 1, Textures::ID::PauseText));
	mBackgroundSprite = backgroundSprite.get();
	mBackgroundSprite->SetScale(XMFLOAT3(0.7f, 0.5f, 0.7f));
	mBackgroundSprite->SetPosition(0, 10.5f, (180 - mGame->mView._43 * 1.4f));
	mBackgroundSprite->SetRotation(-45, 0, 0);
	mSceneGraph->AttachChild(std::move(backgroundSprite));

	mGame->AddRenderItem(mBackgroundSprite->renderItem);
	mGame->BuildFrameResources();
	mGame->BuildPSOs();
	mGame->ReAddMaterials();

}
