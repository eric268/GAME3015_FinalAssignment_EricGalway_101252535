#pragma region step 11
#include "TitleState.h"
#include "Game.h"
#include "Utility.h"
#include "ResourceHolder.h"
#include "SceneNode.h"

//#include <SFML/Graphics/RenderWindow.hpp>


TitleState::TitleState(Game* game, StateStack& stack, Context context)
	: mGame(game),
	State(stack, context)
	//, mText()
	, mShowText(true)
	, mTextEffectTime(0.0f)
{

	//// Initialize the different layers
	//for (std::size_t i = 0; i < SceneNode::LayerCount; ++i)
	//{
	//	SceneNode::Ptr layer(new SceneNode());
	//	mSceneLayers[i] = layer.get();
	//	mSceneGraph.AttachChild(std::move(layer));
	//}

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mGame, 3, 1500, Textures::ID::TitleScreen));
	backgroundSprite->SetScale(XMFLOAT3(10.0f, 1.0f, 2000.0f));
	backgroundSprite->SetPosition(0, -100, 0);
	mSceneGraph.AttachChild(std::move(backgroundSprite));



	//mText.setFont(context.fonts->get(Fonts::Main));
	//mText.setString("Press any key to start");
	//centerOrigin(mText);
	//mText.setPosition(context.window->getView().getSize() / 2.f);
}

void TitleState::draw(GameTimer gt)
{
	mSceneGraph.Draw(gt);
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
#pragma endregion
