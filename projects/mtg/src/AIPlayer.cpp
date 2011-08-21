#include "PrecompiledHeader.h"

#include "AIPlayer.h"
#include "CardDescriptor.h"
#include "CardSelectorSingleton.h"
#include "AIStats.h"
#include "AllAbilities.h"
#include "ExtraCost.h"
#include "GuiCombat.h"
#include "GameStateDuel.h"
#include "DeckManager.h"
#include "AIHints.h"

const char * const MTG_LAND_TEXTS[] = { "artifact", "forest", "island", "mountain", "swamp", "plains", "other lands" };

int AIAction::currentId = 0;

AIAction::AIAction(MTGCardInstance * c, MTGCardInstance * t)
    : efficiency(-1), ability(NULL), player(NULL), click(c), target(t)
{
    id = currentId++;

    // useability tweak - assume that the user is probably going to want to see the full res card,
    // so prefetch it. The idea is that we do it here as we want to start the prefetch before it's time to render,
    // and waiting for it to actually go into play is too late, as we start drawing the card during the interrupt window.
    // This is a good intercept point, as the AI has committed to using this card.

    // if we're not in text mode, always get the thumb
    if (CardSelectorSingleton::Instance()->GetDrawMode() != DrawMode::kText)
    {
        //DebugTrace("Prefetching AI card going into play: " << c->getImageName());
        WResourceManager::Instance()->RetrieveCard(c, RETRIEVE_THUMB);
        
        // also cache the large image if we're using kNormal mode
        if (CardSelectorSingleton::Instance()->GetDrawMode() == DrawMode::kNormal)
        {
            WResourceManager::Instance()->RetrieveCard(c);
        }
    }
}

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
        if (target)
            g->cardClick(target);
        return 1;
    }
    else if (click)
    { //Shouldn't be used, really...
        g->cardClick(click, click);
        if (target)
            g->cardClick(target);
        return 1;
    }
    return 0;
}

AIPlayer::AIPlayer(string file, string fileSmall, MTGDeck * deck) :
    Player(file, fileSmall, deck)
{
    nextCardToPlay = NULL;
    stats = NULL;
    agressivity = 50;
    forceBestAbilityUse = false;
    playMode = Player::MODE_AI;

    //Initialize "AIHints" system
    hints = NULL;
    if (mDeck && mDeck->meta_AIHints.size())
    {
        hints = NEW AIHints(this);
        for (size_t i = 0; i <  mDeck->meta_AIHints.size(); ++i)
            hints->add(mDeck->meta_AIHints[i]);
    }
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
    SAFE_DELETE(hints);
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
    if (gameObs->currentPlayer == this)
        gameObs->userRequestNextGamePhase();
    return 1;
}

bool AIPlayer::tapLandsForMana(ManaCost * cost, MTGCardInstance * target)
{
    DebugTrace(" AI attempting to tap land for mana." << endl
            << "-  Target: " << (target ? target->name : "None" ) << endl
            << "-  Cost: " << (cost ? cost->toString() : "NULL") );
    if(cost && !cost->getConvertedCost())
    {
        DebugTrace("Card has free to play.  ");
        return true;
        //return true becuase we don't need to do anything with a cost of 0;
        //special case for 0 cost, which is valid
    }
    if (!cost)
    {
        DebugTrace("Mana cost is NULL.  ");
        return false;
    }
    ManaCost * pMana = target ? getPotentialMana(target) : getPotentialMana();

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

ManaCost * AIPlayer::getPotentialMana(MTGCardInstance * target)
{
    ManaCost * result = NEW ManaCost();
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
                used[card] = true; //http://code.google.com/p/wagic/issues/detail?id=76
            if (!used[card] && amp->isReactingToClick(card) && amp->output->getConvertedCost() >= 1)
            {
                result->add(amp->output);
                used[card] = true;
            }
        }
    }
    if (this->getManaPool()->getConvertedCost())
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

int AIPlayer::canHandleCost(MTGAbility * ability)
{
    return CanHandleCost(ability->getCost());
}

