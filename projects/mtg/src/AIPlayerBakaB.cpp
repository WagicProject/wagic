#include "PrecompiledHeader.h"
#ifdef AI_CHANGE_TESTING

#include "AIPlayerBakaB.h"
#include "CardDescriptor.h"
#include "AIStats.h"
#include "AllAbilities.h"
#include "ExtraCost.h"
#include "GuiCombat.h"
#include "AIHints.h"
#include "ManaCostHybrid.h"


//
// Abilities/Target Selection
//


MTGCardInstance * AIPlayerBakaB::chooseCard(TargetChooser * tc, MTGCardInstance * source, int random)
{
    for (int i = 0; i < game->hand->nb_cards; i++)
    {
        MTGCardInstance * card = game->hand->cards[i];
        if (!tc->alreadyHasTarget(card) && tc->canTarget(card))
        {
            return card;
        }
    }
    return NULL;
}

bool AIPlayerBakaB::payTheManaCost(ManaCost * cost, MTGCardInstance * target,vector<MTGAbility*>gotPayments)
{
    DebugTrace(" AI attempting to pay a mana cost." << endl
            << "-  Target: " << (target ? target->name : "None" ) << endl
            << "-  Cost: " << (cost ? cost->toString() : "NULL") );
    if(cost && !cost->getConvertedCost())
    {
        DebugTrace("Card or Ability was free to play.  ");
        if(!cost->hasX())//don't return true if it contains {x} but no cost, locks ai in a loop. ie oorchi hatchery cost {x}{x} to play.
        return true;
        //return true if cost does not contain "x" becuase we don't need to do anything with a cost of 0;
    }
    if (!cost)
    {
        DebugTrace("Mana cost is NULL.  ");
        return false;
    }
    ManaCost * pMana = NULL;
    if(!gotPayments.size())
    {
        pMana = target ? getPotentialMana(target) : getPotentialMana();
        pMana->add(this->getManaPool());
    }
    if(cost && pMana && !cost->getConvertedCost() && cost->hasX())
    {
        cost = pMana;//{x}:effect, set x to max.
    }
    if(gotPayments.size())
    {
        DebugTrace(" Ai had a payment in mind.");
        ManaCost * paid = NEW ManaCost();
        vector<AIAction*>clicks = vector<AIAction*>();

        paid->init();
        for(int k = 0;k < int(gotPayments.size());k++)
        {
            AManaProducer * amp = dynamic_cast<AManaProducer*> (gotPayments[k]);
            GenericActivatedAbility * gmp = dynamic_cast<GenericActivatedAbility*>(gotPayments[k]);
            if (amp)
            {
                AIAction * action = NEW AIAction(amp,amp->source);
                clicks.push_back(action);
                paid->add(amp->output);
            }
            else if(gmp)
            {
                AIAction * action = NEW AIAction(gmp,gmp->source);
                clicks.push_back(action);
                AForeach * fmp = dynamic_cast<AForeach*>(gmp->ability);
                if(fmp)
                {
                    amp = dynamic_cast<AManaProducer*> (fmp->ability);
                    int outPut = fmp->checkActivation();
                    for(int k = 0;k < outPut;k++)
                        paid->add(amp->output);
                }
            }
            paid->add(this->getManaPool());//incase some of our payments were mana already in the pool/.
            if(paid->canAfford(cost) && !cost->hasX())
            {
                SAFE_DELETE(paid);
                for(int clicking = 0; clicking < int(clicks.size()); clicking++)
                    clickstream.push(clicks[clicking]);
                return true;
            }
            if(cost->hasX() && k == int(gotPayments.size()) && paid->canAfford(cost))
            {
                SAFE_DELETE(paid);
                for(int clicking = 0; clicking < int(clicks.size()); clicking++)
                    clickstream.push(clicks[clicking]);
                return true;
            }
        }
        SAFE_DELETE(paid);
        return false;
    }
    //pMana is our main payment form, it is far faster then direct search.
    DebugTrace(" the Mana was already in the manapool or could be Paid with potential mana, using potential Mana now.");
    if(!pMana->canAfford(cost))
    {
        delete pMana;
        return false;
    }
    ManaCost * diff = pMana->Diff(cost);
    delete (pMana);
    GameObserver * g = GameObserver::GetInstance();

    map<MTGCardInstance *, bool> used;
    for (size_t i = 1; i < g->mLayers->actionLayer()->mObjects.size(); i++)
    { //0 is not a mtgability...hackish
        //Make sure we can use the ability
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->mObjects[i]);
        AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
        if (amp && canHandleCost(amp))
        {
            MTGCardInstance * card = amp->source;
            if (card == target)
            {
                used[card] = true;
            } //http://code.google.com/p/wagic/issues/detail?id=76
            if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() >= 1)
            {
                used[card] = true;
                int doUse = 1;
                for (int i = Constants::MTG_NB_COLORS - 1; i >= 0; i--)
                {
                    if (diff->getCost(i) && amp->output->getCost(i))
                    {
                        diff->remove(i, 1);
                        doUse = 0;
                        break;
                    }
                }
                if (doUse)
                {
                    AIAction * action = NEW AIAction(amp, card);
                    clickstream.push(action);
                }
            }
        }
    }
    delete (diff);
    return true;
}

int AIPlayerBakaB::getEfficiency(OrderedAIAction * action)
{
    return action->getEfficiency();
}

ManaCost * AIPlayerBakaB::getPotentialMana(MTGCardInstance * target)
{
    ManaCost * result = NEW ManaCost();
    GameObserver * g = GameObserver::GetInstance();
    map<MTGCardInstance *, bool> used;
    for (size_t i = 0; i < g->mLayers->actionLayer()->manaObjects.size(); i++)
    { 
        //Make sure we can use the ability
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->manaObjects[i]);
        AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
        GenericActivatedAbility * gmp = dynamic_cast<GenericActivatedAbility*>(a);
        if(gmp && canHandleCost(gmp))
        {
            //skip for each mana producers.
            AForeach * fmp = dynamic_cast<AForeach*>(gmp->ability);
            if(fmp)
            {
                amp = dynamic_cast<AManaProducer*> (fmp->ability);
                if(amp)
                {
                    used[fmp->source] = true;
                    continue;
                }
            }
        }
        if (amp && canHandleCost(amp))
        {
            MTGCardInstance * card = amp->source;
            if (card == target)
                used[card] = true; //http://code.google.com/p/wagic/issues/detail?id=76
            if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() == 1)
            {//ai can't use cards which produce more then 1 converted while using the old pMana method.
                result->add(amp->output);
                used[card] = true;
            }
        }
    }
    return result;
}

