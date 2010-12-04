#include "PrecompiledHeader.h"

#include "AIPlayer.h"
#include "CardDescriptor.h"
#include "AIStats.h"
#include "AllAbilities.h"
#include "ExtraCost.h"
#include "GuiCombat.h"
#include "GameStateDuel.h"

const char * const MTG_LAND_TEXTS[] = { "artifact", "forest", "island", "mountain", "swamp", "plains", "other lands" };

int AIAction::currentId = 0;

int AIAction::Act()
{
    GameObserver * g = GameObserver::GetInstance();
    if (player)
    {
        g->cardClick(NULL, player);
        return 1;
    }
    if (ability)
    {
        g->mLayers->actionLayer()->reactToClick(ability, click);
        if (target) g->cardClick(target);
        return 1;
    }
    else if (click)
    { //Shouldn't be used, really...
        g->cardClick(click, click);
        if (target) g->cardClick(target);
        return 1;
    }
    return 0;
}

AIPlayer::AIPlayer(MTGDeck * deck, string file, string fileSmall) :
    Player(deck, file, fileSmall)
{
    nextCardToPlay = NULL;
    stats = NULL;
    agressivity = 50;
    forceBestAbilityUse = false;
    Checked = false;
    playMode = Player::MODE_AI;
}

AIPlayer::~AIPlayer()
{
    if (stats)
    {
        stats->save();
        SAFE_DELETE(stats);
    }
    while (!clickstream.empty())
    {
        AIAction * action = clickstream.front();
        SAFE_DELETE(action);
        clickstream.pop();
    }
}
MTGCardInstance * AIPlayer::chooseCard(TargetChooser * tc, MTGCardInstance * source, int random)
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

int AIPlayer::Act(float dt)
{
    GameObserver * gameObs = GameObserver::GetInstance();
    if (gameObs->currentPlayer == this) gameObs->userRequestNextGamePhase();
    return 1;
}

void AIPlayer::tapLandsForMana(ManaCost * cost, MTGCardInstance * target)
{
    if (!cost) return;DebugTrace(" AI tapping land for mana");

    ManaCost * pMana = getPotentialMana(target);
    ManaCost * diff = pMana->Diff(cost);
    delete (pMana);
    GameObserver * g = GameObserver::GetInstance();

    map<MTGCardInstance *, bool> used;
    for (int i = 1; i < g->mLayers->actionLayer()->mCount; i++)
    { //0 is not a mtgability...hackish
        //Make sure we can use the ability
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->mObjects[i]);
        AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
        if (amp && canHandleCost(amp))
        {
            MTGCardInstance * card = amp->source;
            if (card == target) used[card] = true; //http://code.google.com/p/wagic/issues/detail?id=76
            if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() == 1)
            {
                used[card] = true;
                int doTap = 1;
                for (int i = Constants::MTG_NB_COLORS - 1; i >= 0; i--)
                {
                    if (diff->getCost(i) && amp->output->getCost(i))
                    {
                        diff->remove(i, 1);
                        doTap = 0;
                        break;
                    }
                }
                if (doTap)
                {
                    AIAction * action = NEW AIAction(amp, card);
                    clickstream.push(action);
                }
            }
        }
    }
    delete (diff);

}

ManaCost * AIPlayer::getPotentialMana(MTGCardInstance * target)
{
    ManaCost * result = NEW ManaCost();
    GameObserver * g = GameObserver::GetInstance();
    map<MTGCardInstance *, bool> used;
    for (int i = 1; i < g->mLayers->actionLayer()->mCount; i++)
    { //0 is not a mtgability...hackish
        //Make sure we can use the ability
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->mObjects[i]);
        AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
        if (amp && canHandleCost(amp))
        {
            MTGCardInstance * card = amp->source;
            if (card == target) used[card] = true; //http://code.google.com/p/wagic/issues/detail?id=76
            if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() == 1)
            {
                result->add(amp->output);
                used[card] = true;
            }
        }
    }
		if(this->getManaPool()->getConvertedCost())
		{
			//adding the current manapool if any, to the potential mana Ai can use.
			result->add(this->getManaPool());
		}
		return result;
}

int AIPlayer::getEfficiency(AIAction * action)
{
    return action->getEfficiency();
}

//Can't yet handle extraCost objects (ex: sacrifice) if they require a target :(
int AIPlayer::CanHandleCost(ManaCost * cost)
{
    if (!cost) return 1;

    ExtraCosts * ec = cost->extraCosts;
    if (!ec) return 1;

    for (size_t i = 0; i < ec->costs.size(); ++i)
    {
        if (ec->costs[i]->tc)
        {
            return 0;
        }
    }
    return 1;
}

int AIPlayer::canHandleCost(MTGAbility * ability)
{
    return CanHandleCost(ability->cost);
}

