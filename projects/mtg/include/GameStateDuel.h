#ifndef _GAME_STATE_DUEL_H_
#define _GAME_STATE_DUEL_H_


#include "../include/GameState.h"
#include "_includeAll.h"
#include "../include/SimpleMenu.h"


#define DUEL_START 0
#define DUEL_END 1
#define DUEL_CHOOSE_DECK1 2
#define DUEL_CHOOSE_DECK2 3
#define ERROR_NO_DECK 4
#define DUEL_PLAY 5
#define DUEL_MENU 6


#ifdef TESTSUITE
class TestSuite;
#endif

class GameStateDuel: public GameState, public JGuiListener
{
private:
#ifdef TESTSUITE
	TestSuite * testSuite;
#endif
	int mGamePhase;
	Player * mCurrentPlayer;
	Player * mPlayers[2];
	MTGPlayerCards * deck[2];
	GameObserver * game;
	SimpleMenu * deckmenu;
	SimpleMenu * menu;
	JLBFont* mFont;
	
	void loadPlayer(int playerId, int decknb = 0);
public:
	GameStateDuel(GameApp* parent);
	virtual ~GameStateDuel();
#ifdef TESTSUITE
  void loadTestSuitePlayers();
#endif
	virtual void ButtonPressed(int ControllerId, int ControlId);
	virtual void Start();
	virtual void End();
	virtual void Update(float dt);
	virtual void Render(); 

};


#endif