vector<MTGAbility*> AIPlayerBakaB::canPayMana(MTGCardInstance * target,ManaCost * cost)
{
    if(!cost || (cost && !cost->getConvertedCost()))
        return vector<MTGAbility*>();
    ManaCost * result = NEW ManaCost();
    GameObserver * g = GameObserver::GetInstance();
    map<MTGCardInstance *, bool> used;
    vector<MTGAbility*>payments = vector<MTGAbility*>();
    if (this->getManaPool()->getConvertedCost())
    {
        //adding the current manapool if any.
        result->add(this->getManaPool());
    }
    int needColorConverted = cost->getConvertedCost()-int(cost->getCost(0)+cost->getCost(7));
    int fullColor = 0;
    for (size_t i = 0; i < g->mLayers->actionLayer()->manaObjects.size(); i++)
    {
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->manaObjects[i]);
        AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
        if(amp && (amp->getCost() && amp->getCost()->extraCosts && !amp->getCost()->extraCosts->canPay()))
            continue;
        if(fullColor == needColorConverted)
        {
            if(cost->hasColor(0) && amp)//find colorless after color mana.
            {
                if(result->canAfford(cost))
                    continue;
                if (canHandleCost(amp))
                {
                    MTGCardInstance * card = amp->source;
                    if (card == target)
                        used[card] = true; //http://code.google.com/p/wagic/issues/detail?id=76
                    if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() >= 1)
                    {
                        if(!(result->canAfford(cost)))//if we got to this point we should be filling colorless mana requirements.
                        {
                            payments.push_back(amp);
                            result->add(amp->output);
                            used[card] = true;
                        }
                    }
                }
            }
            i = g->mLayers->actionLayer()->manaObjects.size();
            break;
        }
        GenericActivatedAbility * gmp = dynamic_cast<GenericActivatedAbility*>(a);
        if(gmp && canHandleCost(gmp))
        {
            //for each mana producers.
            AForeach * fmp = dynamic_cast<AForeach*>(gmp->ability);
            if(fmp)
            {
                amp = dynamic_cast<AManaProducer*> (fmp->ability);
                if(amp)
                {
                    MTGCardInstance * fecard = gmp->source;
                    if (fecard == target)
                        used[fecard] = true; //http://code.google.com/p/wagic/issues/detail?id=76
                    if(gmp->getCost() && gmp->getCost()->getConvertedCost() > 0)
                    {//ai stil can't use cabal coffers and mana abilities which require mana payments effectively;
                        used[fecard];
                        continue;
                    }
                    if (!used[fecard] && gmp->isReactingToClick(fecard) && amp->output->getConvertedCost() >= 1 && (cost->getConvertedCost() > 1 || cost->hasX()))//wasteful to tap a potential big mana source for a single mana.
                    {
                        int outPut = fmp->checkActivation();
                        for(int k = 0;k < outPut;k++)
                            result->add(amp->output);
                        payments.push_back(gmp);
                        used[fecard] = true;
                    }
                }
            }
        }
        else if (amp && canHandleCost(amp) && amp->isReactingToClick(amp->source,amp->getCost()))
        {
            for (int k = Constants::MTG_NB_COLORS-1; k > 0 ; k--)//go backwards.
            {
                if (cost->hasColor(k) && amp->output->hasColor(k) && result->getCost(k) < cost->getCost(k))
                {
                    MTGCardInstance * card = amp->source;
                    if (card == target)
                        used[card] = true; //http://code.google.com/p/wagic/issues/detail?id=76
                    if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() >= 1)
                    {
                        ManaCost * check = NEW ManaCost();
                        check->init();
                        check->add(k,cost->getCost(k));
                        ManaCost * checkResult = NEW ManaCost();
                        checkResult->init();
                        checkResult->add(k,result->getCost(k));
                        if(!(checkResult->canAfford(check)))
                        {
                            payments.push_back(amp);
                            result->add(k,amp->output->getCost(k));
                            used[card] = true;
                            fullColor++;
                        }
                        SAFE_DELETE(check);
                        SAFE_DELETE(checkResult);
                    }
                }
            }
        }
    }
    ManaCostHybrid * hybridCost;
    int hyb;
    hyb = 0;
    hybridCost = cost->getHybridCost(0);
    if(hybridCost)
    {
        while ((hybridCost = cost->getHybridCost(hyb)) != NULL)
        {
            //here we try to find one of the colors in the hybrid cost, it is done 1 at a time unfortunately
            //{rw}{ub} would be 2 runs of this.90% of the time ai finds it's hybrid in pMana check.
            bool foundColor1 = false;
            bool foundColor2 = false;
            for (size_t i = 0; i < g->mLayers->actionLayer()->manaObjects.size(); i++)
            {
                MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->manaObjects[i]);
                AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
                if (amp && canHandleCost(amp))
                {
                    foundColor1 = amp->output->hasColor(hybridCost->color1)?true:false;
                    foundColor2 = amp->output->hasColor(hybridCost->color2)?true:false;
                    if ((foundColor1 && result->getCost(hybridCost->color1) < hybridCost->value1)||
                        (foundColor2 && result->getCost(hybridCost->color2) < hybridCost->value2))
                    {
                        MTGCardInstance * card = amp->source;
                        if (card == target)
                            used[card] = true; //http://code.google.com/p/wagic/issues/detail?id=76
                        if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() >= 1)
                        {
                            ManaCost * check = NEW ManaCost();
                            check->init();
                            check->add(foundColor1?hybridCost->color1:hybridCost->color2,foundColor1?hybridCost->value1:hybridCost->value2);
                            ManaCost * checkResult = NEW ManaCost();
                            checkResult->init();
                            checkResult->add(foundColor1?hybridCost->color1:hybridCost->color2,result->getCost(foundColor1?hybridCost->color1:hybridCost->color2));
                            if(((foundColor1 && !foundColor2)||(!foundColor1 && foundColor2)) &&!(checkResult->canAfford(check)))
                            {
                                payments.push_back(amp);
                                result->add(foundColor1?hybridCost->color1:hybridCost->color2,amp->output->getCost(foundColor1?hybridCost->color1:hybridCost->color2));
                                used[card] = true;
                                fullColor++;
                            }
                            SAFE_DELETE(check);
                            SAFE_DELETE(checkResult);
                        }
                    }
                }
            }
            hyb++;
        }
    }
    else if(!hybridCost && result->getConvertedCost())
    {
        ManaCost * check = NEW ManaCost();
        ManaCost * checkResult = NEW ManaCost();
        check->init();
        checkResult->init();
        for (int k = 1; k < Constants::MTG_NB_COLORS; k++)
        {
            check->add(k,cost->getCost(k));
            checkResult->add(k,result->getCost(k));
            if(!(checkResult->canAfford(check)))
            {
                SAFE_DELETE(check);
                SAFE_DELETE(checkResult);
                SAFE_DELETE(result);
                payments.clear();
                return payments;//we didn't meet one of the color cost requirements.
            }
        }
        SAFE_DELETE(check);
        SAFE_DELETE(checkResult);
    }
    if(cost->hasX())
    {
        //if we decided to play an "x" ability/card, lets go all out, these effects tend to be game winners.
        //add the rest of the mana.
        for (size_t i = 0; i < g->mLayers->actionLayer()->manaObjects.size(); i++)
        { 
            MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->manaObjects[i]);
            AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
            if (amp && canHandleCost(amp))
            {
                if (!used[amp->source] && amp->isReactingToClick(amp->source) && amp->output->getConvertedCost() >= 1)
                {
                    payments.push_back(amp);
                }
            }
        }
    }
    if(!result->canAfford(cost))
        payments.clear();
    SAFE_DELETE(result);
    return payments;
}

