#pragma once
#include "State.h"
#include "World.h"
#include "Player.h"

class Game;

/// <summary>
/// Class that encapsulates the game state. Responsible for managing draw, update and events for the game state.
/// </summary>
class GameState : public State
{
public:
	/// <summary>
	/// Game State overloaded constructor
	/// </summary>
	/// <param name="game">Takes in a pointer to the game</param>
	/// <param name="stack">Takes a reference to the state stack</param>
	/// <param name="context">Context containing reference to player</param>
	GameState(Game* game, StateStack& stack, Context context);

	/// <summary>
	/// Function responsible for drawing all game state render items
	/// </summary>
	/// <param name="dt">Takes a GameTimer parameter used for getting delta time</param>
	virtual void		draw(GameTimer dt);

	/// <summary>
	/// Function responsible for updating all game state objects
	/// </summary>
	/// <param name="dt">Takes a GameTimer parameter used for getting delta time</param>
	/// <returns>Returns true if updates should continue for other state or false if updates should stop</returns>
	virtual bool		update(GameTimer dt);

	/// <summary>
	/// Function responsible for handling all game state events
	/// </summary>
	/// <param name="event">Takes a WPARAM variable used for passing in key presses</param>
	/// <returns>Returns true if events in other scenes should be handled and false if not</returns>
	virtual bool		handleEvent(const WPARAM event);


private:
	Game* mGame;
	World*				mWorld;
	Player& mPlayer;

	/// <summary>
	/// Function that instantiates and builds all objects in scene and set appropriate values for scene variables
	/// </summary>
	virtual void BuildScene();
};