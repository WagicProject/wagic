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
#ifdef NETWORK_SUPPORT
#include "JNetwork.h"
#endif //NETWORK_SUPPORT

class MTGGamePhase;
class MTGAbility;
class MTGCardInstance;
struct CardGui;
class Player;
class TargetChooser;
class Rules;
class TestSuiteGame;
class Trash;
class DeckManager;
using namespace std;

class GameObserver{
 protected:

  GameType mGameType;
  MTGCardInstance * cardWaitingForTargets;
  queue<WEvent *> eventsQueue;
  // used when we're running to log actions
  list<string> actionsList;
  // used when we're loading to know what to load
  list<string> loadingList;
  list<string>::iterator loadingite;
  RandomGenerator randomGenerator;
  WResourceManager* mResourceManager;
  JGE* mJGE;
  DeckManager* mDeckManager;
  Player * gameOver;
  GamePhase mCurrentGamePhase;

  int untap(MTGCardInstance * card);
  bool WaitForExtraPayment(MTGCardInstance* card);
  void cleanup();
  string startupGameSerialized;
  bool parseLine(const string& s);
  virtual void logAction(const string& s);
  bool processAction(const string& s, bool swapPlayer = false);
  bool processActions(bool undo
                    #ifdef TESTSUITE
                    , TestSuiteGame* testgame
                    #endif
                      );
  friend ostream& operator<<(ostream&, const GameObserver&);
  bool mLoading;
  void nextGamePhase();
  void shuffleLibrary(Player* p);
  Player* createPlayer(const string& playerMode
                  #ifdef TESTSUITE
                  , TestSuiteGame* testgame
                  #endif //TESTSUITE
                    );

 public:
  int currentPlayerId;
  CombatStep combatStep;
  int turn;
  int forceShuffleLibraries();
  int targetListIsSet(MTGCardInstance * card);
  PhaseRing * phaseRing;
  vector<list<Phase*> >gameTurn;
  int cancelCurrentAction();
  ExtraCosts * mExtraPayment;
  int oldGamePhase;
  TargetChooser * targetChooser;
  DuelLayers * mLayers;
  ReplacementEffects *replacementEffects;
  vector<Player *> players; //created outside
  time_t startedAt;
  Rules * mRules;
  MTGCardInstance* ExtraRules;
  Trash* mTrash;

  GameType gameType() const { return mGameType; };
  TargetChooser * getCurrentTargetChooser();
  void stackObjectClicked(Interruptible * action);

  int cardClickLog(bool log, Player* clickedPlayer, MTGGameZone* zone, MTGCardInstance*backup, size_t index, int toReturn);
  int cardClick(MTGCardInstance * card, MTGAbility *ability);
  int cardClick(MTGCardInstance * card, int abilityType);
  int cardClick(MTGCardInstance * card,Targetable * _object = NULL, bool log = true);
  GamePhase getCurrentGamePhase();
  void setCurrentGamePhase(GamePhase phase) { mCurrentGamePhase = phase; };
  const char * getCurrentGamePhaseName();
  const char * getNextGamePhaseName();
  void nextCombatStep();
  void userRequestNextGamePhase(bool allowInterrupt = true, bool log = true);
  void cleanupPhase();
  void nextPlayer();

#ifdef TESTSUITE
  void loadTestSuitePlayer(int playerId, TestSuiteGame* testSuite);
#endif //TESTSUITE
  virtual void loadPlayer(int playerId, PlayerType playerType = PLAYER_TYPE_HUMAN, int decknb=0, bool premadeDeck=false);
  void loadPlayer(int playerId, Player* player);

  Player * currentPlayer;
  Player * currentActionPlayer;
  Player * isInterrupting;
  Player * opponent();
  Player * nextTurnsPlayer();
  Player * currentlyActing();
  GameObserver(WResourceManager* output = 0, JGE* input = 0);
  virtual ~GameObserver();
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
  virtual void Update(float dt);
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
  bool load(const string& s, bool undo = false
#ifdef TESTSUITE
            , TestSuiteGame* testgame = 0
#endif
          );
  bool undo();
  bool isLoading(){ return mLoading; };
  void Mulligan(Player* player = NULL);
  Player* getPlayer(size_t index) { return players[index];};
  bool isStarted() { return (mLayers!=NULL);};
  RandomGenerator* getRandomGenerator() { return &randomGenerator; };
  WResourceManager* getResourceManager() { if(this) return mResourceManager;else return 0;};
  CardSelectorBase* getCardSelector() { return mLayers->mCardSelector;};
  bool operator==(const GameObserver& aGame);
  JGE* getInput(){return mJGE;};
  DeckManager* getDeckManager(){ return mDeckManager; };
  void dumpAssert(bool val);
  void resetStartupGame();
  void setLoser(Player* aPlayer) {
      gameOver = aPlayer;
  };

  bool didWin(Player* aPlayer = 0) const {
      if(!gameOver) {
          // nobody won
          return false;
      } else if(!aPlayer) {
          // somebody won and we don't care who
          return true;
      } else if(gameOver == aPlayer) {
          // aPlayer lost
          return false;
      } else {
          // aPlayer won
          return true;
      }
  };
};

#ifdef NETWORK_SUPPORT
class NetworkGameObserver : public GameObserver
{
protected:
	JNetwork* mpNetworkSession;
	bool mSynchronized;
	bool mForwardAction;
	virtual void logAction(const string& s);
public:
	// no serverIp means a server is being instantiated, otherwise a client
	NetworkGameObserver(JNetwork* pNetwork, WResourceManager* output = 0, JGE* input = 0);
	virtual ~NetworkGameObserver();
	virtual void loadPlayer(int playerId, PlayerType playerType = PLAYER_TYPE_HUMAN, int decknb=0, bool premadeDeck=false);
	virtual void Update(float dt);
	void synchronize();
	static void loadPlayer(void*pThis, stringstream& in, stringstream& out);
	static void sendAction(void*pThis, stringstream& in, stringstream& out);
	static void synchronize(void*pThis, stringstream& in, stringstream& out);
	static void ignoreResponse(void*pThis, stringstream& in, stringstream& out){};
};
#endif


#endif
