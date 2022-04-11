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
	BuildScene();
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
		requestStackPush(States::Menu);
	}

	return true;
}
void TitleState::BuildScene()
{	
	mGame->GetRenderItems().clear();
	mGame->mOpaqueRitems.clear();
	mGame->mFrameResources.clear();
	mGame->BuildMaterials();
	Game::objCBIndex = 0;


	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 1, 1, Textures::ID::TitleScreen));
	mBackgroundSprite = backgroundSprite.get();
	mBackgroundSprite->SetScale(XMFLOAT3(0.75f, 0.5f, 0.55f));
	mBackgroundSprite->SetPosition(0, 0, 0);
	mSceneGraph->AttachChild(std::move(backgroundSprite));

	std::unique_ptr<SpriteNode> promptSprite(new SpriteNode(mGame, 1, 1, Textures::ID::TitleText));
	mTitleText = promptSprite.get();
	mTitleText->SetScale(XMFLOAT3(0.08f, 0.05f, 0.05f));
	mTitleText->SetPosition(-45, 0.1f, -35);
	mSceneGraph->AttachChild(std::move(promptSprite));


	mGame->BuildRenderItems();
	mGame->BuildFrameResources();
	mGame->BuildPSOs();
	mGame->InitalizeCamera(100.0f, 1.5f * XM_PI, 0.1f);
}
