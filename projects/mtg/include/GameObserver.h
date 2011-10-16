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
  // used when we're running to log actions
  list<string> actionsList;
  // used when we're loading to know what to load
  list<string> loadingList;
  list<string>::iterator loadingite;

  int untap(MTGCardInstance * card);
  bool WaitForExtraPayment(MTGCardInstance* card);
  void initialize();
  void cleanup();
  string startupGameSerialized;
  bool parseLine(const string& s);
  void logAction(const string& s);
  bool processActions(bool undo);
  friend ostream& operator<<(ostream&, GameObserver&);
  bool load(const string& s, bool undo);
  bool mLoading;

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
  GameType mGameType;

  TargetChooser * getCurrentTargetChooser();
  void stackObjectClicked(Interruptible * action);

  int cardClick(MTGCardInstance * card, MTGAbility *ability);
  int cardClick(MTGCardInstance * card, int abilityType);
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
  void addObserver(MTGAbility * observer);
  bool removeObserver(ActionElement * observer);
  void startGame(GameType, Rules * rules);
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

  void logAction(Player* player, const string& s="");
  void logAction(int playerId, const string& s="") {
      logAction(players[playerId], s);
  };
  void logAction(MTGCardInstance* card, MTGGameZone* zone, size_t index, int result);
  bool undo();
  bool isLoading(){ return mLoading; };
};

#endif