vector<MTGAbility*> AIPlayerBakaB::canPaySunBurst(ManaCost * cost)
{
    //in canPaySunburst we try to fill the cost with one of each color we can produce, 
    //note it is still possible to use lotus petal for it's first mana ability and not later for a final color
    //a search of true sunburst would cause the game to come to a crawl, trust me, this is the "fast" method for sunburst :)
    ManaCost * result = NEW ManaCost();
    GameObserver * g = GameObserver::GetInstance();
    map<MTGCardInstance *, bool> used;
    vector<MTGAbility*>payments = vector<MTGAbility*>();
    int needColorConverted = 6;
    int fullColor = 0;
    result->add(this->getManaPool());
    for (size_t i = 0; i < g->mLayers->actionLayer()->manaObjects.size(); i++)
    { 
        //Make sure we can use the ability
        if(fullColor == needColorConverted || fullColor == cost->getConvertedCost())
        {
            i = g->mLayers->actionLayer()->manaObjects.size();
            break;
        }
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->manaObjects[i]);
        AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
        if(amp && amp->getCost() && amp->getCost()->extraCosts && !amp->getCost()->extraCosts->canPay())
            continue;//pentid prism, has no cost but contains a counter cost, without this check ai will think it can still use this mana.
        if (amp && canHandleCost(amp) && amp->isReactingToClick(amp->source,amp->getCost()))
        {
            for (int k = Constants::MTG_NB_COLORS-1; k > 0 ; k--)
            {
                if (amp->output->hasColor(k) && result->getCost(k) < 1 && result->getConvertedCost() < cost->getConvertedCost())
                {
                    MTGCardInstance * card = amp->source;
                    if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() >= 1)
                    {
                        ManaCost * check = NEW ManaCost();
                        check->init();
                        check->add(k,1);
                        ManaCost * checkResult = NEW ManaCost();
                        checkResult->init();
                        checkResult->add(k,result->getCost(k));
                        if(!(checkResult->canAfford(check)))
                        {
                            payments.push_back(amp);
                            result->add(k,amp->output->getCost(k));
                            used[card] = true;
                            fullColor++;
                        }
                        SAFE_DELETE(check);
                        SAFE_DELETE(checkResult);
                    }
                }
            }
        }
    }
    for(int i = fullColor;i < cost->getConvertedCost();i++)
    {
        for (size_t i = 0; i < g->mLayers->actionLayer()->manaObjects.size(); i++)
        { 
            MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->manaObjects[i]);
            AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
            if (amp && canHandleCost(amp))
            {
                MTGCardInstance * card = amp->source;
                if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() >= 1)
                {
                    if(!(result->canAfford(cost)))//if we got to this point we should be filling colorless mana requirements.
                    {
                        payments.push_back(amp);
                        result->add(amp->output);
                        used[card] = true;
                    }
                }
            }
        }
    }
    if(!result->canAfford(cost))
        payments.clear();
    SAFE_DELETE(result);
    return payments;
}


//Can't yet handle extraCost objects (ex: sacrifice) if they require a target :(
int AIPlayerBakaB::CanHandleCost(ManaCost * cost)
{
    if (!cost)
        return 1;

    ExtraCosts * ec = cost->extraCosts;
    if (!ec)
        return 1;

    for (size_t i = 0; i < ec->costs.size(); ++i)
    {
        if (ec->costs[i]->tc)
        {
            return 0;
        }
    }
    return 1;
}

int AIPlayerBakaB::canHandleCost(MTGAbility * ability)
{
    return CanHandleCost(ability->getCost());
}




int AIPlayerBakaB::createAbilityTargets(MTGAbility * a, MTGCardInstance * c, RankingContainer& ranking)
{
    if (!a->getActionTc())
    {
        OrderedAIAction aiAction(a, c, NULL);
        ranking[aiAction] = 1;
        return 1;
    }
    vector<Targetable*>potentialTargets;
    GameObserver * g = GameObserver::GetInstance();
    for (int i = 0; i < 2; i++)
    {
        Player * p = g->players[i];
        MTGGameZone * playerZones[] = { p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay,p->game->stack };
        if(a->getActionTc()->canTarget((Targetable*)p))
        {
            if(a->getActionTc()->maxtargets == 1)
            {
                OrderedAIAction aiAction(a, p, c);
                ranking[aiAction] = 1;
            }
            else
                potentialTargets.push_back(p);
        }
        for (int j = 0; j < 5; j++)
        {
            MTGGameZone * zone = playerZones[j];
            for (int k = 0; k < zone->nb_cards; k++)
            {
                MTGCardInstance * t = zone->cards[k];
                if (a->getActionTc()->canTarget(t))
                {
                    if(a->getActionTc()->maxtargets == 1)
                    {
                        OrderedAIAction aiAction(a, c, t);
                        ranking[aiAction] = 1;
                    }
                    else
                    {
                        potentialTargets.push_back(t);
                    }
                }
            }
        }
    }
    vector<Targetable*>realTargets;
    if(a->getActionTc()->maxtargets != 1)
    {
        if(a->getActionTc()->targets.size() && a->getActionTc()->attemptsToFill > 4)
        {
            a->getActionTc()->done = true;
            return 0;
        }
        int targetThis = 0;
        while(potentialTargets.size())
        {
            OrderedAIAction * check = NULL;

            MTGCardInstance * targeting = dynamic_cast<MTGCardInstance*>(potentialTargets[0]);
            if(targeting && targeting->typeAsTarget() == TARGET_CARD)
             check = NEW OrderedAIAction(a,c,targeting);

            Player * ptargeting = dynamic_cast<Player*>(potentialTargets[0]);
            if(ptargeting && ptargeting->typeAsTarget() == TARGET_PLAYER)
                check = NEW OrderedAIAction(a,ptargeting,c);

            targetThis = getEfficiency(check);
            if(targetThis && ptargeting && ptargeting->typeAsTarget() == TARGET_PLAYER)
            {
                OrderedAIAction aiAction(a,ptargeting,c);
                ranking[aiAction] = 1;
            }
            if(targetThis)
                realTargets.push_back(potentialTargets[0]);
            potentialTargets.erase(potentialTargets.begin());
            SAFE_DELETE(check);
        }
        if(!realTargets.size() || (int(realTargets.size()) < a->getActionTc()->maxtargets && a->getActionTc()->targetMin))
            return 0;
        OrderedAIAction aiAction(a, c,realTargets);
        aiAction.target = (MTGCardInstance*)realTargets[0];
        ranking[aiAction] = 1;
    }
    return 1;
}

int AIPlayerBakaB::selectHintAbility()
{
    if (!hints)
        return 0;

    ManaCost * totalPotentialMana = getPotentialMana(); 
    totalPotentialMana->add(this->getManaPool());
    AIAction * action = hints->suggestAbility(totalPotentialMana);
    if (action && ((WRand() % 100) < 95)) //95% chance
    {
        if (!clickstream.size())
        {
            DebugTrace("AIPlayer:Using Activated ability");
            if (payTheManaCost(action->ability->getCost(), action->click))
            {
                clickstream.push(action);
                SAFE_DELETE(totalPotentialMana);
                return 1;
            }
        }
    }
    SAFE_DELETE(action);
    SAFE_DELETE(totalPotentialMana);
    return 0;
}

