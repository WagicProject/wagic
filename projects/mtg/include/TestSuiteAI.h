#ifndef _TESTSUITE_AI_H_
#define _TESTSUITE_AI_H_

#ifdef TESTSUITE

#define MAX_TESTSUITE_ACTIONS 100
#define MAX_TESTUITE_CARDS 100

#include "AIPlayerBaka.h"

class TestSuiteActions
{
public:
    int nbitems;
    string actions[MAX_TESTSUITE_ACTIONS];
    void add(string action);
    TestSuiteActions();
    void cleanup();
};

class TestSuitePlayerZone
{
public:
    int cards[MAX_TESTUITE_CARDS];
    int nbitems;
    void add(int cardid);
    TestSuitePlayerZone();
    void cleanup();
};

class TestSuitePlayerData
{
public:
    int life;
    ManaCost * manapool;
    TestSuitePlayerZone zones[5];
    TestSuitePlayerData();
    ~TestSuitePlayerData();
    void cleanup();

};

class TestSuite;
class TestSuiteState
{
public:
    int phase;
    void parsePlayerState(int playerId, string s);
    TestSuiteState();
    TestSuitePlayerData playerData[2];
    void cleanup();
};

class TestSuitePregame
{
public:
    virtual void performTest() = 0;
};

class TestSuite
{
private:
    int currentfile;
    int nbfiles;
    string files[1024];
    TestSuiteState endState;
    TestSuiteActions actions;
    bool forceAbility;
    int summoningSickness;


    int load(const char * filename);
    void cleanup();

public:
    /* but only used by the testsuite classes */
    float timerLimit;
    int aiMaxCalls;
    int currentAction;

    TestSuiteState initState;
    string getNextAction();
    MTGPlayerCards * buildDeck(int playerId);
    Interruptible * getActionByMTGId(int mtgid);
    int assertGame();

public:
    int startTime, endTime;
    int gameType;
    unsigned int seed;
    int nbFailed, nbTests, nbAIFailed, nbAITests;
    TestSuite(const char * filename);
    void initGame();
    void pregameTests();
    int loadNext();
    static int Log(const char * text);

};

// TODO This should inherit from AIPlayer instead!
class TestSuiteAI:public AIPlayerBaka
{
private:
    MTGCardInstance * getCard(string action);
    float timer;
    TestSuite * suite;

public:
    TestSuiteAI(TestSuite * suite, int playerId);
    virtual int Act(float dt);
    virtual int displayStack();
};

#endif
#endif
