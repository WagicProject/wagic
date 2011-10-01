#ifndef _GAMEOBSERVER_H_
#define _GAMEOBSERVER_H_

#include "Player.h"
#include "MTGAbility.h"
#include "DuelLayers.h"
#include "MTGCardInstance.h"
#include "PlayGuiObject.h"
#include "TargetChooser.h"
#include "PhaseRing.h"
#include "ReplacementEffects.h"
#include "GuiStatic.h"
#include <queue>
#include <time.h>

class MTGGamePhase;
class MTGAbility;
class MTGCardInstance;
struct CardGui;
class Player;
class TargetChooser;
class Rules;
using namespace std;

class GameObserver{
 protected:
  MTGCardInstance * cardWaitingForTargets;
  queue<WEvent *> eventsQueue;

  int untap(MTGCardInstance * card);
  bool WaitForExtraPayment(MTGCardInstance* card);
  void initialize();

 public:
  int currentPlayerId;
  CombatStep combatStep;
  int turn;
  int forceShuffleLibraries();
  int targetListIsSet(MTGCardInstance * card);
  PhaseRing * phaseRing;
  int cancelCurrentAction();
  int currentGamePhase;
  ExtraCosts * mExtraPayment;
  int oldGamePhase;
  TargetChooser * targetChooser;
  DuelLayers * mLayers;
  ReplacementEffects *replacementEffects;
  Player * gameOver;
  vector<Player *> players; //created outside
  time_t startedAt;
  Rules * mRules;

  TargetChooser * getCurrentTargetChooser();
  void stackObjectClicked(Interruptible * action);

  int cardClick(MTGCardInstance * card,Targetable * _object = NULL );
  int getCurrentGamePhase();
  const char * getCurrentGamePhaseName();
  const char * getNextGamePhaseName();
  void nextCombatStep();
  void userRequestNextGamePhase();
  void nextGamePhase();
  void cleanupPhase();
  void nextPlayer();
  void setPlayers(vector<Player *> _players);
  Player * currentPlayer;
  Player * currentActionPlayer;
  Player * isInterrupting;
  Player * opponent();
  Player * currentlyActing();
  GameObserver();
  GameObserver(vector<Player *> _players);
  ~GameObserver();
  void gameStateBasedEffects();
  void enchantmentStatus();
  void Affinity();
  void eventOccured();
  void addObserver(MTGAbility * observer);
  bool removeObserver(ActionElement * observer);
  void startGame(Rules * rules);
  void untapPhase();
  MTGCardInstance * isCardWaiting(){ return cardWaitingForTargets; }
  int isInPlay(MTGCardInstance *  card);
  int isInGrave(MTGCardInstance *  card);
  int isInExile(MTGCardInstance *  card);
  void Update(float dt);
  void Render();
  void ButtonPressed(PlayGuiObject*);
  int getPlayersNumber() {return players.size();};

  int receiveEvent(WEvent * event);
  bool connectRule;
};

#endif