// In this function, target represents the target of the currentAIAction object, while _target is the target of the ability of this AIAction object
// I can't remember as I type this in which condition we use one or the other for this function, if you find out please replace this comment
int AIAction::getEfficiency()
{
    //TODO add multiplier according to what the player wants
    if (efficiency != -1) return efficiency;
    if (!ability) return 0;
    GameObserver * g = GameObserver::GetInstance();
    ActionStack * s = g->mLayers->stackLayer();
    Player * p = g->currentlyActing();
    if (s->has(ability)) return 0;

    MTGAbility * a = AbilityFactory::getCoreAbility(ability);

    if (!a)
    {
        DebugTrace("FATAL: Ability is NULL in AIAction::getEfficiency()");
        return 0;
    }

    if (!((AIPlayer *) p)->canHandleCost(ability)) return 0;
    switch (a->aType)
    {
    case MTGAbility::DAMAGER:
    {
        AADamager * aad = (AADamager *) a;
        if (!target)
        {
            Targetable * _t = aad->getTarget();
            if (_t == p->opponent())
                efficiency = 90;
            else
                efficiency = 0;
            break;
        }
        if (p == target->controller())
        {
            efficiency = 0;
        }
        else if (aad->damage->getValue() >= target->toughness)
        {
            efficiency = 100;
        }
        else if (target->toughness)
        {
            efficiency = (50 * aad->damage->getValue()) / target->toughness;
        }
        else
        {
            efficiency = 0;
        }
        break;
    }
    case MTGAbility::STANDARD_REGENERATE:
    {
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        efficiency = 0;
        if (!_target) break;

        if (!_target->regenerateTokens && g->getCurrentGamePhase() == Constants::MTG_PHASE_COMBATBLOCKERS && (_target->defenser
                || _target->blockers.size()))
        {
            efficiency = 95;
        }

        //TODO If the card is the target of a damage spell
        break;
    }
    case MTGAbility::STANDARD_PREVENT:
    {
        efficiency = 0;//starts out low to avoid spamming it when its not needed.
        if (!target) break;

        bool NeedPreventing;
        NeedPreventing = false;
        if (g->getCurrentGamePhase() == Constants::MTG_PHASE_COMBATBLOCKERS)
        {
            if ((target->defenser || target->blockers.size()) && target->preventable < target->getNextOpponent()->power) NeedPreventing
                    = true;
        }
        if (p == target->controller() && NeedPreventing == true && !(target->getNextOpponent()->has(Constants::DEATHTOUCH)
                || target->getNextOpponent()->has(Constants::WITHER)))
        {
            efficiency = 20 * (target->DangerRanking());//increase this chance to be used in combat if the creature blocking/blocked could kill the creature this chance is taking into consideration how good the creature is, best creature will always be the first "saved"..
            if (target->toughness == 1 && target->getNextOpponent()->power == 1) efficiency += 15;
            //small bonus added for the poor 1/1s, if we can save them, we will unless something else took precidence.
        }
        //note is the target is being blocked or blocking a creature with wither or deathtouch, it is not even considered for preventing as it is a waste.
        //if its combat blockers, it is being blocked or blocking, and has less prevents the the amount of damage it will be taking, the effeincy is increased slightly and totalled by the danger rank multiplier for final result.
        //TODO If the card is the target of a damage spell
        break;
    }
    case MTGAbility::STANDARD_EQUIP:
    {

        efficiency = 0;
        if (!target) break;

        int equips = p->game->battlefield->countByType("Equipment");
        int myArmy = p->game->battlefield->countByType("Creature");
        int equilized = abs(equips / myArmy);

        if (p == target->controller() && target->equipment <= 1 && !a->source->target)
        {
            efficiency = 20 * (target->DangerRanking());
            if (target->hasColor(5)) efficiency += 20;//this is to encourage Ai to equip white creatures in a weenie deck. ultimately it will depend on what had the higher dangerranking.
            if (target->power == 1 && target->toughness == 1 && target->isToken == 0) efficiency += 10; //small bonus to encourage equipping nontoken 1/1 creatures.
        }

        if (p == target->controller() && !a->source->target && target->equipment < equilized)
        {
            efficiency = 15 * (target->DangerRanking());
            efficiency -= 5 * (target->equipment);
        }
        break;
    }

    case MTGAbility::STANDARD_LEVELUP:
    {
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        efficiency = 0;
        Counter * targetCounter = NULL;
        int currentlevel = 0;

        if (_target)
        {
            if (_target->counters && _target->counters->hasCounter("level", 0, 0))
            {
                targetCounter = _target->counters->hasCounter("level", 0, 0);
                currentlevel = targetCounter->nb;
            }
        }

        if (currentlevel < _target->MaxLevelUp)
        {
            efficiency = 85;
            //increase the efficeincy of leveling up by a small amount equal to current level.
            efficiency += currentlevel;
        }

        if (p->game->hand->nb_cards > 0 && p->isAI())
        {
            efficiency -= (10 * p->game->hand->nb_cards);//reduce the eff if by 10 times the amount of cards in Ais hand.
            //it should always try playing more cards before deciding
        }

        if (g->getCurrentGamePhase() == Constants::MTG_PHASE_SECONDMAIN)
        {
            efficiency = 100;
            //in 2nd main, go all out and try to max stuff.
        }
        break;
    }
    case MTGAbility::STANDARD_PUMP:
    {
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        if (!target) break;
        //i do not set a starting eff. on this ability, this allows Ai to sometimes randomly do it as it normally does.
        if (g->getCurrentGamePhase() == Constants::MTG_PHASE_COMBATBLOCKERS)
        {
            if (BAKA_EFFECT_GOOD)
            {
                if ((_target->defenser || _target->blockers.size()) && ((_target->power < _target->getNextOpponent()->toughness
                        || _target->toughness < _target->getNextOpponent()->power) || (_target->has(Constants::TRAMPLE))))
                {
                    //this pump is based on a start eff. of 20 multiplied by how good the creature is.
                    efficiency = 20 * _target->DangerRanking();
                }
                if (_target->isAttacker() && !_target->blockers.size())
                {
                    //this means im heading directly for the player, pump this creature as much as possible.
                    efficiency = 100;
                }
            }
        }
        break;
    }
    case MTGAbility::STANDARD_BECOMES:
    {
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        //nothing huge here, just ensuring that Ai makes his noncreature becomers into creatures during first main, so it can actually use them in combat.
        if (_target && !_target->hasType("Creature") && g->getCurrentGamePhase() == Constants::MTG_PHASE_FIRSTMAIN)
        {
            efficiency = 100;
        }
        break;
    }

    case MTGAbility::UPCOST:
    {
        //hello, Ai pay your upcost please :P, this entices Ai into paying upcost, the conditional isAi() is required strangely ai is able to pay upcost during YOUR upkeep.
        if (g->getCurrentGamePhase() == Constants::MTG_PHASE_UPKEEP && g->currentPlayer->isAI())
        {
            efficiency = 100;
        }
        break;
    }

    case MTGAbility::FOREACH:
    {
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        MTGAbility * a = AbilityFactory::getCoreAbility(ability);
        AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
        efficiency = 0;
        //trying to encourage Ai to use his foreach manaproducers in first main
        if (a->naType == MTGAbility::MANA_PRODUCER && (g->getCurrentGamePhase() == Constants::MTG_PHASE_FIRSTMAIN
                || g->getCurrentGamePhase() == Constants::MTG_PHASE_SECONDMAIN) && _target->controller()->game->hand->nb_cards > 0)
        {
            for (int i = Constants::MTG_NB_COLORS - 1; i > 0; i--)
            {
                if ((p->game->hand->hasColor(i) || p->game->hand->hasColor(0))
                        && (dynamic_cast<AManaProducer*> ((dynamic_cast<AForeach*> (a)->ability))->output->hasColor(i)))
                {
                    efficiency = 100;
                }
            }

            if (p->game->hand->hasX()) efficiency = 100;

        }
        else
        {
            AbilityFactory af;
            int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY);

            if (target && a->naType != MTGAbility::MANA_PRODUCER && ((suggestion == BAKA_EFFECT_BAD && p == target->controller())
                    || (suggestion == BAKA_EFFECT_GOOD && p != target->controller())))
            {
                efficiency = 0;
            }
            else if (a->naType != MTGAbility::MANA_PRODUCER && (g->getCurrentGamePhase() == Constants::MTG_PHASE_FIRSTMAIN
                    || g->getCurrentGamePhase() == Constants::MTG_PHASE_SECONDMAIN))
            {
                //if its not a manaproducing foreach, and its not targetted, its eff is 90.
                //added this basically to cover the unknown foreachs, or untrained ones which were not targetted effects.
                efficiency = 90;
            }

        }
        break;
    }

    case MTGAbility::STANDARDABILITYGRANT:
    {
        efficiency = 0;
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        if (!target) break;
        //ensuring that Ai grants abilities to creatures during first main, so it can actually use them in combat.
        //quick note: the eff is multiplied by creatures ranking then divided by the number of cards in hand.
        //the reason i do this is to encourage more casting and less waste of mana on abilities.
        AbilityFactory af;
        int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY);

        if ((suggestion == BAKA_EFFECT_BAD && p == target->controller()) || (suggestion == BAKA_EFFECT_GOOD && p
                != target->controller()))
        {
            efficiency = 0;
            //stop giving trample to the players creatures.
        }

        if (suggestion == BAKA_EFFECT_BAD && p != target->controller() && target->has(a->abilitygranted))
        {
            efficiency += (25 * target->DangerRanking()) / p->game->hand->nb_cards;
        }

        if (!target->has(a->abilitygranted) && g->getCurrentGamePhase() == Constants::MTG_PHASE_COMBATBEGIN)
        {
            efficiency += (25 * target->DangerRanking()) / p->game->hand->nb_cards;

        }

        if (target->has(a->abilitygranted))
        {
            //trying to avoid Ai giving ie:flying creatures ie:flying twice.
            efficiency = 0;
        }
        break;
    }

    case MTGAbility::UNTAPPER:
        //untap things that Ai owns and are tapped.
    {
        efficiency = 0;
        if (!target) break;

        if (target->isTapped() && target->controller()->isAI())
        {
            efficiency = (20 * target->DangerRanking());
        }
        break;
    }

    case MTGAbility::TAPPER:
        //tap things the player owns and that are untapped.
    {
        if (!target) break;

        if (!target->controller()->isAI()) efficiency = (20 * target->DangerRanking());

        if (target->isTapped()) efficiency = 0;

        break;
    }

    case MTGAbility::LIFER:
    {
        //use life abilities whenever possible.
        AALifer * alife = (AALifer *) a;
        Targetable * _t = alife->getTarget();

        efficiency = 100;
        AbilityFactory af;
        int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY);

        if ((suggestion == BAKA_EFFECT_BAD && _t == p && p->isAI()) || (suggestion == BAKA_EFFECT_GOOD && _t == p && !p->isAI()))
        {
            efficiency = 0;
        }

        break;
    }
		case MTGAbility::STANDARD_DRAW:
		{
			//adding this case since i played a few games where Ai litterally decided to mill himself to death. fastest and easiest win ever.
			//this should help a little, tho ultimately it will be decided later what the best course of action is.
			efficiency = 0;
			//eff of drawing ability is calculated by base 20 + the amount of cards in library minus the amount of cards in hand times 7.
			//drawing is never going to return a hundred eff because later eff is multiplied by 1.3 if no cards in hand.
			efficiency = int(20 + p->game->library->nb_cards) - int(p->game->hand->nb_cards * 7);
			if(p->game->hand->nb_cards > 8)//reduce by 50 if cards in hand are over 8, high chance ai cant play them.
			{
      efficiency -= 50;
			}
			if(a->nbcardAmount >= p->game->library->nb_cards || p->game->hand->nb_cards > 10)
			{
			//if the amount im drawing will mill me to death or i have more then 10 cards in hand, eff is 0;
      efficiency = 0;
			}
			break;
		}
		case MTGAbility::CLONING:
			{
				efficiency = 0;
				if (p == target->controller())
				{
					efficiency = 20 * target->DangerRanking();
				}
				break;
			}
    case MTGAbility::MANA_PRODUCER:
        efficiency = 0;
        break;

    default:
        if (target)
        {
            AbilityFactory af;
            int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY);
            if ((suggestion == BAKA_EFFECT_BAD && p == target->controller()) || (suggestion == BAKA_EFFECT_GOOD && p
                    != target->controller()))
            {
                efficiency = 0;
            }
            else
            {
                efficiency = WRand() % 5; //Small percentage of chance for unknown abilities
            }
        }
        else
        {
            efficiency = WRand() % 10;
        }
        break;
    }

    if (p->game->hand->nb_cards == 0) efficiency = (int) ((float) efficiency * 1.3); //increase chance of using ability if hand is empty
    if (ability->cost)
    {
        ExtraCosts * ec = ability->cost->extraCosts;
        if (ec) efficiency = efficiency / 3; //Decrease chance of using ability if there is an extra cost to use the ability
    }
    return efficiency;
}