int AIPlayerBakaB::selectAbility()
{
    static bool findingAbility = false;
    //this guard is put in place to prevent Ai from
    //ever running selectAbility() function WHILE its already doing so.

    // Break if this happens in debug mode. If this happens, it's actually a bug
    assert(!findingAbility);

    if (findingAbility)
    {//is already looking kick me out of this function!
        return 0;
    }
    findingAbility = true;//im looking now safely!


    // Try Deck hints first
   if (selectHintAbility())
    {
        findingAbility = false;//ok to start looking again.
        return 1;
    }
   GameObserver * go = GameObserver::GetInstance();
   if(go->mLayers->stackLayer()->lastActionController == this)
   {
       //this is here for 2 reasons, MTG rules state that priority is passed with each action.
       //without this ai is able to chain cast {t}:damage:1 target(creature) from everything it can all at once.
       //this not only is illegal but cause ai to waste abilities ei:all damage:1 on a single 1/1 creature.
       findingAbility = false;
       return 1;
   }
    RankingContainer ranking;
    list<int>::iterator it;
    GameObserver * g = GameObserver::GetInstance();
    vector<MTGAbility*>abilityPayment = vector<MTGAbility*>();
    //This loop is extrmely inefficient. TODO: optimize!
    ManaCost * totalPotentialMana = getPotentialMana();
    totalPotentialMana->add(this->getManaPool());
    for (size_t i = 1; i < g->mLayers->actionLayer()->mObjects.size(); i++)
    { //0 is not a mtgability...hackish
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->mObjects[i]);
        //Skip mana abilities for performance
        if (dynamic_cast<AManaProducer*> (a))
            continue;
        //Make sure we can use the ability
        for (int j = 0; j < game->inPlay->nb_cards; j++)
        {
            MTGCardInstance * card = game->inPlay->cards[j];
            if(a->getCost() && !a->isReactingToClick(card, totalPotentialMana))//for preformence reason only look for specific mana if the payment couldnt be made with potential.
            {
                abilityPayment = vector<MTGAbility*>();
                abilityPayment = canPayMana(card,a->getCost());
            }
            if (a->isReactingToClick(card, totalPotentialMana) || abilityPayment.size())
            { //This test is to avoid the huge call to getPotentialManaCost after that
                if(a->getCost() && a->getCost()->hasX() && totalPotentialMana->getConvertedCost() < a->getCost()->getConvertedCost()+1)
                    continue;
                //don't even bother to play an ability with {x} if you can't even afford x=1.
                ManaCost * fullPayment = NULL;
                if (abilityPayment.size())
                {
                    fullPayment = NEW ManaCost();
                    fullPayment->init();
                    for(int ch = 0; ch < int(abilityPayment.size());ch++)
                    {
                        AManaProducer * ampp = dynamic_cast<AManaProducer*> (abilityPayment[ch]);
                        if(ampp)
                            fullPayment->add(ampp->output);
                    }
                    if (fullPayment && a->isReactingToClick(card, fullPayment))
                        createAbilityTargets(a, card, ranking);
                    delete fullPayment;
                }
                else
                {
                    ManaCost * pMana = getPotentialMana(card);
                    pMana->add(this->getManaPool());
                    if (a->isReactingToClick(card, pMana))
                        createAbilityTargets(a, card, ranking);
                    delete (pMana);
                }     
            }
        }
    }
    delete totalPotentialMana;
    if (ranking.size())
    {
        OrderedAIAction action = ranking.begin()->first;
        int chance = 1;
        if (!forceBestAbilityUse)
            chance = 1 + WRand() % 100;
        int actionScore = action.getEfficiency();
        if(action.ability->getCost() && action.ability->getCost()->hasX() && this->game->hand->cards.size())
            actionScore = actionScore/int(this->game->hand->cards.size());//reduce chance for "x" abilities if cards are in hand.
        if (actionScore >= chance)
        {
            if (!clickstream.size())
            {
                if (abilityPayment.size())
                {
                    DebugTrace(" Ai knows exactly what mana to use for this ability.");
                }
                DebugTrace("AIPlayer:Using Activated ability");
                if (payTheManaCost(action.ability->getCost(), action.click,abilityPayment))
                    clickstream.push(NEW AIAction(action));
            }
        }
    }

    findingAbility = false;//ok to start looking again.
    abilityPayment.clear();
    return 1;
}

int AIPlayerBakaB::interruptIfICan()
{
    GameObserver * g = GameObserver::GetInstance();

    if (g->mLayers->stackLayer()->askIfWishesToInterrupt == this)
    {
        if (!clickstream.empty())
            g->mLayers->stackLayer()->cancelInterruptOffer();
        else
            return g->mLayers->stackLayer()->setIsInterrupting(this);
    }
    return 0;
}

int AIPlayerBakaB::effectBadOrGood(MTGCardInstance * card, int mode, TargetChooser * tc)
{
    int id = card->getMTGId();
    AbilityFactory af;
    int autoGuess = af.magicText(id, NULL, card, mode, tc);
    if (autoGuess)
        return autoGuess;
    return BAKA_EFFECT_DONTKNOW;
}

