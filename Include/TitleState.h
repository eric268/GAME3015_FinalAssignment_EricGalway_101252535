#pragma once
#include "State.h"
#include "SpriteNode.h"

class Game;
class SceneNode;
class TitleState : public State
{
public:
	TitleState(Game* game, StateStack& stack, Context context);

	virtual void		draw(GameTimer gt);
	virtual bool		update(GameTimer dt);
	virtual bool		handleEvent(const WPARAM event);


private:
	SpriteNode*			mBackgroundSprite;
	SpriteNode*			mTitleText;
	SpriteNode*			mPressKeyText;
	bool				mShowText;
	float			mTextEffectTime;
	Game* mGame;
	SceneNode* mSceneGraph;
	virtual void BuildScene() override;
	
};