int AIPlayer::createAbilityTargets(MTGAbility * a, MTGCardInstance * c, map<AIAction *, int, CmpAbilities> * ranking)
{
    if (!a->tc)
    {
        AIAction * as = NEW AIAction(a, c, NULL);
        (*ranking)[as] = 1;
        return 1;
    }
    GameObserver * g = GameObserver::GetInstance();
    for (int i = 0; i < 2; i++)
    {
        Player * p = g->players[i];
        MTGGameZone * playerZones[] = { p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay };
        for (int j = 0; j < 4; j++)
        {
            MTGGameZone * zone = playerZones[j];
            for (int k = 0; k < zone->nb_cards; k++)
            {
                MTGCardInstance * t = zone->cards[k];
                if (a->tc->canTarget(t))
                {

                    AIAction * as = NEW AIAction(a, c, t);
                    (*ranking)[as] = 1;
                }
            }
        }
    }
    return 1;
}

int AIPlayer::selectAbility()
{
	  static bool mFindingAbility = false;
		//this gaurd is put in place to prevent Ai from
		//ever running selectAbility() function WHILE its already doing so.
    if (mFindingAbility) 
		{//is already looking kick me out of this function!
			return 0;
		}
    mFindingAbility = true;//im looking now safely!
    map<AIAction *, int, CmpAbilities> ranking;
    list<int>::iterator it;
    GameObserver * g = GameObserver::GetInstance();
    //This loop is extrmely inefficient. TODO: optimize!
    ManaCost * totalPotentialMana = getPotentialMana();
    for (int i = 1; i < g->mLayers->actionLayer()->mCount; i++)
    { //0 is not a mtgability...hackish
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->mObjects[i]);
        //Skip mana abilities for performance
        if (dynamic_cast<AManaProducer*> (a)) continue;
        //Make sure we can use the ability
        for (int j = 0; j < game->inPlay->nb_cards; j++)
        {
            MTGCardInstance * card = game->inPlay->cards[j];
            if (a->isReactingToClick(card, totalPotentialMana))
            { //This test is to avod the huge call to getPotentialManaCost after that
                ManaCost * pMana = getPotentialMana(card);
                if (a->isReactingToClick(card, pMana)) createAbilityTargets(a, card, &ranking);
                delete (pMana);
            }
        }
    }
    delete totalPotentialMana;

    if (ranking.size())
    {
        AIAction * a = ranking.begin()->first;
        int chance = 1;
        if (!forceBestAbilityUse) chance = 1 + WRand() % 100;
        if (getEfficiency(a) < chance)
        {
            a = NULL;
        }
        else
        {
            if (!clickstream.size())
            {
                DebugTrace("AIPlayer:Using Activated ability");
                tapLandsForMana(a->ability->cost, a->click);
                clickstream.push(a);
            }
            else
            {
                a = NULL;
            }
        }
        map<AIAction *, int, CmpAbilities>::iterator it2;
        for (it2 = ranking.begin(); it2 != ranking.end(); it2++)
        {
            if (a && a != it2->first) delete (it2->first);
        }
    }
    mFindingAbility = false;//ok to start looking again.
    return 1;
}