int AIPlayerBakaB::chooseTarget(TargetChooser * _tc, Player * forceTarget,MTGCardInstance * chosenCard,bool checkOnly)
{
    vector<Targetable *> potentialTargets;
    TargetChooser * tc = _tc;
    GameObserver * gameObs = GameObserver::GetInstance();
    if (!(gameObs->currentlyActing() == this))
        return 0;
    if (!tc)
    {
        tc = gameObs->getCurrentTargetChooser();
    }
    if (!tc || !tc->source || tc->maxtargets < 1)
        return 0;
    assert(tc);
    if(!checkOnly && tc->maxtargets > 1)
    {
        tc->initTargets();//just incase....
        potentialTargets.clear();
    }
    //Make sure we own the decision to choose the targets
    assert(tc->Owner == gameObs->currentlyActing());
    if (tc && tc->Owner != gameObs->currentlyActing())
    {
        gameObs->currentActionPlayer = tc->Owner;
		//this is a hack, but if we hit this condition we are locked in a infinate loop
		//so lets give the tc to its owner
		//todo:find the root cause of this.
		DebugTrace("AIPLAYER: Error, was asked to chose targets but I don't own the source of the targetController\n");
		return 0;
	}
    Player * target = forceTarget;
    int playerTargetedZone = 1;
    if (!target)
    {
        target = this;
        int cardEffect = effectBadOrGood(tc->source, MODE_TARGET, tc);
        if(tc->belongsToAbility.size())
        {
            AbilityFactory af;
            MTGAbility * withoutGuessing = af.parseMagicLine(tc->belongsToAbility,NULL,NULL,tc->source);
            cardEffect = af.abilityEfficiency(withoutGuessing,this,MODE_TARGET,tc,NULL);
            delete withoutGuessing;
        }
        if (cardEffect != BAKA_EFFECT_GOOD)
        {
            target = this->opponent();
        }
        if(dynamic_cast<ProliferateChooser*> (tc))
            playerTargetedZone = 2;
    }
    while(playerTargetedZone)
    {
        if (!tc->alreadyHasTarget(target) && tc->canTarget(target) && potentialTargets.size() < 50)
        {
            for (int i = 0; i < 3; i++)
            { //Increase probability to target a player when this is possible
                potentialTargets.push_back(target);
            }
        }
        MTGPlayerCards * playerZones = target->game;
        MTGGameZone * zones[] = { playerZones->hand, playerZones->library, playerZones->inPlay, playerZones->graveyard,playerZones->stack };
        for (int j = 0; j < 5; j++)
        {
            MTGGameZone * zone = zones[j];
            for (int k = 0; k < zone->nb_cards; k++)
            {
                MTGCardInstance * card = zone->cards[k];
                if (!tc->alreadyHasTarget(card) && tc->canTarget(card) && potentialTargets.size() < 50)
                {
                    int multiplier = 1;
                    if (getStats() && getStats()->isInTop(card, 10))
                    {
                        multiplier++;
                        if (getStats()->isInTop(card, 5))
                        {
                            multiplier++;
                            if (getStats()->isInTop(card, 3))
                            {
                                multiplier++;
                            }
                        }
                    }
                    for (int l = 0; l < multiplier; l++)
                    {
                        if(tc->maxtargets != 1 && tc->belongsToAbility.size())
                        {
                            AbilityFactory af;
                            MTGAbility * withoutGuessing = af.parseMagicLine(tc->belongsToAbility,NULL,NULL,tc->source);
                            OrderedAIAction * effCheck = NEW OrderedAIAction(withoutGuessing,(MTGCardInstance*)tc->source,card);
                            if(effCheck->getEfficiency())
                            {
                                potentialTargets.push_back(card);
                            }
                            SAFE_DELETE(effCheck);
                            SAFE_DELETE(withoutGuessing);
                        }
                        else
                        {
                            potentialTargets.push_back(card);
                        }
                    }
                }
            }
        }
        if(playerTargetedZone > 1)
        target = target->opponent();
        playerTargetedZone--;
    }
    if (potentialTargets.size())
    {
        if((!forceTarget && checkOnly)||(tc->maxtargets != 1))
        {
            sort(potentialTargets.begin(), potentialTargets.end());
            potentialTargets.erase(std::unique(potentialTargets.begin(), potentialTargets.end()), potentialTargets.end());
            //checking actual amount of unique targets.
            //multitargeting can not function the same as single target, 
            //a second click on the same target causes it to detoggle target, which can lead to ai lockdowns.
            if(!checkOnly)
                return clickMultiTarget(tc, potentialTargets);
            return int(potentialTargets.size());//return the actual amount of targets ai will atempt to select.
        }
        return clickSingleTarget(tc, potentialTargets, chosenCard);
        //click single target contains nbtargets to keep it as it was previously designed
        //shoving 100 targets into potential, then selecting one of them at random.
    }
    if(checkOnly)return 0;//it wasn't an error if we couldn't find a target while checkonly
    //Couldn't find any valid target,
    //usually that's because we played a card that has bad side effects (ex: when X comes into play, return target land you own to your hand)
    //so we try again to choose a target in the other player's field...
    int cancel = gameObs->cancelCurrentAction();
    if (!cancel && !forceTarget)
        return chooseTarget(tc, target->opponent(), NULL, checkOnly);
    //ERROR!!!
    DebugTrace("AIPLAYER: ERROR! AI needs to choose a target but can't decide!!!");
    return 1;
}

int AIPlayerBakaB::selectMenuOption()
{
    GameObserver * g = GameObserver::GetInstance();
    ActionLayer * object = g->mLayers->actionLayer();
    int doThis = -1;
    if (object->menuObject)
    {
        int checkedLast = 0;
        int checked = 0;
        MenuAbility * currentMenu = NULL;
        if(object->abilitiesMenu->isMultipleChoice && object->currentActionCard)
        {
            for(size_t m = object->mObjects.size()-1;m > 0;m--)
            {
                MenuAbility * ability = dynamic_cast<MenuAbility *>(object->mObjects[m]);
                if(ability && ability->triggered)
                {
                    currentMenu = (MenuAbility *)object->mObjects[m];
                    break;
                }
            }
            if(currentMenu)
                for(unsigned int mk = 0;mk < currentMenu->abilities.size();mk++)
                {
                    MTGAbility * checkEff = NULL;
                    OrderedAIAction * check = NULL;
                    checkEff = currentMenu->abilities[mk];
                    if(checkEff)
                    {
                        if(checkEff->target && checkEff->target->typeAsTarget() == TARGET_CARD)
                            check = NEW OrderedAIAction(checkEff,checkEff->source,(MTGCardInstance*)checkEff->target);
                        else if(checkEff->target && checkEff->target->typeAsTarget() == TARGET_PLAYER)
                            check = NEW OrderedAIAction(checkEff,(Player*)checkEff->target,checkEff->source);
                        else
                            check = NEW OrderedAIAction(checkEff,checkEff->source);
                    }
                    if(check)
                    {
                        checked = getEfficiency(check);
                        SAFE_DELETE(check);
                    }
                    if(checked > 60 && checked > checkedLast)
                    {
                        doThis = mk;
                        checkedLast = checked;
                    }
                    checked = 0;
                }
        }
        else
        {
            for(unsigned int k = 0;k < object->abilitiesMenu->mObjects.size();k++)
            {
                MTGAbility * checkEff = NULL;
                OrderedAIAction * check = NULL;
                if(object->abilitiesMenu->mObjects[k]->GetId() >= 0)
                    checkEff = (MTGAbility *)object->mObjects[object->abilitiesMenu->mObjects[k]->GetId()];
                if(checkEff)
                {
                    Targetable * checkTarget = checkEff->target;
                    if(checkTarget && checkTarget->typeAsTarget() == TARGET_CARD)
                        check = NEW OrderedAIAction(checkEff,checkEff->source,(MTGCardInstance*)checkTarget);
                    else if(checkTarget && checkTarget->typeAsTarget() == TARGET_PLAYER)
                        check = NEW OrderedAIAction(checkEff,(Player*)checkTarget,checkEff->source);
                    else
                        check = NEW OrderedAIAction(checkEff,checkEff->source);
                }
                if(check)
                {
                    checked = getEfficiency(check);
                    SAFE_DELETE(check);
                }
                if(checked > 60 && checked > checkedLast)
                {
                    doThis = k;
                    checkedLast = checked;
                }
                checked = 0;
            }
        }
    }
    return doThis;
}

