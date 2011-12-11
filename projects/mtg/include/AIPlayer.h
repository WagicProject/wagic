/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/

 AIPlayer is the interface to represent a CPU Player.
 At its core, AIPlayer inherits from Player, and its children need to implement the function "Act" which
 pretty much handles all the logic.
 A sample implementation can be found in AIPlayerBaka.

 Ideally, mid-term, AIPlayer will handle all the mechanical tasks (clicking on cards, etc...) while its children are just in charge of the logic

 */

#ifndef _IAPLAYER_H
#define _IAPLAYER_H

#include "Player.h"
#include "config.h"

#include <queue>
using std::queue;

class AIStats;
class AIPlayer;

class AIAction
{
public:
    AIPlayer * owner;
    MTGAbility * ability;
    NestedAbility * nability;
    Player * player;
    int id;
    MTGCardInstance * click;
    MTGCardInstance * target; // TODO Improve
    vector<Targetable*>mAbilityTargets;
    Targetable * playerAbilityTarget;
    //player targeting through abilities is handled completely seperate from spell targeting.
    
    AIAction(AIPlayer * owner, MTGAbility * a, MTGCardInstance * c, MTGCardInstance * t = NULL)
        : owner(owner), ability(a), player(NULL), click(c), target(t),playerAbilityTarget(NULL)
    {
    };

    AIAction(AIPlayer * owner, MTGCardInstance * c, MTGCardInstance * t = NULL);

    AIAction(AIPlayer * owner, Player * p)//player targeting through spells
        :  owner(owner), ability(NULL), player(p), click(NULL), target(NULL),playerAbilityTarget(NULL)
    {
    };

    AIAction(AIPlayer * owner, MTGAbility * a, MTGCardInstance * c, vector<Targetable*>targetCards)
        :  owner(owner), ability(a), player(NULL), click(c), mAbilityTargets(targetCards),playerAbilityTarget(NULL)
    {
    };

    AIAction(AIPlayer * owner, MTGAbility * a, Player * p, MTGCardInstance * c)//player targeting through abilities.
        : owner(owner), ability(a), click(c),target(NULL), playerAbilityTarget(p)
    {
    };
    int Act();
    int clickMultiAct(vector<Targetable*>&actionTargets);
};



class AIPlayer: public Player{

private:
    static int totalAIDecks; //a cache that counts the number of AI deck files in the AI folder. see getTotalAIDecks() below.
    static int countTotalDecks(int lower, int higher, int current);

protected:
    bool mFastTimerMode;
    queue<AIAction *> clickstream;
    int clickMultiTarget(TargetChooser * tc,vector<Targetable*>&potentialTargets);
    int clickSingleTarget(TargetChooser * tc,vector<Targetable*>&potentialTargets, MTGCardInstance * Choosencard = NULL);
    RandomGenerator randomGenerator;

public:

    //These variables are used by TestSuite and Rules.cpp... TODO change that?
    int agressivity;
    bool forceBestAbilityUse;

    void End(){};
    virtual int displayStack() {return 0;};
    virtual int receiveEvent(WEvent * event);
    virtual void Render();

    AIPlayer(GameObserver *observer, string deckFile, string deckFileSmall, MTGDeck * deck = NULL);
    virtual ~AIPlayer();
    
    virtual int chooseTarget(TargetChooser * tc = NULL, Player * forceTarget = NULL, MTGCardInstance * Chosencard = NULL, bool checkonly = false) = 0;
    virtual int affectCombatDamages(CombatStep) = 0;
    virtual int Act(float dt) = 0;
    
    int isAI(){return 1;};

    void setFastTimerMode(bool mode = true) { mFastTimerMode = mode; };
    RandomGenerator* getRandomGenerator(){return &randomGenerator;};

    bool parseLine(const string& s);

    static int getTotalAIDecks();
    static void invalidateTotalAIDecks();
};


class AIPlayerFactory{
 public:
  AIPlayer * createAIPlayer(GameObserver *observer, MTGAllCards * collection, Player * opponent, int deckid = 0);
#ifdef AI_CHANGE_TESTING
  AIPlayer * createAIPlayerTest(GameObserver *observer, MTGAllCards * collection, Player * opponent, string folder);
#endif
};


#endif