int AIPlayer::interruptIfICan()
{
    GameObserver * g = GameObserver::GetInstance();

    if (g->mLayers->stackLayer()->askIfWishesToInterrupt == this)
    {
        if (!clickstream.empty())
            g->mLayers->stackLayer()->cancelInterruptOffer();
        else
            g->mLayers->stackLayer()->setIsInterrupting(this);
        return 1;
    }
    return 0;
}

int AIPlayer::effectBadOrGood(MTGCardInstance * card, int mode, TargetChooser * tc)
{
    int id = card->getMTGId();
    AbilityFactory af;
    int autoGuess = af.magicText(id, NULL, card, mode, tc);
    if (autoGuess) return autoGuess;
    return BAKA_EFFECT_DONTKNOW;
}

int AIPlayer::chooseTarget(TargetChooser * _tc, Player * forceTarget)
{
    vector<Targetable *> potentialTargets;
    TargetChooser * tc = _tc;
    int nbtargets = 0;
    GameObserver * gameObs = GameObserver::GetInstance();
    int checkOnly = 0;
    if (tc)
    {
        checkOnly = 1;
    }
    else
    {
        tc = gameObs->getCurrentTargetChooser();
    }
    if (!tc) return 0;

    if (!(gameObs->currentlyActing() == this)) return 0;
    Player * target = forceTarget;

    if (!target)
    {
        target = this;
        int cardEffect = effectBadOrGood(tc->source, MODE_TARGET, tc);
        if (cardEffect != BAKA_EFFECT_GOOD)
        {
            target = this->opponent();
        }
    }

    if (!tc->alreadyHasTarget(target) && tc->canTarget(target) && nbtargets < 50)
    {
        for (int i = 0; i < 3; i++)
        { //Increase probability to target a player when this is possible
            potentialTargets.push_back(target);
            nbtargets++;
        }
        if (checkOnly) return 1;
    }
    MTGPlayerCards * playerZones = target->game;
    MTGGameZone * zones[] = { playerZones->hand, playerZones->library, playerZones->inPlay, playerZones->graveyard };
    for (int j = 0; j < 4; j++)
    {
        MTGGameZone * zone = zones[j];
        for (int k = 0; k < zone->nb_cards; k++)
        {
            MTGCardInstance * card = zone->cards[k];
            if (!tc->alreadyHasTarget(card) && tc->canTarget(card) && nbtargets < 50)
            {
                if (checkOnly) return 1;
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
                    potentialTargets.push_back(card);
                    nbtargets++;
                }
            }
        }
    }
    if (nbtargets)
    {
        int i = WRand() % nbtargets;
        int type = potentialTargets[i]->typeAsTarget();
        switch (type)
        {
        case TARGET_CARD:
        {
            MTGCardInstance * card = ((MTGCardInstance *) potentialTargets[i]);
            clickstream.push(NEW AIAction(card));
            return 1;
            break;
        }
        case TARGET_PLAYER:
        {
            Player * player = ((Player *) potentialTargets[i]);
            clickstream.push(NEW AIAction(player));
            return 1;
            break;
        }
        }
    }
    //Couldn't find any valid target,
    //usually that's because we played a card that has bad side effects (ex: when X comes into play, return target land you own to your hand)
    //so we try again to choose a target in the other player's field...
    if (checkOnly) return 0;
    int cancel = gameObs->cancelCurrentAction();
    if (!cancel && !forceTarget) return chooseTarget(_tc, target->opponent());

    //ERROR!!!
    DebugTrace("AIPLAYER: ERROR! AI needs to choose a target but can't decide!!!");
    return 0;
}