MTGCardInstance * AIPlayerBakaB::FindCardToPlay(ManaCost * pMana, const char * type)
{
    int maxCost = -1;
    MTGCardInstance * nextCardToPlay = NULL;
    MTGCardInstance * card = NULL;
    CardDescriptor cd;
    cd.init();
    cd.setType(type);
    card = NULL;
    gotPayments = vector<MTGAbility*>();
    while ((card = cd.nextmatch(game->hand, card)))
    {
        if (!CanHandleCost(card->getManaCost()))
            continue;

        if (card->hasType(Subtypes::TYPE_LAND))
        {
            if (game->playRestrictions->canPutIntoZone(card, game->inPlay) == PlayRestriction::CANT_PLAY)
                continue;
        }
        else
        {
            if (game->playRestrictions->canPutIntoZone(card, game->stack) == PlayRestriction::CANT_PLAY)
                continue;
        }

        if (card->hasType(Subtypes::TYPE_LEGENDARY) && game->inPlay->findByName(card->name))
            continue;

        int currentCost = card->getManaCost()->getConvertedCost();
        int hasX = card->getManaCost()->hasX();
        gotPayments.clear();
        if(!pMana->canAfford(card->getManaCost()))
            gotPayments = canPayMana(card,card->getManaCost());
            //for preformence reason we only look for specific mana if the payment couldn't be made with pmana.
        if ((currentCost > maxCost || hasX) && (gotPayments.size() || pMana->canAfford(card->getManaCost())))
        {
            TargetChooserFactory tcf;
            TargetChooser * tc = tcf.createTargetChooser(card);
            int shouldPlayPercentage = 10;
            if (tc)
            {
                int hasTarget = chooseTarget(tc,NULL,NULL,true);
                if(
                    (tc->maxtargets > hasTarget && tc->maxtargets > 1 && !tc->targetMin && tc->maxtargets != TargetChooser::UNLITMITED_TARGETS) ||//target=<3>creature
                    (tc->maxtargets == TargetChooser::UNLITMITED_TARGETS && hasTarget < 1)//target=creatures
                    )
                    hasTarget = 0;
                if (!hasTarget)//single target covered here.
                {
                    SAFE_DELETE(tc);
                    continue;
                }
                shouldPlayPercentage = 90;
                if(tc->targetMin && hasTarget < tc->maxtargets)
                    shouldPlayPercentage = 0;
                if(tc->maxtargets > 1 && tc->maxtargets != TargetChooser::UNLITMITED_TARGETS && hasTarget <= tc->maxtargets)
                {
                    int maxA = hasTarget-tc->maxtargets;
                    shouldPlayPercentage += (10*maxA);//reduce the chances of playing multitarget if we are not above max targets.
                }
                if(tc->maxtargets == TargetChooser::UNLITMITED_TARGETS)
                {
                    shouldPlayPercentage = 40 + (10*hasTarget);
                    int totalCost = pMana->getConvertedCost()-currentCost;
                    int totalTargets = hasTarget+hasTarget;
                    if(hasX &&  totalCost <= totalTargets)// {x} spell with unlimited targeting tend to divide damage, we want atleast 1 damage per target before casting.
                    {
                        shouldPlayPercentage = 0;
                    }
                }
                SAFE_DELETE(tc);
            }
            else
            {
                int shouldPlay = effectBadOrGood(card);
                if (shouldPlay == BAKA_EFFECT_GOOD)
                {
                    shouldPlayPercentage = 90;
                }
                else if (BAKA_EFFECT_DONTKNOW == shouldPlay)
                {
                    shouldPlayPercentage = 80;
                }
            }
            //Reduce the chances of playing a spell with X cost if available mana is low
            if (hasX)
            {
                int xDiff = pMana->getConvertedCost() - currentCost;
                if (xDiff < 0)
                    xDiff = 0;
                shouldPlayPercentage = shouldPlayPercentage - static_cast<int> ((shouldPlayPercentage * 1.9f) / (1 + xDiff));
            }
            if(card->getManaCost() && card->getManaCost()->kicker && card->getManaCost()->kicker->isMulti)
            {

                ManaCost * withKickerCost= NEW ManaCost(card->getManaCost());
                withKickerCost->add(withKickerCost->kicker);
                int canKick = 0;
                while(pMana->canAfford(withKickerCost))
                {
                    withKickerCost->add(withKickerCost->kicker);
                    canKick += 1;
                }
                SAFE_DELETE(withKickerCost);
                shouldPlayPercentage = 10*canKick;
            }
            if (WRand() % 100 > shouldPlayPercentage)
                continue;
            nextCardToPlay = card;
            maxCost = currentCost;
            if (hasX)
                maxCost = pMana->getConvertedCost();
        }
    }
    if(nextCardToPlay)
    {
   DebugTrace(" AI wants to play card." << endl
            << "- Next card to play: " << (nextCardToPlay ? nextCardToPlay->name : "None" ) << endl );
    }
    return nextCardToPlay;
}


void AIPlayerBakaB::initTimer()
{
    if (mFastTimerMode)
        timer = 0;
    else
        timer = 0.1f;
}