// In this function, target represents the target of the currentAIAction object, while _target is the target of the ability of this AIAction object
// I can't remember as I type this in which condition we use one or the other for this function, if you find out please replace this comment
int AIAction::getEfficiency()
{
    if (efficiency > -1)
        return efficiency;
    if (!ability)
        return 0;
    GameObserver * g = GameObserver::GetInstance();
    ActionStack * s = g->mLayers->stackLayer();
    Player * p = g->currentlyActing();
    if (s->has(ability))
        return 0;

    MTGAbility * a = AbilityFactory::getCoreAbility(ability);

    if (!a)
    {
        DebugTrace("FATAL: Ability is NULL in AIAction::getEfficiency()");
        return 0;
    }

    if (!((AIPlayer *) p)->canHandleCost(ability))
        return 0;
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
        else if (aad->getDamage() >= target->toughness)
        {
            efficiency = 100;
        }
        else if (target->toughness)
        {
            efficiency = (50 * aad->getDamage()) / target->toughness;
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
        if (!_target)
            break;

        if (!_target->regenerateTokens && g->getCurrentGamePhase() == Constants::MTG_PHASE_COMBATBLOCKERS
                && (_target->defenser || _target->blockers.size())
        )
        {
            efficiency = 95;
        }
        //TODO If the card is the target of a damage spell
        break;
    }
    case MTGAbility::STANDARD_PREVENT:
		{
            efficiency = 0;//starts out low to avoid spamming it when its not needed.
            if (!target && !dynamic_cast<ALord*> (a))
                break;
            if(dynamic_cast<ALord*> (a) && !target)
            {
                //this is a special case for all(this) targetting workaround.
                //adding a direct method for targetting the source is planned for
                //the coming releases, all(this) workaround prevents eff from being returned
                //as its not targetted the same as abilities
                //for now this dirty hack will calculate eff on lords as tho the source is
                //the target...otherwise these abilities will never be used.
                target = a->source;
            }

            bool NeedPreventing;
            NeedPreventing = false;
            if (g->getCurrentGamePhase() == Constants::MTG_PHASE_COMBATBLOCKERS)
            {
                if(!target->getNextOpponent()->typeAsTarget() == TARGET_CARD)
                    break;
                if ((target->defenser || target->blockers.size()) && target->preventable < target->getNextOpponent()->power)
                    NeedPreventing = true;
                if (p == target->controller() && target->controller()->isAI() && NeedPreventing  && !(target->getNextOpponent()->has(Constants::DEATHTOUCH)
                    || target->getNextOpponent()->has(Constants::WITHER)))
                {
                    efficiency = 20 * (target->DangerRanking());//increase this chance to be used in combat if the creature blocking/blocked could kill the creature this chance is taking into consideration how good the creature is, best creature will always be the first "saved"..
                    if (target->toughness == 1 && target->getNextOpponent()->power == 1)
                        efficiency += 15;
                    //small bonus added for the poor 1/1s, if we can save them, we will unless something else took precidence.
                    //note is the target is being blocked or blocking a creature with wither or deathtouch, it is not even considered for preventing as it is a waste.
                    //if its combat blockers, it is being blocked or blocking, and has less prevents the the amount of damage it will be taking, the effeincy is increased slightly and totalled by the danger rank multiplier for final result.
                    int calculateAfterDamage = 0;
                    int damages = 0;
                    if((target->defenser || target->blockers.size()) && target->controller() == p)
                    {
                        damages = target->getNextOpponent()->power;
                        calculateAfterDamage = int(target->toughness - damages);
                        if((calculateAfterDamage + target->preventable) > 0)
                        {
                            efficiency = 0;
                            //this is to avoid wasting prevents on creatures that will already survive.
                            //this should take into account bushido and flanking as this check is run after every trigger.
                        }
                    }
                }
            }
            //TODO If the card is the target of a damage spell
            break;
        }
    case MTGAbility::STANDARD_EQUIP:
        {

            efficiency = 0;
            if (!target)
                break;

            int equips = p->game->battlefield->countByType("Equipment");
            int myArmy = p->game->battlefield->countByType("Creature");
            // when can this ever be negative?
            int equilized = myArmy ? abs(equips / myArmy) : 0;

            if (p == target->controller() && target->equipment <= 1 && !a->source->target)
            {
                efficiency = 20 * (target->DangerRanking());
                if (target->hasColor(Constants::MTG_COLOR_WHITE))
                    efficiency += 20;//this is to encourage Ai to equip white creatures in a weenie deck. ultimately it will depend on what had the higher dangerranking.
                if (target->power == 1 && target->toughness == 1 && target->isToken == 0)
                    efficiency += 10; //small bonus to encourage equipping nontoken 1/1 creatures.
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
                if (currentlevel < _target->MaxLevelUp)
                {
                    efficiency = 85;
                    //increase the efficeincy of leveling up by a small amount equal to current level.
                    efficiency += currentlevel;

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
                }
            }
            break;
        }
    case MTGAbility::COUNTERS:
        {
            MTGCardInstance * _target = (MTGCardInstance *) target;
            if(!_target)
            _target = (MTGCardInstance *) (a->target);
            efficiency = 0;
            if(AACounter * cc = dynamic_cast<AACounter*> (a))
            {
                if(cc && _target)
                {
                    if(_target->controller() == p && cc->toughness>=0)
                    {
                        efficiency = 90;

                    }
                    if(_target->controller() != p && ((_target->toughness + cc->toughness <= 0 && _target->toughness) || (cc->toughness < 0 && cc->power < 0)))
                    {
                        efficiency = 90;

                    }
                    if(_target->counters && _target->counters->hasCounter(cc->power,cc->toughness) && _target->counters->hasCounter(cc->power,cc->toughness)->nb > 15)
                    {
                        efficiency = _target->counters->hasCounter(cc->power,cc->toughness)->nb;
                    }
                    if(cc->maxNb && _target->counters && _target->counters->hasCounter(cc->power,cc->toughness)->nb >= cc->maxNb) 
                        efficiency = 0;
                }
            }
            break;
        }
    case MTGAbility::STANDARD_PUMP:
        {
            MTGCardInstance * _target = (MTGCardInstance *) (a->target);
            efficiency = 0;
            if(!_target)
                break;
            if(!target && !dynamic_cast<ALord*> (a) && (((MTGCardInstance *)a->source)->hasSubtype(Subtypes::TYPE_AURA) || ((MTGCardInstance *)a->source)->hasSubtype(Subtypes::TYPE_EQUIPMENT)))
            {
                if(((MTGCardInstance *)a->source)->target)
                    _target = ((MTGCardInstance *)a->source)->target;
                target = (MTGCardInstance *)a->source;
            }
            if (!target && !dynamic_cast<ALord*> (a))
                break;
            if(dynamic_cast<ALord*> (a) && !target)
            {
                target = a->source;
            }
				AbilityFactory af;
        int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY);
        //i do not set a starting eff. on this ability, this allows Ai to sometimes randomly do it as it normally does.
        int currentPhase = g->getCurrentGamePhase();
        if ((currentPhase == Constants::MTG_PHASE_COMBATBLOCKERS) || (currentPhase == Constants::MTG_PHASE_COMBATATTACKERS))
        {
            if (suggestion == BAKA_EFFECT_GOOD && target->controller() == p)
            {
                if(_target->defenser || _target->blockers.size())
                {
                    if(!_target->getNextOpponent()->typeAsTarget() == TARGET_CARD)
                        break;
                    if (_target->power < _target->getNextOpponent()->toughness ||(_target->getNextOpponent() && _target->toughness < _target->getNextOpponent()->power) || (_target->has(Constants::TRAMPLE)))
                    {
                        //this pump is based on a start eff. of 20 multiplied by how good the creature is.
                        efficiency = 20 * _target->DangerRanking();
                    }
                }
                if (_target->isAttacker() && !_target->blockers.size())
                {
                    //this means im heading directly for the player, pump this creature as much as possible.
                    efficiency = 100;
                    if(_target->power > 50)
                        efficiency -= _target->power;//we don't need to go overboard. better to not put all your eggs in a single basket.
                }
            }
        }
        if (suggestion == BAKA_EFFECT_BAD && target->controller() != p && target->toughness > 0)
        {
            efficiency = 100;
        }
        break;
    }
    case MTGAbility::STANDARD_BECOMES:
    {
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        //nothing huge here, just ensuring that Ai makes his noncreature becomers into creatures during first main, so it can actually use them in combat.
        if (_target && !_target->isCreature() && g->getCurrentGamePhase() == Constants::MTG_PHASE_FIRSTMAIN)
        {
            efficiency = 100;
        }
        break;
    }

    case MTGAbility::UPCOST:
    {
        //hello, Ai pay your upcost please :P, this entices Ai into paying upcost, the conditional isAi() is required strangely ai is able to pay upcost during YOUR upkeep.
        if (g->getCurrentGamePhase() == Constants::MTG_PHASE_UPKEEP && g->currentPlayer == p && p == a->source->controller())
        {
            efficiency = 100;
        }
        break;
    }

    case MTGAbility::FOREACH:
    case MTGAbility::MANA_PRODUCER://only way to hit this condition is nested manaabilities, ai skips manaproducers by defualt when finding an ability to use.
    {
        MTGCardInstance * _target = (MTGCardInstance *) (a->target);
        MTGAbility * a = AbilityFactory::getCoreAbility(ability);

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

            if (p->game->hand->hasX())
                efficiency = 100;

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
            MTGCardInstance * _target = (MTGCardInstance*)(a->target);
            if(!_target)
                break;
        if (!target && !dynamic_cast<ALord*> (a))
            break;
            if(dynamic_cast<ALord*> (a) && !target)
            {
             target = a->source;
            }
            
        //ensuring that Ai grants abilities to creatures during first main, so it can actually use them in combat.
        //quick note: the eff is multiplied by creatures ranking then divided by the number of cards in hand.
        //the reason i do this is to encourage more casting and less waste of mana on abilities.
        AbilityFactory af;
        int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY);

        int efficiencyModifier = (25 * target->DangerRanking());
        if (p->game->hand->nb_cards > 1)
        {
            efficiencyModifier -= p->game->hand->nb_cards*3;
        }
        if (suggestion == BAKA_EFFECT_BAD && p != target->controller() && !target->has(a->abilitygranted))
        {
            efficiency += efficiencyModifier;
        }

        if (!target->has(a->abilitygranted) && g->getCurrentGamePhase() == Constants::MTG_PHASE_COMBATBEGIN
                && p == target->controller()
        )
        {
            efficiency += efficiencyModifier;
        }

        if (suggestion == BAKA_EFFECT_GOOD && target->has(a->abilitygranted))
        {
            //trying to avoid Ai giving ie:flying creatures ie:flying twice.
            efficiency = 0;
        }

        if ((suggestion == BAKA_EFFECT_BAD && p == target->controller()) 
                || (suggestion == BAKA_EFFECT_GOOD && p != target->controller())
        )
        {
            efficiency = 0;
            //stop giving trample to the players creatures.
        }
        break;
    }

    case MTGAbility::UNTAPPER:
        //untap things that Ai owns and are tapped.
    {
        efficiency = 0;
        if (!target && !dynamic_cast<ALord*> (a))
            break;
            if(dynamic_cast<ALord*> (a) && !target)
            {
             target = a->source;
            }

            if (target->isTapped() && target->controller() == p &&!target->isCreature())
            {
                efficiency = 100;
            }
            if (target->isTapped() && target->controller() == p && target->isCreature())
            {
                efficiency = (20 * target->DangerRanking());
            }
        break;
    }

    case MTGAbility::TAPPER:
        //tap things the player owns and that are untapped.
    {
        if (!target && !dynamic_cast<ALord*> (a))
            break;
            if(dynamic_cast<ALord*> (a) && !target)
            {
             target = a->source;
            }

        if (target->controller() != p)
            efficiency = (20 * target->DangerRanking());

        if (target->isTapped())
            efficiency = 0;

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

        if ((suggestion == BAKA_EFFECT_BAD && _t == p) || (suggestion == BAKA_EFFECT_GOOD && _t == p))
        {
            efficiency = 0;
        }

        break;
    }
    case MTGAbility::STANDARD_DRAW:
    {
        AADrawer * drawer = (AADrawer *)a;
        //adding this case since i played a few games where Ai litterally decided to mill himself to death. fastest and easiest win ever.
        //this should help a little, tho ultimately it will be decided later what the best course of action is.
        //eff of drawing ability is calculated by base 20 + the amount of cards in library minus the amount of cards in hand times 7.
        //drawing is never going to return a hundred eff because later eff is multiplied by 1.3 if no cards in hand.
        efficiency = int(20 + p->game->library->nb_cards) - int(p->game->hand->nb_cards * 7);
        if (p->game->hand->nb_cards > 8)//reduce by 50 if cards in hand are over 8, high chance ai cant play them.
        {
            efficiency -= 70;
        }
        if ((drawer->getNumCards() >= p->game->library->nb_cards && (Targetable*)p == drawer->getTarget()) || (p->game->hand->nb_cards > 10 && (Targetable*)p == drawer->getTarget()))
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
	case MTGAbility::STANDARD_FIZZLER:
		{
			Interruptible * action = g->mLayers->stackLayer()->getAt(-1);
			Spell * spell = (Spell *) action;
			Player * lastStackActionController = NULL;
			if(spell && spell->type == ACTION_SPELL)
				lastStackActionController = spell->source->controller();   
			if(p != target->controller() && lastStackActionController && lastStackActionController != p)
				efficiency = 60;//we want ai to fizzle at higher than "unknown" ability %.
			break;
		}
    default:
        if (target)
        {
            AbilityFactory af;
            int suggestion = af.abilityEfficiency(a, p, MODE_ABILITY,NULL,target);
            if ((suggestion == BAKA_EFFECT_BAD && p == target->controller())
                    || (suggestion == BAKA_EFFECT_GOOD && p != target->controller()))
            {
                efficiency = 0;
            }
            else
            {
            //without a base to start with Wrand % 5 almost always returns 0.
                efficiency = 10 + WRand() % 5; //Small percentage of chance for unknown abilities
            }
        }
        else
        {
            efficiency = 10 + WRand() % 10;
        }
        break;
    }

    if (p->game->hand->nb_cards == 0)
        efficiency = (int) ((float) efficiency * 1.3); //increase chance of using ability if hand is empty
    ManaCost * cost = ability->getCost();
    if (cost)
    {
        ExtraCosts * ec = cost->extraCosts;
        if (ec)
        {
            for(unsigned int i = 0; i < ec->costs.size();i++)
            {
                ExtraCost * tapper = dynamic_cast<TapCost*>(ec->costs[i]);
                if(tapper)
                    continue;
                else
                    efficiency = efficiency / 2;
            }
            //Decrease chance of using ability if there is an extra cost to use the ability, ignore tap
        }
    }
    return efficiency;
}

