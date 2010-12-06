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
  static GameObserver * mInstance;
  MTGCardInstance * cardWaitingForTargets;
  queue<WEvent *> eventsQueue;

  int nbPlayers;
  int untap(MTGCardInstance * card);
  bool WaitForExtraPayment(MTGCardInstance* card);

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
  Player * players[2]; //created outside
  time_t startedAt;
  Rules * mRules;

  TargetChooser * getCurrentTargetChooser();
  void stackObjectClicked(Interruptible * action);

  int cardClick(MTGCardInstance * card,Targetable * _object = NULL );
  int getCurrentGamePhase();
  void nextCombatStep();
  void userRequestNextGamePhase();
  void nextGamePhase();
  void cleanupPhase();
  void nextPlayer();
  static void Init(Player * _players[], int _nbplayers);
  static GameObserver * GetInstance();
  static void EndInstance();
  Player * currentPlayer;
  Player * currentActionPlayer;
  Player * isInterrupting;
  Player * opponent();
  Player * currentlyActing();
  GameObserver(Player * _players[], int _nbplayers);
  ~GameObserver();
  void stateEffects();
  void eventOccured();
  void addObserver(MTGAbility * observer);
  void removeObserver(ActionElement * observer);
  void startGame(Rules * rules);
  void untapPhase();
  void draw();
  int isInPlay(MTGCardInstance *  card);

  void Update(float dt);
  void Render();
  void ButtonPressed(PlayGuiObject*);

  int receiveEvent(WEvent * event);
};

#endif