int AIPlayerBakaB::computeActions()
{
    /*Zeth fox:TODO:rewrite this entire function, It's a mess.
    I made it far to complicated for what it does and is prone to error and inefficiency.
    Ai run's certain part's when it doesn't need to and run's certain actions when it shouldn't, 
    and it is far to easy to cripple the ai even with what appears to be a minor change to this function;
    reasoning:I split this from 2 to 3 else statements, leaving chooseblockers in the 3rd else,
    the 2nd else is run about 90% of the time over the third, this was causing ai to miss the chance to chooseblockers()
    when it could have blocked almost 90% of the time.*/
    GameObserver * g = GameObserver::GetInstance();
    Player * p = (Player*)this;
    Player * currentP = g->currentlyActing();
    if (!(g->currentlyActing() == p))
        return 0;
    ActionLayer * object = g->mLayers->actionLayer();
    if (object->menuObject)
    {
        int doThis = selectMenuOption();
        if(doThis >= 0)
        {
            if(object->abilitiesMenu->isMultipleChoice)
                g->mLayers->actionLayer()->doMultipleChoice(doThis);
            else
                g->mLayers->actionLayer()->doReactTo(doThis);
        }
        else if(doThis < 0 || object->checkCantCancel())
            g->mLayers->actionLayer()->doReactTo(object->abilitiesMenu->mObjects.size()-1);
        return 1;
    }
    TargetChooser * currentTc = g->getCurrentTargetChooser();
    if(currentTc)
    {
        int targetResult = currentTc->Owner == this? chooseTarget():0;
        if (targetResult)
            return 1;
    }

    static bool findingCard = false;
    //this guard is put in place to prevent Ai from
    //ever running computeActions() function WHILE its already doing so.
    // Break if this happens in debug mode. If this happens, it's actually a bug
    assert(!findingCard);
    if (findingCard)
    {//is already looking kick me out of this function!
        return 0;
    } 
    Interruptible * action = g->mLayers->stackLayer()->getAt(-1);
    Spell * spell = (Spell *) action;
    Player * lastStackActionController = NULL;
    if(spell && spell->type == ACTION_SPELL)
      lastStackActionController = spell->source->controller();           
    if ((interruptIfICan() || g->isInterrupting == this) 
        && this == currentP 
        //and i am the currentlyActivePlayer
        && ((lastStackActionController && lastStackActionController != this) || (g->mLayers->stackLayer()->count(0, NOT_RESOLVED) == 0)))
        //am im not interupting my own spell, or the stack contains nothing.
    {
        bool ipotential = false;
        if(p->game->hand->hasType("instant") || p->game->hand->hasAbility(Constants::FLASH))
        {
            findingCard = true;
            ManaCost * icurrentMana = getPotentialMana();
            icurrentMana->add(this->getManaPool());
            if (icurrentMana->getConvertedCost())
            {
                //if theres mana i can use there then potential is true.
                ipotential = true;
            }
            if (!nextCardToPlay)
            {
                nextCardToPlay = FindCardToPlay(icurrentMana, "instant");
                if (game->playRestrictions->canPutIntoZone(nextCardToPlay, game->stack) == PlayRestriction::CANT_PLAY)
                    nextCardToPlay = NULL;
            }
            SAFE_DELETE (icurrentMana);
        }
        if (!nextCardToPlay)
        {
            selectAbility();
        }
        if (nextCardToPlay)
        {
            if (ipotential)
            {
                if(payTheManaCost(nextCardToPlay->getManaCost(),nextCardToPlay,gotPayments))
                {
                    AIAction * a = NEW AIAction(nextCardToPlay);
                    clickstream.push(a);
                    gotPayments.clear();
                }
            }
            findingCard = false;
            nextCardToPlay = NULL;
            return 1;
        }
        nextCardToPlay = NULL;
        findingCard = false;
        return 1;
    }
    else if(p == this && g->mLayers->stackLayer()->count(0, NOT_RESOLVED) == 0)
    { //standard actions
        switch (g->getCurrentGamePhase())
        {
        case Constants::MTG_PHASE_UPKEEP:
            selectAbility();
            break;
        case Constants::MTG_PHASE_FIRSTMAIN:
        case Constants::MTG_PHASE_SECONDMAIN:
            {
                ManaCost * currentMana = getPotentialMana();
                bool potential = false;
                currentMana->add(this->getManaPool());
                if (currentMana->getConvertedCost())
                {
                    //if theres mana i can use there then potential is true.
                    potential = true;
                }
                nextCardToPlay = FindCardToPlay(currentMana, "land");
                //look for the most expensive creature we can afford. If not found, try enchantment, then artifact, etc...
                const char* types[] = {"creature", "enchantment", "artifact", "sorcery", "instant"};
                int count = 0;
                while (!nextCardToPlay && count < 5)
                {
                    if(clickstream.size()) //don't find cards while we have clicking to do.
                    {
                        SAFE_DELETE(currentMana);
                        return 0;
                    }
                    nextCardToPlay = FindCardToPlay(currentMana, types[count]);
                    if (game->playRestrictions->canPutIntoZone(nextCardToPlay, game->stack) == PlayRestriction::CANT_PLAY)
                        nextCardToPlay = NULL;
                    count++;
                }

                SAFE_DELETE(currentMana);
                if (nextCardToPlay)
                {
                    if(nextCardToPlay->has(Constants::SUNBURST))
                    {
                        //had to force this on Ai other wise it would pay nothing but 1 color for a sunburst card.
                        //this does not teach it to use manaproducer more effectively, it simply allow it to 
                        //use the manaproducers it does understand better on sunburst by force.
                        vector<MTGAbility*>checking = canPaySunBurst(nextCardToPlay->getManaCost());
                        if(payTheManaCost(nextCardToPlay->getManaCost(),NULL,checking))
                        {
                            AIAction * a = NEW AIAction(nextCardToPlay);
                            clickstream.push(a);
                            return 1;
                        }
                        nextCardToPlay = NULL;
                        gotPayments.clear();//if any.
                        return 1;
                    }
                    if(payTheManaCost(nextCardToPlay->getManaCost(),nextCardToPlay,gotPayments))
                    {
                        AIAction * a = NEW AIAction(nextCardToPlay);
                        clickstream.push(a);
                        gotPayments.clear();
                    }
                    return 1;
                }
                else
                {
                    selectAbility();
                }
                break;
            }
        case Constants::MTG_PHASE_COMBATATTACKERS:
            {
                if(g->currentPlayer == this)//only on my turns.
                    chooseAttackers();
                break;
            }
        case Constants::MTG_PHASE_COMBATBLOCKERS:
            {
                if(g->currentPlayer != this)//only on my opponents turns.
                    chooseBlockers();
                break;
            }
        case Constants::MTG_PHASE_ENDOFTURN:
            selectAbility();
            break;
        default:
            break;
        }
    }
    else
    {
        cout << "my turn" << endl;
        switch (g->getCurrentGamePhase())
        {
        case Constants::MTG_PHASE_UPKEEP:
        case Constants::MTG_PHASE_FIRSTMAIN:
        case Constants::MTG_PHASE_COMBATATTACKERS:
        case Constants::MTG_PHASE_COMBATBLOCKERS:
        case Constants::MTG_PHASE_SECONDMAIN:
            {
                selectAbility();
                break;
            }
        default:
            break;
        }
        return 1;
    }
    return 1;
};


//
// Combat //
//

int AIPlayerBakaB::getCreaturesInfo(Player * player, int neededInfo, int untapMode, int canAttack)
{
    int result = 0;
    CardDescriptor cd;
    cd.init();
    cd.setType("Creature");
    cd.unsecureSetTapped(untapMode);
    MTGCardInstance * card = NULL;
    while ((card = cd.nextmatch(player->game->inPlay, card)))
    {
        if (!canAttack || card->canAttack())
        {
            if (neededInfo == INFO_NBCREATURES)
            {
                result++;
            }
            else
            {
                result += card->power;
            }
        }
    }
    return result;
}

int AIPlayerBakaB::chooseAttackers()
{
    //Attack with all creatures
    //How much damage can the other player do during his next Attack ?
    int opponentForce = getCreaturesInfo(opponent(), INFO_CREATURESPOWER);
    int opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES);
    int myForce = getCreaturesInfo(this, INFO_CREATURESPOWER, -1, 1);
    int myCreatures = getCreaturesInfo(this, INFO_NBCREATURES, -1, 1);
    bool attack = ((myCreatures > opponentCreatures) || (myForce > opponentForce) || (myForce > 2 * opponent()->life));
    if (agressivity > 80 && !attack && life > opponentForce)
    {
        opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES, -1);
        opponentForce = getCreaturesInfo(opponent(), INFO_CREATURESPOWER, -1);
        attack = (myCreatures >= opponentCreatures && myForce > opponentForce)
                || (myForce > opponentForce) || (myForce > opponent()->life);
    }
    printf("Choose attackers : %i %i %i %i -> %i\n", opponentForce, opponentCreatures, myForce, myCreatures, attack);
    if (attack)
    {
        CardDescriptor cd;
        cd.init();
        cd.setType("creature");
        MTGCardInstance * card = NULL;
        GameObserver * g = GameObserver::GetInstance();
        MTGAbility * a = g->mLayers->actionLayer()->getAbility(MTGAbility::MTG_ATTACK_RULE);
        while ((card = cd.nextmatch(game->inPlay, card)))
        {
            g->mLayers->actionLayer()->reactToClick(a, card);
        }
    }
    return 1;
}

