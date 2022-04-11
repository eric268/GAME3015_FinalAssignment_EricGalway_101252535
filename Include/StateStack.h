#pragma once
#include "State.h"
#include "StateIdentifiers.h"
#include "ResourceIdentifiers.h"
#include "../D3DCommon/GameTimer.h"

#include <vector>
#include <utility>
#include <functional>
#include <map>

class Game;

/// <summary>
/// Class that encapsulates the state machine. It has the ability to add remove and clear all states currently in the stack
/// </summary>
class StateStack
{
public:

	/// <summary>
	/// Enumeration used for identifying which action is being performed on the state stack
	/// </summary>
	enum Action
	{
		Push,
		Pop,
		Clear
	};

public:
	/// <summary>
	/// Overloaded constructor for the state stack
	/// </summary>
	/// <param name="game">Takes in a pointer to the game application</param>
	/// <param name="context">Takes a state context as a parameter</param>
	explicit StateStack(Game* game, State::Context context);

	/// <summary>
	/// Templated function used for registering various states with their respective state ID's
	/// </summary>
	/// <typeparam name="T">Templated type name</typeparam>
	/// <param name="stateID">State ID value</param>
	template<typename T>
	void registerState(States::ID stateID);

	/// <summary>
	/// Function responsible for updating all states in state stack
	/// </summary>
	/// <param name="dt">Takes a GameTimer parameter used for getting delta time</param>
	void				update(GameTimer dt);

	/// <summary>
	/// Function responsible for calling state draw functions currently in the state stack
	/// </summary>
	/// <param name="gt">Takes a GameTimer parameter used for getting delta time</param>
	void				draw(GameTimer gt);

	/// <summary>
	/// Function responsible for handling all state events in state stack
	/// </summary>
	void				handleEvent(const WPARAM event);

	/// <summary>
	/// Adds a state to the state stack
	/// </summary>
	/// <param name="stateID">State ID enumeration</param>
	void				pushState(States::ID stateID);

	/// <summary>
	/// Removes the top state from the state stack
	/// </summary>
	void				popState();

	/// <summary>
	/// Removes all states from the state stack
	/// </summary>
	void				clearStates();

	/// <summary>
	/// Checks if the state is empty
	/// </summary>
	/// <returns>Returns true if the state stack is empty and false if it is not empty</returns>
	bool				isEmpty() const;

	/// <summary>
	/// Virtual function used for calling build scene in various states on initialization 
	/// </summary>
	virtual void        BuildScene();

	/// <summary>
	/// Applies changes to the state stack 
	/// </summary>
	void				applyPendingChanges();
private:
	/// <summary>
	/// Creates a new state that can be added to the state stack in the future
	/// </summary>
	/// <param name="stateID">ID for the newly created state</param>
	/// <returns>Returns a unique State pointer </returns>
	State::Ptr			createState(States::ID stateID);

private:

	/// <summary>
	/// Struct that encapsulates changes that will be made to the state stack
	/// </summary>
	struct PendingChange
	{
		/// <summary>
		/// Overloaded constructor for pending changes struct
		/// </summary>
		/// <param name="action">Takes an action</param>
		/// <param name="stateID">Takes a state ID as a parameter inintalized to none</param>
		explicit			PendingChange(Action action, States::ID stateID = States::None);

		Action				action;
		States::ID			stateID;
	};

private:
	std::vector<State::Ptr>								mStack;
	std::vector<PendingChange>							mPendingList;
	Game* mGame;
	State::Context										mContext;
	std::map<States::ID, std::function<State::Ptr()>>	mFactories;
};

template <typename T>
void StateStack::registerState(States::ID stateID)
{
	mFactories[stateID] = [this]()
	{
		return State::Ptr(new T(mGame, *this, mContext));
	};
}
