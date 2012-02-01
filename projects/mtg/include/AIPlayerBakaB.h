#include "config.h"
#ifdef AI_CHANGE_TESTING

#ifndef _AI_PLAYER_BAKA_B_H_
#define _AI_PLAYER_BAKA_B_H_

#include "AIPlayerBaka.h"
#include "AllAbilities.h"

class AIStats;
class AIHints;


class AIPlayerBakaB: public AIPlayerBaka{
protected:
    int orderBlockers();
    int combatDamages();
    int interruptIfICan();
    int chooseAttackers();
    int chooseBlockers();
    int canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy);
    int effectBadOrGood(MTGCardInstance * card, int mode = MODE_PUTINTOPLAY, TargetChooser * tc = NULL);


    // returns 1 if the AI algorithm supports a given cost (ex:simple mana cost), 0 otherwise
    int CanHandleCost(ManaCost * cost); 

    //Tries to play an ability recommended by the deck creator
    int selectHintAbility();

    MTGCardInstance * chooseCard(TargetChooser * tc, MTGCardInstance * source, int random = 0);
    int selectMenuOption();
    int useAbility();

    AIStats * getStats();

    MTGCardInstance * FindCardToPlay(ManaCost * potentialMana, const char * type);

    bool payTheManaCost(ManaCost * cost, MTGCardInstance * card = NULL,vector<MTGAbility*> gotPayment = vector<MTGAbility*>());
    int getCreaturesInfo(Player * player, int neededInfo = INFO_NBCREATURES , int untapMode = 0, int canAttack = 0);
    ManaCost * getPotentialMana(MTGCardInstance * card = NULL);
    int selectAbility();

    //used by MomirPlayer, hence protected instead of private
    virtual int getEfficiency(OrderedAIAction * action);

 public:

    AIPlayerBakaB(GameObserver *observer, string deckFile, string deckfileSmall, string avatarFile, MTGDeck * deck = NULL);
    virtual int Act(float dt);
    void initTimer();
    virtual int computeActions();
    virtual void Render();
    virtual int receiveEvent(WEvent * event);
    virtual ~AIPlayerBakaB();
    int affectCombatDamages(CombatStep step);
    int canHandleCost(MTGAbility * ability);
    int chooseTarget(TargetChooser * tc = NULL, Player * forceTarget =NULL,MTGCardInstance * Choosencard = NULL,bool checkonly = false);

    //used by AIHInts, therefore public instead of private :/
    int createAbilityTargets(MTGAbility * a, MTGCardInstance * c, RankingContainer& ranking);
};

#endif

#endif