/* Can I first strike my oponent and get away with murder ? */
int AIPlayerBakaB::canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy)
{
    if (ennemy->has(Constants::FIRSTSTRIKE) || ennemy->has(Constants::DOUBLESTRIKE))
        return 0;
    if (!(card->has(Constants::FIRSTSTRIKE) || card->has(Constants::DOUBLESTRIKE)))
        return 0;
    if (!(card->power >= ennemy->toughness))
        return 0;
    if (!(card->power >= ennemy->toughness + 1) && ennemy->has(Constants::FLANKING))
        return 0;
    return 1;
}

int AIPlayerBakaB::chooseBlockers()
{
    GameObserver * g = GameObserver::GetInstance();

    //Should not block during my own turn...
    if (g->currentPlayer == this)
        return 0;
    map<MTGCardInstance *, int> opponentsToughness;
    int opponentForce = getCreaturesInfo(opponent(), INFO_CREATURESPOWER);

    //Initialize the list of opponent's attacking cards toughness
    CardDescriptor cdAttackers;
    cdAttackers.init();
    cdAttackers.setType("Creature");
    MTGCardInstance * card = NULL;
    while ((card = cdAttackers.nextmatch(opponent()->game->inPlay, card)))
    {
        if (card->isAttacker())
            opponentsToughness[card] = card->toughness;
    }

    //A Descriptor to find untapped creatures in our game
    CardDescriptor cd;
    cd.init();
    cd.setType("Creature");
    cd.unsecureSetTapped(-1);
    card = NULL;
    MTGAbility * a = g->mLayers->actionLayer()->getAbility(MTGAbility::MTG_BLOCK_RULE);

    // We first try to block the major threats, those that are marked in the Top 3 of our stats
    while ((card = cd.nextmatch(game->inPlay, card)))
    {
        g->mLayers->actionLayer()->reactToClick(a, card);
        int set = 0;
        while (!set)
        {
            if (!card->defenser)
            {
                set = 1;
            }
            else
            {
                MTGCardInstance * attacker = card->defenser;
                map<MTGCardInstance *, int>::iterator it = opponentsToughness.find(attacker);
                if (it == opponentsToughness.end())
                {
                    opponentsToughness[attacker] = attacker->toughness;
                    it = opponentsToughness.find(attacker);
                }
                if (opponentsToughness[attacker] > 0 && getStats() && getStats()->isInTop(attacker, 3, false))
                {
                    opponentsToughness[attacker] -= card->power;
                    set = 1;
                }
                else
                {
                    g->mLayers->actionLayer()->reactToClick(a, card);
                }
            }
        }
    }

    //If blocking one of the major threats is not enough to kill it,
    // We change strategy, first we unassign its blockers that where assigned above
    card = NULL;
    while ((card = cd.nextmatch(game->inPlay, card)))
    {
        if (card->defenser && opponentsToughness[card->defenser] > 0)
        {
            while (card->defenser)
            {
                g->mLayers->actionLayer()->reactToClick(a, card);
            }
        }
    }

    //Assign the "free" potential blockers to attacking creatures that are not blocked enough
    card = NULL;
    while ((card = cd.nextmatch(game->inPlay, card)))
    {
        if (!card->defenser)
        {
            g->mLayers->actionLayer()->reactToClick(a, card);
            int set = 0;
            while (!set)
            {
                if (!card->defenser)
                {
                    set = 1;
                }
                else
                {
                    MTGCardInstance * attacker = card->defenser;
                    if (opponentsToughness[attacker] <= 0 || (card->toughness <= attacker->power && opponentForce * 2 < life
                            && !canFirstStrikeKill(card, attacker)) || attacker->nbOpponents() > 1)
                    {
                        g->mLayers->actionLayer()->reactToClick(a, card);
                    }
                    else
                    {
                        set = 1;
                    }
                }
            }
        }
    }
    selectAbility();
    return 1;
}

int AIPlayerBakaB::orderBlockers()
{

    GameObserver * g = GameObserver::GetInstance();
    if (ORDER == g->combatStep && g->currentPlayer == this)
    {
        DebugTrace("AIPLAYER: order blockers");
        g->userRequestNextGamePhase(); //TODO clever rank of blockers
        return 1;
    }

    return 0;
}

int AIPlayerBakaB::affectCombatDamages(CombatStep step)
{
    GameObserver * g = GameObserver::GetInstance();
    GuiCombat * gc = g->mLayers->combatLayer();
    for (vector<AttackerDamaged*>::iterator attacker = gc->attackers.begin(); attacker != gc->attackers.end(); ++attacker)
        gc->autoaffectDamage(*attacker, step);
    return 1;
}

//TODO: Deprecate combatDamages
int AIPlayerBakaB::combatDamages()
{
    int currentGamePhase =  GameObserver::GetInstance()->getCurrentGamePhase();

    if (currentGamePhase == Constants::MTG_PHASE_COMBATBLOCKERS)
        return orderBlockers();

    if (currentGamePhase != Constants::MTG_PHASE_COMBATDAMAGE)
        return 0;

    return 0;

}


//
// General
//

AIStats * AIPlayerBakaB::getStats()
{
    if (!stats)
    {
        char statFile[512];
        sprintf(statFile, "ai/baka/stats/%s.stats", opponent()->deckFileSmall.c_str());
        stats = NEW AIStats(this, statFile);
    }
    return stats;
}


void AIPlayerBakaB::Render()
{
#ifdef RENDER_AI_STATS
    if (getStats()) getStats()->Render();
#endif

}

int AIPlayerBakaB::receiveEvent(WEvent * event)
{
    if (getStats())
        return getStats()->receiveEvent(event);
    return 0;
}


AIPlayerBakaB::AIPlayerBakaB(string file, string fileSmall, string avatarFile, MTGDeck * deck) :
   AIPlayerBaka(file, fileSmall, avatarFile,  deck)
{

}

int AIPlayerBakaB::Act(float dt)
{
    GameObserver * g = GameObserver::GetInstance();
    if (!(g->currentlyActing() == this))
    {
        return 0;
    }

    int currentGamePhase = g->getCurrentGamePhase();

    oldGamePhase = currentGamePhase;

    if (mFastTimerMode)
        timer -= 1;
    else 
        timer -= dt;
    if (timer > 0)
    {
        return 0;
    }
    initTimer();


    if (combatDamages())
    {
        return 0;
    }
    interruptIfICan();

    //computeActions only when i have priority
    if (!(g->currentlyActing() == this))
    {
        DebugTrace("Cannot interrupt");
        return 0;
    }
    if (clickstream.empty())
        computeActions();
    if (clickstream.empty())
    {
        if (g->isInterrupting == this)
        {
            g->mLayers->stackLayer()->cancelInterruptOffer(); //endOfInterruption();
        }
        else
        {
            if (g->currentActionPlayer == this)//if im not the action player why would i requestnextphase?
            g->userRequestNextGamePhase();
        }
    }
    else
    {
        while(clickstream.size())
        {
        AIAction * action = clickstream.front();
        action->Act();
        SAFE_DELETE(action);
        clickstream.pop();
        }
    }
    return 1;
};

AIPlayerBakaB::~AIPlayerBakaB() {
    if (stats)
    {
        stats->save();
        SAFE_DELETE(stats);
    }
    SAFE_DELETE(hints);
}




#endif