int AIPlayer::getCreaturesInfo(Player * player, int neededInfo, int untapMode, int canAttack)
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

int AIPlayer::chooseAttackers()
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
        attack = (myCreatures >= opponentCreatures && myForce > opponentForce) || (myForce > opponentForce) || (myForce
                > opponent()->life);
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
int AIPlayer::canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy)
{
    if (ennemy->has(Constants::FIRSTSTRIKE) || ennemy->has(Constants::DOUBLESTRIKE)) return 0;
    if (!(card->has(Constants::FIRSTSTRIKE) || card->has(Constants::DOUBLESTRIKE))) return 0;
    if (!(card->power >= ennemy->toughness)) return 0;
    if (!(card->power >= ennemy->toughness + 1) && ennemy->has(Constants::FLANKING)) return 0;
    return 1;
}

int AIPlayer::chooseBlockers()
{
    map<MTGCardInstance *, int> opponentsToughness;
    int opponentForce = getCreaturesInfo(opponent(), INFO_CREATURESPOWER);
    //int opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES, -1);
    //int myForce = getCreaturesInfo(this,INFO_CREATURESPOWER);
    //int myCreatures = getCreaturesInfo(this, INFO_NBCREATURES, -1);
    CardDescriptor cd;
    cd.init();
    cd.setType("Creature");
    cd.unsecureSetTapped(-1);
    MTGCardInstance * card = NULL;
    GameObserver * g = GameObserver::GetInstance();
    MTGAbility * a = g->mLayers->actionLayer()->getAbility(MTGAbility::MTG_BLOCK_RULE);

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
											&& !canFirstStrikeKill(card, attacker)) || attacker->nbOpponents() > 1 || attacker->controller()->isAI())
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
    return 1;
}

