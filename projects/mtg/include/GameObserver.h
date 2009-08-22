#ifndef _GAMEOBSERVER_H_
#define _GAMEOBSERVER_H_

#include "Player.h"
#include "MTGAbility.h"
#include "DuelLayers.h"
#include "MTGCardInstance.h"
#include "PlayGuiObject.h"
#include "DuelLayers.h"
#include "TargetChooser.h"
#include "PhaseRing.h"
#include "ReplacementEffects.h"
#include "GuiStatic.h"

class MTGGamePhase;
class MTGAbility;
class MTGCardInstance;
struct CardGui;
class Player;
class TargetChooser;


class GameObserver{
 protected:
  int reaction;
  static GameObserver * mInstance;
  MTGCardInstance * cardWaitingForTargets;

  int nbPlayers;
  int currentPlayerId;
  int currentRound;
  int blockersAssigned;


 public:
  int blockersSorted;
  int turn;
  int forceShuffleLibraries();
  int targetListIsSet(MTGCardInstance * card);
  PhaseRing * phaseRing;
  int cancelCurrentAction();
  int currentGamePhase;
  ExtraCosts * waitForExtraPayment;
  int oldGamePhase;
  TargetChooser * targetChooser;
  DuelLayers * mLayers;
  ReplacementEffects *replacementEffects;
  Player * gameOver;
  Player * players[2]; //created outside
  MTGGamePhase * gamePhaseManager; //Created Outside ?

  TargetChooser * getCurrentTargetChooser();
  void stackObjectClicked(Interruptible * action);

  void cardClick(MTGCardInstance * card,Targetable * _object = NULL );
  int enteringPhase(int phase);
  int getCurrentGamePhase();
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
  void setGamePhaseManager(MTGGamePhase * _phases);
  void stateEffects();
  void eventOccured();
  void addObserver(MTGAbility * observer);
  void removeObserver(ActionElement * observer);
  void startGame(int shuffle = 1, int draw = 1);
  void untapPhase();
  void draw();
  int isInPlay(MTGCardInstance *  card);
  bool isCreature(MTGCardInstance *  card);

  void Update(float dt);
  void Render();
  void ButtonPressed(PlayGuiObject*);

  int receiveEvent(WEvent * event);
};

#endif
