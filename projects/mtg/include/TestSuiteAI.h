#ifndef _TESTSUITE_AI_H_
#define _TESTSUITE_AI_H_

#ifdef TESTSUITE

#define MAX_TESTSUITE_ACTIONS 100
#define MAX_TESTUITE_CARDS 100

#include "Threading.h"
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


class TestSuiteGame;
class TestSuite;
class TestSuiteAI;
class TestSuiteState
{
public:
    int phase;
    void parsePlayerState(int playerId, string s);
    TestSuiteState();
    ~TestSuiteState();

    vector<TestSuiteAI*> players;
    void cleanup(TestSuiteGame* tsGame);
};

class TestSuiteGame
{
    friend class TestSuiteAI;
    friend class TestSuite;
protected:
    string filename;
    int summoningSickness;
    bool forceAbility;
    GameType gameType;
    unsigned int seed;
    int aiMaxCalls;
    TestSuiteState endState;
    TestSuiteState initState;
    TestSuiteActions actions;
    float timerLimit;
    bool isOK;
    int currentAction;
    GameObserver* observer;

    static boost::mutex mMutex;
    virtual void handleResults(bool wasAI, int error);
    TestSuite* testsuite;
    bool load();

public:
    ~TestSuiteGame();
    TestSuiteGame(TestSuite* testsuite);
    TestSuiteGame(TestSuite* testsuite, string _filename);
    void initGame();
    void assertGame();
    MTGPlayerCards * buildDeck(Player* player, int playerId);
    GameType getGameType() { return gameType; };
    string getNextAction();
    Interruptible * getActionByMTGId(int mtgid);
    static int Log(const char * text);
    void setObserver(GameObserver* anObserver) {observer = anObserver; };
};

class TestSuite : public TestSuiteGame
{
private:
    int currentfile;
    int nbfiles;
    string files[1024];

    void cleanup();
    vector<boost::thread> mWorkerThread;
    Rules* mRules;
    bool mProcessing;

public:
    int startTime, endTime;
    unsigned int seed;
    int nbFailed, nbTests, nbAIFailed, nbAITests;
    TestSuite(const char * filename);
    void initGame(GameObserver* g);
    void pregameTests();
    int loadNext();
    string getNextFile() {
        boost::mutex::scoped_lock lock(mMutex);
        if (currentfile >= nbfiles) return "";
        currentfile++;
        return files[currentfile - 1];
    };
    static void ThreadProc(void* inParam);
    void setRules(Rules* rules) {mRules = rules;};
    void handleResults(bool wasAI, int error);
};

class TestSuiteAI:public AIPlayerBaka
{
private:
    MTGCardInstance * getCard(string action);
    float timer;
    TestSuiteGame * suite;

public:
    TestSuiteAI(TestSuiteGame *tsGame, int playerId);
    virtual int Act(float dt);
    virtual int displayStack();
    bool summoningSickness() {return (suite->summoningSickness == 1); }
};

#endif
#endif
