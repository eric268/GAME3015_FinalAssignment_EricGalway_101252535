#pragma once
#include "State.h"
#include "SpriteNode.h"
#include "../D3DCommon/GameTimer.h"

class Game;
class SceneNode;

class MenuState : public State
{
public:
	MenuState(Game* mGame, StateStack& stack, Context context);

	virtual void			draw(GameTimer dt);
	virtual bool			update(GameTimer dt);
	virtual bool			handleEvent(WPARAM key);


private:
	enum OptionNames
	{
		Play,
		Exit,
	};


private:
	SceneNode* mSceneGraph;
	SpriteNode*				mBackgroundSprite;
	SpriteNode*				mOption1Sprite;
	SpriteNode*				mOption2Sprite;

	Game* mGame;

	virtual void BuildScene() override;

};
