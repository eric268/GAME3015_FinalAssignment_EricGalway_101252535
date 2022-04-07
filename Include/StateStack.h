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

class StateStack
{
public:
	enum Action
	{
		Push,
		Pop,
		Clear
	};

public:
	explicit StateStack(Game* game, State::Context context);

	template<typename T>
	void registerState(States::ID stateID);
	void				update(GameTimer dt);
	void				draw(GameTimer gt);
	void				handleEvent(const WPARAM event);

	void				pushState(States::ID stateID);
	void				popState();
	void				clearStates();

	bool				isEmpty() const;
	virtual void        BuildScene();
	void				applyPendingChanges();
private:
	State::Ptr			createState(States::ID stateID);



private:
	struct PendingChange
	{
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
