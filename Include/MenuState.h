#pragma once
#include "State.h"
#include "SpriteNode.h"
#include "../D3DCommon/GameTimer.h"

class Game;
class SceneNode;

/// <summary>
/// Class that encapsulates the menu state. Responsible for managing draw, update and events for the menu state.
/// </summary>
class MenuState : public State
{
public:

	/// <summary>
	/// Menu State overloaded constructor
	/// </summary>
	/// <param name="game">Takes in a pointer to the menu</param>
	/// <param name="stack">Takes a reference to the state stack</param>
	/// <param name="context">Context containing reference to player</param>
	MenuState(Game* game, StateStack& stack, Context context);

	/// <summary>
	/// Function responsible for drawing all menu state render items
	/// </summary>
	/// <param name="dt">Takes a GameTimer parameter used for getting delta time</param>
	virtual void			draw(GameTimer dt);

	/// <summary>
	/// Function responsible for updating all MENU state objects
	/// </summary>
	/// <param name="dt">Takes a GameTimer parameter used for getting delta time</param>
	/// <returns>Returns true if updates should continue for other state or false if updates should stop</returns>
	virtual bool			update(GameTimer dt);

	/// <summary>
	/// Function responsible for handling all MENU state events
	/// </summary>
	/// <param name="event">Takes a WPARAM variable used for passing in key presses</param>
	/// <returns>Returns true if events in other scenes should be handled and false if not</returns>
	virtual bool			handleEvent(WPARAM key);


private:

	/// <summary>
	/// Enumeration for the menu states selectable options
	/// </summary>
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
	SpriteNode*				mArrowSprite;

	OptionNames arrowPosition;

	Game* mGame;

	/// <summary>
	/// Function that instantiates and builds all objects in scene and set appropriate values for scene variables
	/// </summary>
	virtual void BuildScene() override;
};
