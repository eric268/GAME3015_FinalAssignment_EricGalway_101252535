#pragma once
#include "StateIdentifiers.h"
#include "ResourceIdentifiers.h"
#include "../D3DCommon/GameTimer.h"
#include "../D3DCommon/d3dApp.h"

#include <memory>

class StateStack;
class Player;
class Game;

/// <summary>
/// State class that encapsulates the state functionality for different application states
/// </summary>
class State
{
public:
	typedef std::unique_ptr<State> Ptr;

	/// <summary>
	/// Wrapper class that encapsulates a reference to the player and other future variables needed in the state class 
	/// </summary>
	struct Context
	{
		/// <summary>
		/// Overloaded constructor
		/// </summary>
		/// <param name="player">Takes in a reference to the player</param>
		Context(Player& player);
		Player* player;
	};

public:
	/// <summary>
	/// Overloaded state constructor
	/// </summary>
	/// <param name="game">Takes in a pointer to the game</param>
	/// <param name="stack">Takes a reference to the state stack</param>
	/// <param name="context">Context containing reference to player</param>
	State(Game* game, StateStack& stack, Context context);

	/// <summary>
	/// Destructor for different states
	/// </summary>
	virtual ~State();

	/// <summary>
	/// Pure virtual function responsible for drawing all state render items
	/// </summary>
	/// <param name="dt">Takes a GameTimer parameter used for getting delta time</param>
	virtual void draw(GameTimer gt) = 0;

	/// <summary>
	/// Pure virtual function responsible for updating all state objects
	/// </summary>
	/// <param name="dt">Takes a GameTimer parameter used for getting delta time</param>
	/// <returns>Returns true if updates should continue for other state or false if updates should stop</returns>
	virtual bool update(GameTimer gt) = 0;

	/// <summary>
	/// Pure virtual function responsible for handling all state events
	/// </summary>
	/// <param name="event">Takes a WPARAM variable used for passing in key presses</param>
	/// <returns>Returns true if events in other scenes should be handled and false if not</returns>
	virtual bool handleEvent(WPARAM event) = 0;

	/// <summary>
	/// Pure virtual function that instantiates and builds all objects in scene and set appropriate values for scene variables
	/// </summary>
	virtual void BuildScene() = 0;

	/// <summary>
	/// Function used to add states to the state stack
	/// </summary>
	/// <param name="stateID">Takes in an enum usesd for identifying states</param>
	void requestStackPush(States::ID stateID);

	/// <summary>
	/// Function used to remove top state from the state stack
	/// </summary>
	void requestStackPop();

	/// <summary>
	/// Function used to remove all states from the state stack
	/// </summary>
	void requestStateClear();

	/// <summary>
	/// Returns the state context variables
	/// </summary>
	/// <returns>Returns a constant context variable</returns>
	Context getContext() const;

	StateStack* mStack;
	Context mContext;
	Game* mGame;

};