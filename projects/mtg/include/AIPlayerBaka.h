#ifndef _AI_PLAYER_BAKA_H_
#define _AI_PLAYER_BAKA_H_

#include "AIPlayer.h"
#include "AllAbilities.h"

class AIStats;
class AIHints;


//Would love to define those classes as private nested classes inside of AIPlayerBaka, but they are used by AIHints (which itself should be known only by AIPlayerBaka anyways)
// Any way to clean this up and make all the AIPlayerBaka universe (AIHints, AIPlayerBaka, OrderedAIAction) "closed" ?
class OrderedAIAction: public AIAction
{
protected:
    Player * getPlayerTarget();

public:
    int efficiency;
    
    OrderedAIAction(AIPlayer * owner, MTGAbility * a, MTGCardInstance * c, MTGCardInstance * t = NULL)
        : AIAction(owner, a, c, t), efficiency(-1)
    {
    };

    OrderedAIAction(AIPlayer * owner, MTGCardInstance * c, MTGCardInstance * t = NULL);

    OrderedAIAction(AIPlayer * owner, Player * p)//player targeting through spells
        : AIAction(owner,p), efficiency(-1)
    {
    };

    OrderedAIAction(AIPlayer * owner, MTGAbility * a, MTGCardInstance * c, vector<Targetable*>targetCards)
        : AIAction(owner,a,  c, targetCards), efficiency(-1)
    {
    };

    OrderedAIAction(AIPlayer * owner, MTGAbility * a, Player * p, MTGCardInstance * c)//player targeting through abilities.
        : AIAction(owner, a, p, c), efficiency(-1)
    {
    };
    int getEfficiency();

    // Functions depending on the type of Ability
    int getEfficiency(AADamager * aad);

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
 protected:
    virtual int orderBlockers();
    virtual int combatDamages();
    virtual int interruptIfICan();
    virtual int chooseAttackers();
    virtual int chooseBlockers();
    virtual int canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy);
    virtual int effectBadOrGood(MTGCardInstance * card, int mode = MODE_PUTINTOPLAY, TargetChooser * tc = NULL);


    // returns 1 if the AI algorithm supports a given cost (ex:simple mana cost), 0 otherwise (ex: cost involves Sacrificing a target)
    virtual int CanHandleCost(ManaCost * cost, MTGCardInstance * card = NULL); 

    //Tries to play an ability recommended by the deck creator
    virtual int selectHintAbility();

    virtual vector<MTGAbility*> canPayMana(MTGCardInstance * card = NULL,ManaCost * mCost = NULL, map<MTGCardInstance*, bool>usedCards = map<MTGCardInstance*,bool>());
    virtual vector<MTGAbility*> canPaySunBurst(ManaCost * mCost = NULL);

    virtual MTGCardInstance * chooseCard(TargetChooser * tc, MTGCardInstance * source, int random = 0);
    virtual int selectMenuOption();

    virtual AIStats * getStats();

    MTGCardInstance * nextCardToPlay;
    AIHints * hints;
    AIStats * stats;
    int oldGamePhase;
    float timer;
    virtual MTGCardInstance * FindCardToPlay(ManaCost * potentialMana, const char * type);

    //used by MomirPlayer, hence protected instead of private
    virtual int getEfficiency(OrderedAIAction * action);
    virtual int getEfficiency(MTGAbility * ability);
    virtual bool payTheManaCost(ManaCost * cost, MTGCardInstance * card = NULL,vector<MTGAbility*> gotPayment = vector<MTGAbility*>());
    virtual int getCreaturesInfo(Player * player, int neededInfo = INFO_NBCREATURES , int untapMode = 0, int canAttack = 0);
    virtual ManaCost * getPotentialMana(MTGCardInstance * card = NULL);
    virtual int selectAbility();

 public:
    enum {
        INFO_NBCREATURES,
        INFO_CREATURESPOWER,
        INFO_CREATURESRANK,
        INFO_CREATURESTOUGHNESS,
        INFO_CREATURESATTACKINGPOWER
    };

    vector<MTGAbility*>gotPayments;
    AIPlayerBaka(GameObserver *observer, string deckFile, string deckfileSmall, string avatarFile, MTGDeck * deck = NULL);
    virtual int Act(float dt);
    void initTimer();
    virtual int computeActions();
    virtual void Render();
    virtual int receiveEvent(WEvent * event);
    virtual ~AIPlayerBaka();
    virtual int affectCombatDamages(CombatStep step);
    virtual int canHandleCost(MTGAbility * ability);
    virtual int chooseTarget(TargetChooser * tc = NULL, Player * forceTarget = NULL,MTGCardInstance * Chosencard = NULL,bool checkonly = false);

    //used by AIHInts, therefore public instead of private :/
    virtual int createAbilityTargets(MTGAbility * a, MTGCardInstance * c, RankingContainer& ranking);
};

#endif