int AIPlayer::orderBlockers()
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

int AIPlayer::affectCombatDamages(CombatStep step)
{
    GameObserver * g = GameObserver::GetInstance();
    GuiCombat * gc = g->mLayers->combatLayer();
    for (vector<AttackerDamaged*>::iterator attacker = gc->attackers.begin(); attacker != gc->attackers.end(); ++attacker)
        gc->autoaffectDamage(*attacker, step);
    return 1;
}

//TODO: Deprecate combatDamages
int AIPlayer::combatDamages()
{
    //int result = 0;
    GameObserver * gameObs = GameObserver::GetInstance();
    int currentGamePhase = gameObs->getCurrentGamePhase();

    if (currentGamePhase == Constants::MTG_PHASE_COMBATBLOCKERS) return orderBlockers();

    if (currentGamePhase != Constants::MTG_PHASE_COMBATDAMAGE) return 0;

    return 0;

}

AIStats * AIPlayer::getStats()
{
    if (!stats)
    {
        char statFile[512];
        sprintf(statFile, JGE_GET_RES("ai/baka/stats/%s.stats").c_str(), opponent()->deckFileSmall.c_str());
        stats = NEW AIStats(this, statFile);
    }
    return stats;
}

AIPlayer * AIPlayerFactory::createAIPlayer(MTGAllCards * collection, Player * opponent, int deckid)
{
    char deckFile[512];
    char avatarFile[512];
    char deckFileSmall[512];

    if (deckid == GameStateDuel::MENUITEM_EVIL_TWIN)
    { //Evil twin
        sprintf(deckFile, "%s", opponent->deckFile.c_str());
        DebugTrace(opponent->deckFile);
        sprintf(avatarFile, "%s", "baka.jpg");
        sprintf(deckFileSmall, "%s", "ai_baka_eviltwin");
    }
    else
    {
        if (!deckid)
        {
            //random deck
            int nbdecks = 0;
            int found = 1;
            while (found && nbdecks < options[Options::AIDECKS_UNLOCKED].number)
            {
                found = 0;
                char buffer[512];
                sprintf(buffer, JGE_GET_RES("ai/baka/deck%i.txt").c_str(), nbdecks + 1);
                std::ifstream file(buffer);
                if (file)
                {
                    found = 1;
                    file.close();
                    nbdecks++;
                }
            }
            if (!nbdecks) return NULL;
            deckid = 1 + WRand() % (nbdecks);
        }
        sprintf(deckFile, JGE_GET_RES("ai/baka/deck%i.txt").c_str(), deckid);
        sprintf(avatarFile, "avatar%i.jpg", deckid);
        sprintf(deckFileSmall, "ai_baka_deck%i", deckid);
    }

    MTGDeck * tempDeck = NEW MTGDeck(deckFile, collection);
    //MTGPlayerCards * deck = NEW MTGPlayerCards(tempDeck);
    AIPlayerBaka * baka = NEW AIPlayerBaka(tempDeck, deckFile, deckFileSmall, avatarFile);
    baka->deckId = deckid;

    delete tempDeck;
    return baka;
}