int AIPlayer::createAbilityTargets(MTGAbility * a, MTGCardInstance * c, RankingContainer& ranking)
{
    if (!a->tc)
    {
        AIAction aiAction(a, c, NULL);
        ranking[aiAction] = 1;
        return 1;
    }
    GameObserver * g = GameObserver::GetInstance();
    for (int i = 0; i < 2; i++)
    {
        Player * p = g->players[i];
		MTGGameZone * playerZones[] = { p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay,p->game->stack };
        for (int j = 0; j < 5; j++)
        {
            MTGGameZone * zone = playerZones[j];
            for (int k = 0; k < zone->nb_cards; k++)
            {
                MTGCardInstance * t = zone->cards[k];
                if (a->tc->canTarget(t))
                {

                    AIAction aiAction(a, c, t);
                    ranking[aiAction] = 1;
                }
            }
        }
    }
    return 1;
}

int AIPlayer::selectHintAbility()
{
    if (!hints)
        return 0;

    ManaCost * totalPotentialMana = getPotentialMana(); 

    AIAction * action = hints->suggestAbility(totalPotentialMana);
    if (action && ((WRand() % 100) < 95)) //95% chance
    {
        if (!clickstream.size())
        {
            DebugTrace("AIPlayer:Using Activated ability");
            if (tapLandsForMana(action->ability->getCost(), action->click))
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

int AIPlayer::selectAbility()
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

    RankingContainer ranking;
    list<int>::iterator it;
    GameObserver * g = GameObserver::GetInstance();
    //This loop is extrmely inefficient. TODO: optimize!
    ManaCost * totalPotentialMana = getPotentialMana();
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
            if (a->isReactingToClick(card, totalPotentialMana))
            { //This test is to avoid the huge call to getPotentialManaCost after that
                ManaCost * pMana = getPotentialMana(card);
                if (a->isReactingToClick(card, pMana))
                    createAbilityTargets(a, card, ranking);
                delete (pMana);
            }
        }
    }
    delete totalPotentialMana;

    if (ranking.size())
    {
        AIAction action = ranking.begin()->first;
        int chance = 1;
        if (!forceBestAbilityUse)
            chance = 1 + WRand() % 100;
        int actionScore = action.getEfficiency();
        if (actionScore >= chance)
        {
            if (!clickstream.size())
            {
                DebugTrace("AIPlayer:Using Activated ability");
                if (tapLandsForMana(action.ability->getCost(), action.click))
                    clickstream.push(NEW AIAction(action));
            }
        }
    }

    findingAbility = false;//ok to start looking again.
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
            return g->mLayers->stackLayer()->setIsInterrupting(this);
    }
    return 0;
}

