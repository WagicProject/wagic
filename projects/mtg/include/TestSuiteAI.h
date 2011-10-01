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


class TestSuite;
class TestSuiteAI;
class TestSuiteState
{
public:
    int phase;
    void parsePlayerState(int playerId, string s);
    TestSuiteState();
    ~TestSuiteState();

    TestSuiteAI* players[2];
    void cleanup(TestSuite*);
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

    int load(const char * filename);
    void cleanup();

public:
    /* but only used by the testsuite classes */
    float timerLimit;
    int aiMaxCalls;
    int currentAction;
    int summoningSickness;

    TestSuiteState initState;
    string getNextAction();
    MTGPlayerCards * buildDeck(Player*, int playerId);
    Interruptible * getActionByMTGId(GameObserver* observer, int mtgid);
    int assertGame(GameObserver*);

public:
    int startTime, endTime;
    int gameType;
    unsigned int seed;
    int nbFailed, nbTests, nbAIFailed, nbAITests;
    TestSuite(const char * filename);
    void initGame(GameObserver* g);
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
    TestSuiteAI(GameObserver *observer, TestSuite * suite, int playerId);
    virtual int Act(float dt);
    virtual int displayStack();
    bool summoningSickness() {return (suite->summoningSickness == 1); }
};

#endif
#endif
