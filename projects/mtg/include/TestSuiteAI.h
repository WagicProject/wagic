#ifndef _TESTSUITE_AI_H_
#define _TESTSUITE_AI_H_

#define MAX_TESTSUITE_ACTIONS 100
#define MAX_TESTUITE_CARDS 100

#include "../include/AIPlayer.h"

class TestSuiteActions{
 public:
  int nbitems;
  string actions[MAX_TESTSUITE_ACTIONS];
  void add(string action);
  TestSuiteActions();
  void cleanup();
};

class TestSuitePlayerZone{
 public:
  int cards[MAX_TESTUITE_CARDS];
  int nbitems;
  void add(int cardid);
  TestSuitePlayerZone();
  void cleanup();
};

class TestSuitePlayerData{
 public:
  int life;
  ManaCost * manapool;
  TestSuitePlayerZone zones[5];
  TestSuitePlayerData();
  ~TestSuitePlayerData();
  void cleanup();

};



class TestSuite;
class TestSuiteState{
 public:
  int phase;
  void parsePlayerState(int playerId, string s,TestSuite * suite);
  TestSuiteState();
  TestSuitePlayerData playerData[2];
  void cleanup();
};

class TestSuite{
 public:
  MTGAllCards* collection;
  int summoningSickness;
  int gameType;
  float timerLimit;
  int currentAction;
  TestSuiteState initState;
  TestSuiteState endState;
  TestSuiteActions actions;
  string files[1024];
  int nbfiles;
  int currentfile;
  int load(const char * filename);
  TestSuite(const char * filename,MTGAllCards* _collection);
  void initGame();
  int assertGame();
  MTGPlayerCards * buildDeck(int playerId);
  string getNextAction();
  int phaseStrToInt(string s);
  MTGCardInstance * getCardByMTGId(int mtgid);
  Interruptible * getActionByMTGId(int mtgid);
  int loadNext();
  void cleanup();
  int Log(const char * text);
  int getMTGId(string name);

};

class TestSuiteAI:public AIPlayerBaka{
 public:
  TestSuite * suite;
  float timer;
  int playMode;
  TestSuiteAI(TestSuite * suite, int playerId);
  virtual int Act(float dt);
  virtual int displayStack();

};



#endif