int AIPlayer::effectBadOrGood(MTGCardInstance * card, int mode, TargetChooser * tc)
{
    int id = card->getMTGId();
    AbilityFactory af;
    int autoGuess = af.magicText(id, NULL, card, mode, tc);
    if (autoGuess)
        return autoGuess;
    return BAKA_EFFECT_DONTKNOW;
}

int AIPlayer::chooseTarget(TargetChooser * _tc, Player * forceTarget,MTGCardInstance * Choosencard)
{
    vector<Targetable *> potentialTargets;
    TargetChooser * tc = _tc;
    int nbtargets = 0;
    GameObserver * gameObs = GameObserver::GetInstance();
    int checkOnly = 0;
    if (tc)
    {
        if(!Choosencard)
            checkOnly = 1;
    }
    else
    {
        tc = gameObs->getCurrentTargetChooser();
    }
    if (!tc)
        return 0;

	//Make sure we own the decision to choose the targets
	assert(tc->source->controller() == this);
	if (tc->source->controller() != this)
	{
		gameObs->currentActionPlayer = tc->source->controller();
		//this is a hack, but if we hit this condition we are locked in a infinate loop
		//so lets give the tc to its owner
		//todo:find the root cause of this.
		DebugTrace("AIPLAYER: Error, was asked to chose targets but I don't own the source of the targetController\n");
		return 0;
	}

    tc->initTargets(); //cleanup the targetchooser just in case.
    if (!(gameObs->currentlyActing() == this))
        return 0;
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
        if (checkOnly)
            return 1;
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
                if (checkOnly)
                    return 1;
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
        //targetting the stack
        zone = playerZones->stack;
        for (int k = 0; k < zone->nb_cards; k++)
        {
         MTGCardInstance* card = zone->cards[k];
            if (!tc->alreadyHasTarget(card) && tc->canTarget(card) && nbtargets < 50)
            {
                if (checkOnly)
                    return 1;
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
                if(!Choosencard)
                {
                    MTGCardInstance * card = ((MTGCardInstance *) potentialTargets[i]);
                    clickstream.push(NEW AIAction(card));
                    Choosencard = card;
                }
                //can't be 100% positive that this wont have an adverse side-effect
                //hoping this fills a edge case where ai will keep trying to choose a target for a card which it already has a target for.
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
    if (checkOnly)
    {
        return 0;
    }
    int cancel = gameObs->cancelCurrentAction();
    if (!cancel && !forceTarget)
        return chooseTarget(_tc, target->opponent());

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
int AIPlayer::canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy)
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

int AIPlayer::chooseBlockers()
{
    GameObserver * g = GameObserver::GetInstance();

    //Should not block during my own turn...
    if (g->currentPlayer == this)
        return 0;
    //Should not run this if I'm not the player with priority
    if (g->currentActionPlayer != this)
        return 0;
    //I am interrupting, why would I be choosing blockers now?
    if(g->isInterrupting == this)
        return 0;
    //ai should not be allowed to run this if it is not legally allowed to do so
    //this fixes a bug where ai would try to use this as an interupt
    //when ai is given priority to select blockers it is allowed to run this as normal.
    //but as soon as its selected it blockers and proirity switch back to other player
    //kick the ai out of this function.
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
    int currentGamePhase =  GameObserver::GetInstance()->getCurrentGamePhase();

    if (currentGamePhase == Constants::MTG_PHASE_COMBATBLOCKERS)
        return orderBlockers();

    if (currentGamePhase != Constants::MTG_PHASE_COMBATDAMAGE)
        return 0;

    return 0;

}

AIStats * AIPlayer::getStats()
{
    if (!stats)
    {
        char statFile[512];
        sprintf(statFile, "ai/baka/stats/%s.stats", opponent()->deckFileSmall.c_str());
        stats = NEW AIStats(this, statFile);
    }
    return stats;
}

AIPlayer * AIPlayerFactory::createAIPlayer(MTGAllCards * collection, Player * opponent, int deckid)
{
    char deckFile[512];
    string avatarFilename; // default imagename
    char deckFileSmall[512];
    
    if (deckid == GameStateDuel::MENUITEM_EVIL_TWIN)
    { //Evil twin
        sprintf(deckFile, "%s", opponent->deckFile.c_str());
        DebugTrace("Evil Twin => " << opponent->deckFile);
        avatarFilename = "avatar.jpg";
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
                sprintf(buffer, "ai/baka/deck%i.txt", nbdecks + 1);
                if (FileExists(buffer))
                {
                    found = 1;
                    nbdecks++;
                }
            }
            if (!nbdecks)
                return NULL;
            deckid = 1 + WRand() % (nbdecks);
        }
        sprintf(deckFile, "ai/baka/deck%i.txt", deckid);
        DeckMetaData *aiMeta = DeckManager::GetInstance()->getDeckMetaDataByFilename( deckFile, true);
        avatarFilename = aiMeta->getAvatarFilename();
        sprintf(deckFileSmall, "ai_baka_deck%i", deckid);
    }

    int deckSetting = EASY;
    if ( opponent ) 
    {
        bool isOpponentAI = opponent->isAI() == 1;
        DeckMetaData *meta = DeckManager::GetInstance()->getDeckMetaDataByFilename( opponent->deckFile, isOpponentAI);
        if ( meta->getVictoryPercentage() >= 65)
            deckSetting = HARD;
    }
    
    // AIPlayerBaka will delete MTGDeck when it's time
    AIPlayerBaka * baka = NEW AIPlayerBaka(deckFile, deckFileSmall, avatarFilename, NEW MTGDeck(deckFile, collection,0, deckSetting));
    baka->deckId = deckid;
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
        if ((currentCost > maxCost || hasX) && pMana->canAfford(card->getManaCost()))
        {
            TargetChooserFactory tcf;
            TargetChooser * tc = tcf.createTargetChooser(card);
            int shouldPlayPercentage = 10;
            if (tc)
            {
                int hasTarget = (chooseTarget(tc));
                if(tc)
                delete tc;
                if (!hasTarget)
                    continue;
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
                if (xDiff < 0)
                    xDiff = 0;
                shouldPlayPercentage = shouldPlayPercentage - static_cast<int> ((shouldPlayPercentage * 1.9f) / (1 + xDiff));
            }

            if (WRand() % 100 > shouldPlayPercentage)
                continue;
            nextCardToPlay = card;
            maxCost = currentCost;
            if (hasX)
                maxCost = pMana->getConvertedCost();
        }
    }
    return nextCardToPlay;
}

