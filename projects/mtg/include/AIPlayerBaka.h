#ifndef _AI_PLAYER_BAKA_H_
#define _AI_PLAYER_BAKA_H_

#include "AIPlayer.h"

class AIStats;
class AIHints;


//Would love to define those classes as private nested classes inside of AIPlayerBaka, but they are used by AIHints (which itself should be known only by AIPlayerBaka anyways)
// Any way to clean this up and make all the AIPlayerBaka universe (AIHints, AIPlayerBaka, OrderedAIAction) "closed" ?
class OrderedAIAction: public AIAction
{
protected:
    int efficiency;

public:

    
    OrderedAIAction(MTGAbility * a, MTGCardInstance * c, MTGCardInstance * t = NULL)
        : AIAction(a, c, t), efficiency(-1)
    {
    };

    OrderedAIAction(MTGCardInstance * c, MTGCardInstance * t = NULL);

    OrderedAIAction(Player * p)//player targeting through spells
        : AIAction(p), efficiency(-1)
    {
    };

    OrderedAIAction(MTGAbility * a, MTGCardInstance * c, vector<Targetable*>targetCards)
        : AIAction(a,  c, targetCards), efficiency(-1)
    {
    };

    OrderedAIAction(MTGAbility * a, Player * p, MTGCardInstance * c)//player targeting through abilities.
        : AIAction(a, p, c), efficiency(-1)
    {
    };
    int getEfficiency();
};

// compares Abilities efficiency
class CmpAbilities
{ 
public:
    bool operator()(const OrderedAIAction& a1, const OrderedAIAction& a2) const
    {
        OrderedAIAction* a1Ptr = const_cast<OrderedAIAction*>(&a1);
        OrderedAIAction* a2Ptr = const_cast<OrderedAIAction*>(&a2);
        int e1 = a1Ptr->getEfficiency();
        int e2 = a2Ptr->getEfficiency();
        if (e1 == e2) return a1Ptr->id < a2Ptr->id;
        return (e1 > e2);
    }
};

typedef std::map<OrderedAIAction, int, CmpAbilities> RankingContainer;


class AIPlayerBaka: public AIPlayer{
private:
    int orderBlockers();
    int combatDamages();
    int interruptIfICan();
    int chooseAttackers();
    int chooseBlockers();
    int canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy);
    int effectBadOrGood(MTGCardInstance * card, int mode = MODE_PUTINTOPLAY, TargetChooser * tc = NULL);


    // returns 1 if the AI algorithm supports a given cost (ex:simple mana cost), 0 otherwise (ex: cost involves Sacrificing a target)
    int CanHandleCost(ManaCost * cost); 

    //Tries to play an ability recommended by the deck creator
    int selectHintAbility();

    vector<MTGAbility*> canPayMana(MTGCardInstance * card = NULL,ManaCost * mCost = NULL);
    vector<MTGAbility*> canPaySunBurst(ManaCost * mCost = NULL);

    MTGCardInstance * chooseCard(TargetChooser * tc, MTGCardInstance * source, int random = 0);
    int selectMenuOption();
    int useAbility();

    AIStats * getStats();

 protected:
    MTGCardInstance * nextCardToPlay;
    AIHints * hints;
    AIStats * stats;
    int oldGamePhase;
    float timer;
    MTGCardInstance * FindCardToPlay(ManaCost * potentialMana, const char * type);

    //used by MomirPlayer, hence protected instead of private
    virtual int getEfficiency(OrderedAIAction * action);
    bool payTheManaCost(ManaCost * cost, MTGCardInstance * card = NULL,vector<MTGAbility*> gotPayment = vector<MTGAbility*>());
    int getCreaturesInfo(Player * player, int neededInfo = INFO_NBCREATURES , int untapMode = 0, int canAttack = 0);
    ManaCost * getPotentialMana(MTGCardInstance * card = NULL);
    int selectAbility();

 public:
    enum {
        INFO_NBCREATURES,
        INFO_CREATURESPOWER,
        INFO_CREATURESRANK,
        INFO_CREATURESTOUGHNESS,
        INFO_CREATURESATTACKINGPOWER
    };

     vector<MTGAbility*>gotPayments;
    int deckId;
    AIPlayerBaka(string deckFile, string deckfileSmall, string avatarFile, MTGDeck * deck = NULL);
    virtual int Act(float dt);
    void initTimer();
    virtual int computeActions();
    virtual void Render();
    virtual int receiveEvent(WEvent * event);
    virtual ~AIPlayerBaka();
    int affectCombatDamages(CombatStep step);
    int canHandleCost(MTGAbility * ability);
    int chooseTarget(TargetChooser * tc = NULL, Player * forceTarget = NULL,MTGCardInstance * Chosencard = NULL,bool checkonly = false);

    //used by AIHInts, therefore public instead of private :/
    int createAbilityTargets(MTGAbility * a, MTGCardInstance * c, RankingContainer& ranking);
};

#endif