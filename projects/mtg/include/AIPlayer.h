/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _IAPLAYER_H
#define _IAPLAYER_H

#include "Player.h"

#include <queue>
using std::queue;

#define INFO_NBCREATURES 0
#define INFO_CREATURESPOWER 1
#define INFO_CREATURESRANK 2
#define INFO_CREATURESTOUGHNESS 3
#define INFO_CREATURESATTACKINGPOWER 4


class AIStats;

class AIAction
{
protected:
    int efficiency;
    static int currentId;
public:
    MTGAbility * ability;
    NestedAbility * nability;
    Player * player;
    int id;
    bool checked;
    MTGCardInstance * click;
    MTGCardInstance * target; // TODO Improve

    AIAction(MTGAbility * a, MTGCardInstance * c, MTGCardInstance * t = NULL)
        : efficiency(-1), ability(a), player(NULL), click(c), target(t)
    {
        id = currentId++;
    };

    AIAction(MTGCardInstance * c, MTGCardInstance * t = NULL)
        : efficiency(-1), ability(NULL), player(NULL), click(c), target(t)
    {
        id = currentId++;
    };

    AIAction(Player * p)
        : efficiency(-1), ability(NULL), player(p), click(NULL), target(NULL)
    {
    };

    int getEfficiency();
    int Act();
};

// compares Abilities efficiency
class CmpAbilities
{ 
public:
    bool operator()(const AIAction& a1, const AIAction& a2) const
    {
        AIAction* a1Ptr = const_cast<AIAction*>(&a1);
        AIAction* a2Ptr = const_cast<AIAction*>(&a2);
        int e1 = a1Ptr->getEfficiency();
        int e2 = a2Ptr->getEfficiency();
        if (e1 == e2) return a1Ptr->id < a2Ptr->id;
        return (e1 > e2);
    }
};

typedef std::map<AIAction, int, CmpAbilities> RankingContainer;

class AIPlayer: public Player{
protected:
    //Variables used by Test suite
    MTGCardInstance * nextCardToPlay;
    queue<AIAction *> clickstream;
    bool tapLandsForMana(ManaCost * cost, MTGCardInstance * card = NULL);
    int orderBlockers();
    int combatDamages();
    int interruptIfICan();
    int chooseAttackers();
    int chooseBlockers();
    int canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy);
    int effectBadOrGood(MTGCardInstance * card, int mode = MODE_PUTINTOPLAY, TargetChooser * tc = NULL);
    int getCreaturesInfo(Player * player, int neededInfo = INFO_NBCREATURES , int untapMode = 0, int canAttack = 0);
    AIStats * getStats();

    // returns 1 if the AI algorithm supports a given cost (ex:simple mana cost), 0 otherwise (ex: cost involves Sacrificing a target)
    int CanHandleCost(ManaCost * cost); 

public:
    AIStats * stats;
    int agressivity;
    bool Checked;
    bool forceBestAbilityUse;
    void End(){};
    virtual int displayStack() {return 0;};
    int receiveEvent(WEvent * event);
    void Render();
    ManaCost * getPotentialMana(MTGCardInstance * card = NULL);
    AIPlayer(MTGDeck * deck, string deckFile, string deckFileSmall);
    virtual ~AIPlayer();
    virtual MTGCardInstance * chooseCard(TargetChooser * tc, MTGCardInstance * source, int random = 0);
    virtual int chooseTarget(TargetChooser * tc = NULL, Player * forceTarget =NULL);
    virtual int Act(float dt);
    virtual int affectCombatDamages(CombatStep);
    int isAI(){return 1;};
    int canHandleCost(MTGAbility * ability);
    int selectAbility();
    int createAbilityTargets(MTGAbility * a, MTGCardInstance * c, RankingContainer& ranking);
    int useAbility();
    virtual int getEfficiency(AIAction * action);

};


class AIPlayerBaka: public AIPlayer{
 protected:
  int oldGamePhase;
  float timer;
  MTGCardInstance * FindCardToPlay(ManaCost * potentialMana, const char * type);
 public:
  int deckId;
  AIPlayerBaka(MTGDeck * deck, string deckFile, string deckfileSmall, string avatarFile);
  virtual int Act(float dt);
  void initTimer();
  virtual int computeActions();
};

class AIPlayerFactory{
 public:
  AIPlayer * createAIPlayer(MTGAllCards * collection, Player * opponent, int deckid = 0);
};


#endif
