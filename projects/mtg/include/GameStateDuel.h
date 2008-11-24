#ifndef _GAME_STATE_DUEL_H_
#define _GAME_STATE_DUEL_H_


#include "../include/GameState.h"
#include "_includeAll.h"
#include "../include/SimpleMenu.h"

enum
  {
    DUEL_START,
    DUEL_END,
    DUEL_CHOOSE_DECK1,
    DUEL_CHOOSE_DECK2,
    ERROR_NO_DECK,
    DUEL_PLAY,
    DUEL_MENU
  };

#define CHOOSE_OPPONENT 7

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
  SimpleMenu * opponentMenu;
  SimpleMenu * menu;
  JLBFont* mFont, *opponentMenuFont;
  int nbAIDecks;

  void loadPlayer(int playerId, int decknb = 0, int isAI = 0);
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

