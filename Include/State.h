#pragma once
#include "StateIdentifiers.h"
#include "ResourceIdentifiers.h"
#include "../D3DCommon/GameTimer.h"
#include "../D3DCommon/d3dApp.h"

#include <memory>

namespace sf
{
	class RenderWindow;
}

class StateStack;
class Player;

class State
{
public:
	typedef std::unique_ptr<State> Ptr;

	struct Context
	{
		Context(/*sf::RenderWindow& window,*/ TextureHolder& textures, FontHolder& fonts, Player& player);
		//sf::RenderWindow* window;
		TextureHolder* textures;
		FontHolder* fonts;
		Player* player;
	};

public:
	State(StateStack& stack, Context context);
	virtual ~State();

	virtual void draw(GameTimer gt) = 0;
	virtual bool update(GameTimer gt) = 0;
	virtual bool handleEvent(WPARAM event) = 0;


protected:
	void requestStackPush(States::ID stateID);
	void requestStackPop();
	void requestStateClear();

	Context getContext() const;
private:
	StateStack* mStack;
	Context mContext;

};