MTGCardInstance * AIPlayerBaka::FindCardToPlay(ManaCost * pMana, const char * type)
{
    int maxCost = -1;
    MTGCardInstance * nextCardToPlay = NULL;
    MTGCardInstance * card = NULL;
    CardDescriptor cd;
    cd.init();
    cd.setType(type);
    card = NULL;
    while ((card = cd.nextmatch(game->hand, card)))
    {
        if (!CanHandleCost(card->getManaCost())) continue;
        if (card->hasType(Subtypes::TYPE_CREATURE) && this->castrestrictedcreature > 0 && this->castrestrictedspell > 0) continue;
        if (card->hasType(Subtypes::TYPE_ENCHANTMENT) && this->castrestrictedspell > 0) continue;
        if (card->hasType(Subtypes::TYPE_ARTIFACT) && this->castrestrictedspell > 0) continue;
        if (card->hasType(Subtypes::TYPE_SORCERY) && this->castrestrictedspell > 0) continue;
        if (card->hasType(Subtypes::TYPE_INSTANT) && this->castrestrictedspell > 0) continue;
        if (card->hasType(Subtypes::TYPE_LAND) && !this->canPutLandsIntoPlay) continue;
        if (card->hasType(Subtypes::TYPE_LEGENDARY) && game->inPlay->findByName(card->name)) continue;
        int currentCost = card->getManaCost()->getConvertedCost();
        int hasX = card->getManaCost()->hasX();
        if ((currentCost > maxCost || hasX) && pMana->canAfford(card->getManaCost()))
        {
            TargetChooserFactory tcf;
            TargetChooser * tc = tcf.createTargetChooser(card);
            int shouldPlayPercentage = 10;
            if (tc)
            {
                int hasTarget = (chooseTarget(tc));
                delete tc;
                if (!hasTarget) continue;
                shouldPlayPercentage = 90;
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
                if (xDiff < 0) xDiff = 0;
                shouldPlayPercentage = shouldPlayPercentage - static_cast<int> ((shouldPlayPercentage * 1.9f) / (1 + xDiff));
            }

            if (WRand() % 100 > shouldPlayPercentage) continue;
            nextCardToPlay = card;
            maxCost = currentCost;
            if (hasX) maxCost = pMana->getConvertedCost();
        }
    }
    return nextCardToPlay;
}

AIPlayerBaka::AIPlayerBaka(MTGDeck * deck, string file, string fileSmall, string avatarFile) :
    AIPlayer(deck, file, fileSmall)
{
    mAvatarTex = WResourceManager::Instance()->RetrieveTexture(avatarFile, RETRIEVE_LOCK, TEXTURE_SUB_AVATAR);

    if (!mAvatarTex)
    {
        avatarFile = "baka.jpg";
        mAvatarTex = WResourceManager::Instance()->RetrieveTexture(avatarFile, RETRIEVE_LOCK, TEXTURE_SUB_AVATAR);
    }

    if (mAvatarTex)
        mAvatar = WResourceManager::Instance()->RetrieveQuad(avatarFile, 0, 0, 35, 50, "bakaAvatar", RETRIEVE_NORMAL,
                TEXTURE_SUB_AVATAR);
    else
        mAvatar = NULL;

    initTimer();
}

void AIPlayerBaka::initTimer()
{
    timer = 0.1f;
}

