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
		requestStackPop();
		mGame->GetRenderItems().pop_back();
		mGame->GetRenderItems().shrink_to_fit();
		mGame->mOpaqueRitems.pop_back();
		mGame->mOpaqueRitems.shrink_to_fit();
		Game::objCBIndex--;

		mGame->mFrameResources.clear();
		mGame->BuildFrameResources();
		mGame->BuildMaterials();
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

	mGame->mFrameResources.clear();
	mGame->BuildMaterials();

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 1, 1, Textures::ID::TitleScreen));
	mBackgroundSprite = backgroundSprite.get();
	mBackgroundSprite->SetScale(XMFLOAT3(0.25f, 0.5f, 0.25f));
	mBackgroundSprite->SetPosition(0, -0.2f, 0);
	mBackgroundSprite->SetRotation(0, 0, 0);
	mSceneGraph->AttachChild(std::move(backgroundSprite));

	mGame->AddRenderItem(mBackgroundSprite->renderItem);
	mGame->BuildFrameResources();
	mGame->BuildPSOs();
	mGame->ReAddMaterials();
}
