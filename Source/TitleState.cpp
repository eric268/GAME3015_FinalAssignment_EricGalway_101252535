#pragma region step 11
#include "TitleState.h"
#include "Game.h"
#include "Utility.h"
#include "ResourceHolder.h"
#include "SceneNode.h"

TitleState::TitleState(Game* game, StateStack& stack, Context context)
	: mGame(game),
	  State(game, stack, context),
	 mBackgroundSprite(nullptr),
	 mSceneGraph(new SceneNode(game)),
	 mShowText(true),
	 mTextEffectTime(0.0f)
{
	//BuildScene();
}

void TitleState::draw(GameTimer gt)
{
	mSceneGraph->Draw(gt);
}

bool TitleState::update(GameTimer dt)
{
	mTextEffectTime += dt.DeltaTime();

	if (mTextEffectTime >= 0.5f)
	{
		//mShowText = !mShowText;
		mTextEffectTime =0.0f;
	}

	return true;
}

bool TitleState::handleEvent(const WPARAM event)
{
	// If any key is pressed, trigger the next screen
	if (mGame->GetKeyIsPressed())
	{
		requestStackPop();
		//requestStackPush(States::Menu);
	}

	return true;
}
void TitleState::BuildScene()
{
	mGame->GetRenderItems().clear();
	mGame->mOpaqueRitems.clear();
	mGame->mFrameResources.clear();

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 1, 1, Textures::ID::TitleScreen));
	mBackgroundSprite = backgroundSprite.get();
	mBackgroundSprite->SetScale(XMFLOAT3(5.0f, 1.0f, 5));
	mBackgroundSprite->SetPosition(0, -100, 0);
	mSceneGraph->AttachChild(std::move(backgroundSprite));
}
#pragma endregion