AIPlayerBaka::AIPlayerBaka(string file, string fileSmall, string avatarFile, MTGDeck * deck) :
    AIPlayer(file, fileSmall, deck)
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

    if (fileSmall == "ai_baka_eviltwin")
        mAvatar->SetHFlip(true);
    
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
    Player * currentP = g->currentlyActing();
    if (!(g->currentlyActing() == this))
        return 0;
    if (g->mLayers->actionLayer()->menuObject)
    {
        g->mLayers->actionLayer()->doReactTo(0);
        return 1;
    }
    if (chooseTarget())
        return 1;
    int currentGamePhase = g->getCurrentGamePhase();
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
        //i can interupt or am interupting
        && p != this 
        //and its not my turn
        && this == currentP 
        //and i am the currentlyActivePlayer
        && ((lastStackActionController && lastStackActionController != this) || (g->mLayers->stackLayer()->count(0, NOT_RESOLVED) == 0)))
        //am im not interupting my own spell, or the stack contains nothing.
    {
        findingCard = true;
        ManaCost * icurrentMana = getPotentialMana();
        bool ipotential = false;
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

        if (!nextCardToPlay)
        {
            selectAbility();
        }



        if (nextCardToPlay)
        {
            if (ipotential)
            {
                if(tapLandsForMana(nextCardToPlay->getManaCost(),nextCardToPlay))
                {
                    AIAction * a = NEW AIAction(nextCardToPlay);
                    clickstream.push(a);
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
        switch (currentGamePhase)
        {
        case Constants::MTG_PHASE_FIRSTMAIN:
        case Constants::MTG_PHASE_SECONDMAIN:
            {
                ManaCost * currentMana = getPotentialMana();
                bool potential = false;
                if (currentMana->getConvertedCost())
                {
                    //if theres mana i can use there then potential is true.
                    potential = true;
                }
                nextCardToPlay = FindCardToPlay(currentMana, "land");
                selectAbility();

                //look for the most expensive creature we can afford. If not found, try enchantment, then artifact, etc...
                const char* types[] = {"creature", "enchantment", "artifact", "sorcery", "instant"};
                int count = 0;
                while (!nextCardToPlay && count < 5)
                {
                    nextCardToPlay = FindCardToPlay(currentMana, types[count]);
                    if (game->playRestrictions->canPutIntoZone(nextCardToPlay, game->stack) == PlayRestriction::CANT_PLAY)
                        nextCardToPlay = NULL;
                    count++;
                }

                SAFE_DELETE(currentMana);

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
                                        if (nextCardToPlay->sunburst < nextCardToPlay->getManaCost()->getConvertedCost() || nextCardToPlay->getManaCost()->hasX())
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
                    }
                    if(tapLandsForMana(nextCardToPlay->getManaCost(),nextCardToPlay))
                    {
                        AIAction * a = NEW AIAction(nextCardToPlay);
                        clickstream.push(a);
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
            chooseAttackers();
            break;
        case Constants::MTG_PHASE_ENDOFTURN:
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
            {
                chooseBlockers();
                selectAbility();
                break;
            }
        default:
            selectAbility();
            break;
        }
        return 1;
    }
    return 1;
}
;

int AIPlayer::receiveEvent(WEvent * event)
{
    if (getStats())
        return getStats()->receiveEvent(event);
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

