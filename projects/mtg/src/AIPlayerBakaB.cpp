#include "PrecompiledHeader.h"
#ifdef AI_CHANGE_TESTING

#include "AIPlayerBakaB.h"
#include "CardDescriptor.h"
#include "AIStats.h"
#include "ExtraCost.h"
#include "GuiCombat.h"
#include "AIHints.h"
#include "ManaCostHybrid.h"


//
// Abilities/Target Selection
//


MTGCardInstance * AIPlayerBakaB::chooseCard(TargetChooser * tc, MTGCardInstance * source, int random)
{
    return AIPlayerBaka::chooseCard(tc, source, random);
}

bool AIPlayerBakaB::payTheManaCost(ManaCost * cost, MTGCardInstance * target, vector<MTGAbility*>gotPayments)
{
    return AIPlayerBaka::payTheManaCost(cost, target, gotPayments);
}

int AIPlayerBakaB::getEfficiency(OrderedAIAction * action)
{
   return AIPlayerBaka::getEfficiency(action);
}

ManaCost * AIPlayerBakaB::getPotentialMana(MTGCardInstance * target)
{
  return AIPlayerBaka::getPotentialMana(target);
}

int AIPlayerBakaB::CanHandleCost(ManaCost * cost)
{
   return AIPlayerBaka::CanHandleCost(cost);
}

int AIPlayerBakaB::canHandleCost(MTGAbility * ability)
{
    return AIPlayerBaka::canHandleCost(ability);
}




int AIPlayerBakaB::createAbilityTargets(MTGAbility * a, MTGCardInstance * c, RankingContainer& ranking)
{
    return AIPlayerBaka::createAbilityTargets(a, c, ranking);
}

int AIPlayerBakaB::selectHintAbility()
{
    return AIPlayerBaka::selectHintAbility();
}

int AIPlayerBakaB::selectAbility()
{
    return AIPlayerBaka::selectAbility();
}

int AIPlayerBakaB::interruptIfICan()
{
    return AIPlayerBaka::interruptIfICan();
}

int AIPlayerBakaB::effectBadOrGood(MTGCardInstance * card, int mode, TargetChooser * tc)
{
    return AIPlayerBaka::effectBadOrGood(card, mode, tc);
}

int AIPlayerBakaB::chooseTarget(TargetChooser * _tc, Player * forceTarget,MTGCardInstance * chosenCard,bool checkOnly)
{
    return AIPlayerBaka::chooseTarget( _tc, forceTarget, chosenCard, checkOnly);
}

int AIPlayerBakaB::selectMenuOption()
{
     return AIPlayerBaka::selectMenuOption();
}

MTGCardInstance * AIPlayerBakaB::FindCardToPlay(ManaCost * pMana, const char * type)
{
    return AIPlayerBaka::FindCardToPlay(pMana, type);
}


void AIPlayerBakaB::initTimer()
{
    return AIPlayerBaka::initTimer();
}

int AIPlayerBakaB::computeActions()
{
     return AIPlayerBaka::computeActions();

};


//
// Combat //
//

int AIPlayerBakaB::getCreaturesInfo(Player * player, int neededInfo, int untapMode, int canAttack)
{
     return AIPlayerBaka::getCreaturesInfo(player, neededInfo, untapMode, canAttack);
}

int AIPlayerBakaB::chooseAttackers()
{
     return AIPlayerBaka::chooseAttackers();

}

/* Can I first strike my oponent and get away with murder ? */
int AIPlayerBakaB::canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy)
{
    return AIPlayerBaka::canFirstStrikeKill(card, ennemy);
}

int AIPlayerBakaB::chooseBlockers()
{
    return AIPlayerBaka::chooseBlockers();
}

int AIPlayerBakaB::orderBlockers()
{
    return AIPlayerBaka::orderBlockers();
}

int AIPlayerBakaB::affectCombatDamages(CombatStep step)
{
    return AIPlayerBaka::affectCombatDamages(step);
}

//TODO: Deprecate combatDamages
int AIPlayerBakaB::combatDamages()
{
    return AIPlayerBaka::combatDamages();
}


//
// General
//

AIStats * AIPlayerBakaB::getStats()
{
    return AIPlayerBaka::getStats();
}


void AIPlayerBakaB::Render()
{
    return AIPlayerBaka::Render();
}

int AIPlayerBakaB::receiveEvent(WEvent * event)
{
    return AIPlayerBaka::receiveEvent(event);
}


AIPlayerBakaB::AIPlayerBakaB(GameObserver *observer, string file, string fileSmall, string avatarFile, MTGDeck * deck) :
   AIPlayerBaka(observer, file, fileSmall, avatarFile,  deck)
{

}

int AIPlayerBakaB::Act(float dt)
{
    return AIPlayerBaka::Act(dt);
};

AIPlayerBakaB::~AIPlayerBakaB() {
}




#endif

