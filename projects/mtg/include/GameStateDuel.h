#ifndef _GAME_STATE_DUEL_H_
#define _GAME_STATE_DUEL_H_


#include "../include/GameState.h"
#include "../include/SimpleMenu.h"
#include "../include/MTGDeck.h"
#include "../include/GameObserver.h"

#define CHOOSE_OPPONENT 7

#ifdef TESTSUITE
class TestSuite;
#endif
class Credits;
class Rules;


class GameStateDuel: public GameState, public JGuiListener
{
 private:
#ifdef TESTSUITE
  TestSuite * testSuite;
#endif
  Credits * credits;
  int mGamePhase;
  Player * mCurrentPlayer;
  Player * mPlayers[2];
  MTGPlayerCards * deck[2];
  GameObserver * game;
  SimpleMenu * deckmenu;
  SimpleMenu * opponentMenu;
  SimpleMenu * menu;
  bool premadeDeck;
  int OpponentsDeckid;
  string musictrack;
  Rules * rules;
  
  bool MusicExist(string FileName);
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
  void initRand (unsigned seed = 0);

};


#endif

