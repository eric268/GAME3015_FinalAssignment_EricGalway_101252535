#pragma once
#include "State.h"
#include "SpriteNode.h"

class Game;

class PauseState : public State 
{
public:
	PauseState(Game* game, StateStack& stack, Context context);

	virtual void		draw(GameTimer dt);
	virtual bool		update(GameTimer dt);
	virtual bool		handleEvent(const WPARAM event);

private: 
	SpriteNode* mBackgroundSprite;
	SceneNode* mSceneGraph;
	Game* mGame;

	virtual void BuildScene();
};