int AIPlayerBaka::computeActions()
{
    GameObserver * g = GameObserver::GetInstance();
    Player * p = g->currentPlayer;
    if (!(g->currentlyActing() == this)) return 0;
    if (g->mLayers->actionLayer()->menuObject)
    {
        g->mLayers->actionLayer()->doReactTo(0);
        return 1;
    }
    if (chooseTarget()) return 1;
    int currentGamePhase = g->getCurrentGamePhase();
    if (g->isInterrupting == this)
    { // interrupting
        selectAbility();
        return 1;
    }
    else if (p == this && g->mLayers->stackLayer()->count(0, NOT_RESOLVED) == 0)
    { //standard actions
        CardDescriptor cd;
        MTGCardInstance * card = NULL;
        switch (currentGamePhase)
        {
        case Constants::MTG_PHASE_FIRSTMAIN:
        case Constants::MTG_PHASE_SECONDMAIN:
        {

            bool potential = false;
            ManaCost * currentMana = getPotentialMana();
            if (currentMana->getConvertedCost())
            {
							//if theres mana i can use there then potential is true.
                potential = true;
            }
            nextCardToPlay = FindCardToPlay(currentMana, "land");
            selectAbility();
            //look for the most expensive creature we can afford
            if (castrestrictedspell == 0 && nospellinstant == 0)
            {
                if (onlyonecast == 0 || castcount < 2)
                {
                    if (onlyoneinstant == 0 || castcount < 2)
                    {
                        if (castrestrictedcreature == 0 && nocreatureinstant == 0)
                        {
                            if (!nextCardToPlay)
                            {
                                nextCardToPlay = FindCardToPlay(currentMana, "creature");
                            }
                        }
                        //Let's Try an enchantment maybe ?
                        if (!nextCardToPlay)
                        {
                            nextCardToPlay = FindCardToPlay(currentMana, "enchantment");
                        }
                        if (!nextCardToPlay)
                        {
                            nextCardToPlay = FindCardToPlay(currentMana, "artifact");
                        }
                        if (!nextCardToPlay)
                        {
                            nextCardToPlay = FindCardToPlay(currentMana, "sorcery");
                        }
                        if (!nextCardToPlay)
                        {
                            nextCardToPlay = FindCardToPlay(currentMana, "instant");
                        }
                        if (!nextCardToPlay)
                        {
                         selectAbility();
                        }
                    }
                }

            }
            if (potential) delete (currentMana);
            if (nextCardToPlay)
            {
                if (potential)
                {
                    /////////////////////////
                    //had to force this on Ai other wise it would pay nothing but 1 color for a sunburst card.
                    //this does not teach it to use manaproducer more effectively, it simply allow it to use the manaproducers it does understand better on sunburst by force.
                    if (nextCardToPlay->has(Constants::SUNBURST))
                    {
                        ManaCost * SunCheck = manaPool;
                        SunCheck = getPotentialMana();
                        for (int i = Constants::MTG_NB_COLORS - 1; i > 0; i--)
                        {
                            //sunburst for Ai
                            if (SunCheck->hasColor(i))
                            {
                                if (nextCardToPlay->getManaCost()->hasColor(i) > 0)
                                {//do nothing if the card already has this color.
                                }
                                else
                                {
                                    if (nextCardToPlay->sunburst < nextCardToPlay->getManaCost()->getConvertedCost())
                                    {
                                        nextCardToPlay->getManaCost()->add(i, 1);
                                        nextCardToPlay->getManaCost()->remove(0, 1);
                                        nextCardToPlay->sunburst += 1;
                                    }
                                }
                            }
                        }
                        delete (SunCheck);
                    }
                    /////////////////////////
                    tapLandsForMana(nextCardToPlay->getManaCost());
                }
                AIAction * a = NEW AIAction(nextCardToPlay);
                clickstream.push(a);
                return 1;
            }
            else
            {
               selectAbility();
            }
            if (p->getManaPool()->getConvertedCost() > 0 && Checked == false)//not the best thing ever, but allows the Ai a chance to double check if its mana pool has something before moving on, atleast one time.
            {
                Checked = true;
                computeActions();
            }
            break;
        }
        case Constants::MTG_PHASE_COMBATATTACKERS:
            chooseAttackers();
            break;
        case Constants::MTG_PHASE_ENDOFTURN:
            Checked = false;
            break;
        default:
            selectAbility();
            break;
        }
    }
    else
    {
        cout << "my turn" << endl;
        switch (currentGamePhase)
        {
        case Constants::MTG_PHASE_COMBATBLOCKERS:
            chooseBlockers();
            break;
        default:
            break;
        }
        return 1;
    }
    return 1;
}
;

int AIPlayer::receiveEvent(WEvent * event)
{
    if (getStats()) return getStats()->receiveEvent(event);
    return 0;
}

void AIPlayer::Render()
{
#ifdef RENDER_AI_STATS
    if (getStats()) getStats()->Render();
#endif
}

int AIPlayerBaka::Act(float dt)
{
    GameObserver * g = GameObserver::GetInstance();

    if (!(g->currentlyActing() == this))
    {
        return 0;
    }

    int currentGamePhase = g->getCurrentGamePhase();

    oldGamePhase = currentGamePhase;

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
    if (!(g->currentlyActing() == this))
    {
        DebugTrace("Cannot interrupt");
        return 0;
    }
    if (clickstream.empty()) computeActions();
    if (clickstream.empty())
    {
        if (g->isInterrupting == this)
        {
            g->mLayers->stackLayer()->cancelInterruptOffer(); //endOfInterruption();
        }
        else
        {
            g->userRequestNextGamePhase();
        }
    }
    else
    {
        AIAction * action = clickstream.front();
        action->Act();
        SAFE_DELETE(action);
        clickstream.pop();
    }
    return 1;
}
;

