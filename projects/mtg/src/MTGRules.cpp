#include "PrecompiledHeader.h"

#include "CardSelectorSingleton.h"
#include "MTGRules.h"
#include "Translate.h"
#include "Subtypes.h"
#include "GameOptions.h"

MTGPutInPlayRule::MTGPutInPlayRule(int _id) :
MTGAbility(_id, NULL)
{
    aType = MTGAbility::PUT_INTO_PLAY;
}

int MTGPutInPlayRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    int cardsinhand = game->players[0]->game->hand->nb_cards;
    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;
    if (!player->game->hand->hasCard(card))
        return 0;
    if ((game->turn < 1) && (cardsinhand != 0) && (card->basicAbilities[Constants::LEYLINE])
        && game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN
        && game->players[0]->game->graveyard->nb_cards == 0
        && game->players[0]->game->exile->nb_cards == 0
        )
    {

        if (card->basicAbilities[Constants::LEYLINE])
        {
            MTGCardInstance * copy = player->game->putInZone(card, player->game->hand, player->game->temp);
            Spell * spell = NEW Spell(copy);
            spell->resolve();
            delete spell;
        }
        return 1;
    }
    if(!allowedToCast(card,player))
        return 0;

    if (card->hasType("land"))
    {
        if (currentPlayer->game->playRestrictions->canPutIntoZone(card, currentPlayer->game->inPlay) == PlayRestriction::CANT_PLAY)
            return 0;
        if (player == currentPlayer
            && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN)
            )
        {
            return 1;
        }
    }
    else if ((card->hasType("instant")) || card->has(Constants::FLASH)
        || (player == currentPlayer && !game->isInterrupting
        && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN
        || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN))
        )
    {

        if (currentPlayer->game->playRestrictions->canPutIntoZone(card, currentPlayer->game->stack) == PlayRestriction::CANT_PLAY)
            return 0;
        ManaCost * playerMana = player->getManaPool();
        ManaCost * cost = card->getManaCost();

#ifdef WIN32
        cost->Dump();
#endif
        //cost of card.
        if (playerMana->canAfford(cost))
        {
            //-------
            if (card->has(Constants::SUNBURST))
            {
                for (int i = 1; i != 6; i++)
                {
                    if (player->getManaPool()->hasColor(i))
                    {
                        if (card->getManaCost()->hasColor(i) > 0)
                        {//do nothing if the card already has this color.
                        }
                        else
                        {
                            if (card->sunburst < card->getManaCost()->getConvertedCost())
                            {
                                card->getManaCost()->add(i, 1);
                                card->getManaCost()->remove(0, 1);
                                card->sunburst += 1;
                            }
                        }
                    }
                    //-------
                }
            }
            return 1;//play if you can afford too.
        }
    }
    return 0;//dont play if you cant afford it.
}

int MTGPutInPlayRule::reactToClick(MTGCardInstance * card)
{
    if (!isReactingToClick(card))
        return 0;
    Player * player = game->currentlyActing();
    ManaCost * cost = card->getManaCost();

    //this handles extra cost payments at the moment a card is played.

    if (cost->isExtraPaymentSet())
    {
        if (!game->targetListIsSet(card))
        {
            return 0;
        }
    }
    else
    {
        cost->setExtraCostsAction(this, card);
        game->mExtraPayment = cost->extraCosts;
        return 0;
    }

    ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
    int payResult = player->getManaPool()->pay(card->getManaCost());
    card->getManaCost()->doPayExtra();
    ManaCost * spellCost = previousManaPool->Diff(player->getManaPool());

    delete previousManaPool;
    if (card->hasType("land"))
    {
        MTGCardInstance * copy = player->game->putInZone(card, player->game->hand, player->game->temp);
        Spell * spell = NEW Spell(copy);
        spell->resolve();
        delete spellCost;
        delete spell;
    }
    else
    {
        Spell * spell = NULL;
        MTGCardInstance * copy = player->game->putInZone(card, player->game->hand, player->game->stack);
        if (game->targetChooser)
        {
            spell = game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, spellCost, payResult, 0);
            game->targetChooser = NULL;
        }
        else
        {
            spell = game->mLayers->stackLayer()->addSpell(copy, NULL, spellCost, payResult, 0);
        }

        if (card->has(Constants::STORM))
        {
            int storm = player->game->stack->seenThisTurn("*") + player->opponent()->game->stack->seenThisTurn("*");
            ManaCost * spellCost = player->getManaPool();
            for (int i = storm; i > 1; i--)
            {
                spell = game->mLayers->stackLayer()->addSpell(copy, NULL, spellCost, payResult, 1);

            }
        }//end of storm
        if (!card->has(Constants::STORM))
        {
            copy->X = spell->computeX(copy);
            copy->XX = spell->computeXX(copy);
        }
    }

    return 1;
}

//The Put into play rule is never destroyed
int MTGPutInPlayRule::testDestroy()
{
    return 0;
}

ostream& MTGPutInPlayRule::toString(ostream& out) const
{
    out << "MTGPutInPlayRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGPutInPlayRule * MTGPutInPlayRule::clone() const
{
    MTGPutInPlayRule * a = NEW MTGPutInPlayRule(*this);
    a->isClone = 1;
    return a;
}

//cast from anywhere possible with this??

//Alternative cost rules
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

MTGAlternativeCostRule::MTGAlternativeCostRule(int _id) :
MTGAbility(_id, NULL)
{
    aType = MTGAbility::ALTERNATIVE_COST;
}

int MTGAlternativeCostRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
	ManaCost * alternateCost = card->getManaCost()->alternative;
	if (!game->currentlyActing()->game->hand->hasCard(card))
 		return 0;
    return isReactingToClick( card, mana, alternateCost );
}

int MTGAlternativeCostRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana, ManaCost *alternateManaCost)
{
    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;

    if (!alternateManaCost)
        return 0;

    if(!allowedToCast(card,player))
        return 0;
    if(!allowedToAltCast(card,player))
        return 0;

    if (card->hasType("land"))
    {
        if (currentPlayer->game->playRestrictions->canPutIntoZone(card, currentPlayer->game->inPlay) == PlayRestriction::CANT_PLAY)
            return 0;
        if (player == currentPlayer
            && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN
            || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN)
            )
            return 1;
    }
    else if ((card->hasType("instant")) || card->has(Constants::FLASH) 
        || (player == currentPlayer && !game->isInterrupting
        && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN
        || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN))
        )
    {
        if (currentPlayer->game->playRestrictions->canPutIntoZone(card, currentPlayer->game->stack) == PlayRestriction::CANT_PLAY)
            return 0;
        ManaCost * playerMana = player->getManaPool();

#ifdef WIN32
        ManaCost * cost = card->getManaCost();
        cost->Dump();
#endif
        //cost of card.
        if (playerMana->canAfford(alternateManaCost))
        {
            return 1;
        }
    }
    return 0;//dont play if you cant afford it.
}

int MTGAlternativeCostRule::reactToClick(MTGCardInstance * card) 
{
	if ( !isReactingToClick(card))
		return 0;

	ManaCost *alternateCost = card->getManaCost()->alternative;
    Player * player = game->currentlyActing();
    ManaCost * playerMana = player->getManaPool();
	card->paymenttype = MTGAbility::ALTERNATIVE_COST;

    return reactToClick(card, card->getManaCost()->alternative, ManaCost::MANA_PAID_WITH_ALTERNATIVE);
}

int MTGAlternativeCostRule::reactToClick(MTGCardInstance * card, ManaCost *alternateCost, int alternateCostType){

    Player * player = game->currentlyActing();
    ManaPool * playerMana = player->getManaPool();
    //this handles extra cost payments at the moment a card is played.

    assert(alternateCost);
    if (alternateCost->isExtraPaymentSet() )
	{
		if (!game->targetListIsSet(card))
		    return 0;
	}
    else
    {
        alternateCost->setExtraCostsAction(this, card);
        game->mExtraPayment = alternateCost->extraCosts;
        return 0;
    }
    //------------------------------------------------------------------------
    
    playerMana->pay(alternateCost);
    alternateCost->doPayExtra();

    card->alternateCostPaid[alternateCostType] = 1;

    if (card->hasType("land"))
    {
        MTGCardInstance * copy = player->game->putInZone(card, card->currentZone, player->game->temp);
        Spell * spell = NEW Spell(copy);
        copy->alternateCostPaid[alternateCostType] = 1;
        spell->resolve();
        SAFE_DELETE(spell);
        game->mLayers->stackLayer()->addSpell(copy, NULL, NULL, alternateCostType, 1);
    }
    else
    {   
        ManaCost * previousManaPool = NEW ManaCost(playerMana); 
        ManaCost *spellCost = previousManaPool->Diff(player->getManaPool());
        SAFE_DELETE(previousManaPool);
        MTGCardInstance * copy = player->game->putInZone(card, card->currentZone, player->game->stack);
        copy->alternateCostPaid[alternateCostType] = 1;
        Spell * spell = game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, spellCost, alternateCostType, 0);
        game->targetChooser = NULL;

        if (card->has(Constants::STORM))
        {
           int storm = player->game->stack->seenThisTurn("*") + player->opponent()->game->stack->seenThisTurn("*");
           for (int i = storm; i > 1; i--)
            {
                game->mLayers->stackLayer()->addSpell(copy, NULL, playerMana, alternateCostType, 1);
            }
        }//end of storm
        else
        {
            copy->X = spell->computeX(copy);
            copy->XX = spell->computeXX(copy);
        }
    }

    
    return 1;
}


//The Put into play rule is never destroyed
int MTGAlternativeCostRule::testDestroy()
{
    return 0;
}

ostream& MTGAlternativeCostRule::toString(ostream& out) const
{
    out << "MTGAlternativeCostRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGAlternativeCostRule * MTGAlternativeCostRule::clone() const
{
    MTGAlternativeCostRule * a = NEW MTGAlternativeCostRule(*this);
    a->isClone = 1;
    return a;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//buyback follows its own resolving rules
MTGBuyBackRule::MTGBuyBackRule(int _id) :
MTGAlternativeCostRule(_id)
{
    aType = MTGAbility::BUYBACK_COST;
}

int MTGBuyBackRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    Player * player = game->currentlyActing();
    if(!allowedToCast(card,player))
        return 0;
    if (!player->game->hand->hasCard(card))
        return 0;
    return MTGAlternativeCostRule::isReactingToClick( card, mana, card->getManaCost()->BuyBack );
}

int MTGBuyBackRule::reactToClick(MTGCardInstance * card) 
{
    if (!isReactingToClick(card))
        return 0;

    Player *player = game->currentlyActing();
    ManaCost * playerMana = player->getManaPool();
    ManaCost * alternateCost = card->getManaCost()->BuyBack;
    
    card->paymenttype = MTGAbility::BUYBACK_COST;

    return MTGAlternativeCostRule::reactToClick(card, alternateCost, ManaCost::MANA_PAID_WITH_BUYBACK);

}

//The Put into play rule is never destroyed
int MTGBuyBackRule::testDestroy()
{
    return 0;
}

ostream& MTGBuyBackRule::toString(ostream& out) const
{
    out << "MTGBuyBackRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGBuyBackRule * MTGBuyBackRule::clone() const
{
    MTGBuyBackRule * a = NEW MTGBuyBackRule(*this);
    a->isClone = 1;
    return a;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//flashback follows its own resolving rules
MTGFlashBackRule::MTGFlashBackRule(int _id) :
MTGAlternativeCostRule(_id)
{
    aType = MTGAbility::FLASHBACK_COST;
}
int MTGFlashBackRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    Player * player = game->currentlyActing();
    if (!player->game->graveyard->hasCard(card))
        return 0;
    return MTGAlternativeCostRule::isReactingToClick(card, mana, card->getManaCost()->FlashBack );
}

int MTGFlashBackRule::reactToClick(MTGCardInstance * card) 
{
    Player *player = game->currentlyActing();
    ManaCost * alternateCost = card->getManaCost()->FlashBack;
    ManaCost * playerMana = game->currentlyActing()->getManaPool();
    if (!isReactingToClick(card))
        return 0;

    card->paymenttype = MTGAbility::FLASHBACK_COST;

    return MTGAlternativeCostRule::reactToClick(card, alternateCost, ManaCost::MANA_PAID_WITH_FLASHBACK);

}

//The Put into play rule is never destroyed
int MTGFlashBackRule::testDestroy()
{
    return 0;
}

ostream& MTGFlashBackRule::toString(ostream& out) const
{
    out << "MTGFlashBackRule ::: (";
    return MTGAbility::toString(out) << ")";
}
MTGFlashBackRule * MTGFlashBackRule::clone() const
{
    MTGFlashBackRule * a = NEW MTGFlashBackRule(*this);
    a->isClone = 1;
    return a;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//retrace
MTGRetraceRule::MTGRetraceRule(int _id) :
MTGAlternativeCostRule(_id)
{
    aType = MTGAbility::RETRACE_COST;
}

int MTGRetraceRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    Player * player = game->currentlyActing();
    ManaCost * alternateManaCost = card->getManaCost()->Retrace;

    if (!player->game->graveyard->hasCard(card))
        return 0;
        
    return MTGAlternativeCostRule::isReactingToClick( card, mana, alternateManaCost  );
}


int MTGRetraceRule::reactToClick(MTGCardInstance * card)
{
    if (!isReactingToClick(card))
        return 0;
    Player *player = game->currentlyActing();
    ManaCost * playerMana = player->getManaPool();
    ManaCost * alternateCost = card->getManaCost()->Retrace;
    
    card->paymenttype = MTGAbility::RETRACE_COST;

    return MTGAlternativeCostRule::reactToClick(card, alternateCost, ManaCost::MANA_PAID_WITH_RETRACE);
}


//The Put into play rule is never destroyed
int MTGRetraceRule::testDestroy()
{
    return 0;
}

ostream& MTGRetraceRule::toString(ostream& out) const
{
    out << "MTGRetraceRule ::: (";
    return MTGAbility::toString(out) << ")";
}
MTGRetraceRule * MTGRetraceRule::clone() const
{
    MTGRetraceRule * a = NEW MTGRetraceRule(*this);
    a->isClone = 1;
    return a;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

MTGMorphCostRule::MTGMorphCostRule(int _id) :
    MTGAbility(_id, NULL)
{
    aType = MTGAbility::MORPH_COST;
}
int MTGMorphCostRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    int cardsinhand = game->players[0]->game->hand->nb_cards;
    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;
    if (!player->game->hand->hasCard(card))
        return 0;
    if (!card->getManaCost()->morph)
        return 0;
    if(!allowedToCast(card,player))
        return 0;
    if(!allowedToAltCast(card,player))
        return 0;
    //note lands can morph too, this is different from other cost types.
    if ((card->hasType("instant")) || card->has(Constants::FLASH) || (player == currentPlayer
        && !game->isInterrupting
        && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN
        || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN))
        )
    {
        if (currentPlayer->game->playRestrictions->canPutIntoZone(card, currentPlayer->game->stack) == PlayRestriction::CANT_PLAY)
            return 0;
        ManaCost * playerMana = player->getManaPool();
        ManaCost * cost = card->getManaCost();
        ManaCost * morph = card->getManaCost()->morph;
#ifdef WIN32
        cost->Dump();
#endif
        //cost of card.
        if (morph && playerMana->canAfford(morph))
        {
            return 1;
        }
    }
    return 0;//dont play if you cant afford it.
}

int MTGMorphCostRule::reactToClick(MTGCardInstance * card)
{
//morphs reactToClick is extremely different then the other cost.
    if (!isReactingToClick(card))
        return 0;
    Player * player = game->currentlyActing();
    ManaCost * cost = card->getManaCost();
    ManaCost * morph = card->getManaCost()->morph;
    ManaCost * playerMana = player->getManaPool();
    //this handles extra cost payments at the moment a card is played.
    if (playerMana->canAfford(morph))
    {
        if (cost->morph->isExtraPaymentSet())
        {
            card->paymenttype = MTGAbility::MORPH_COST;
            if (!game->targetListIsSet(card))
            {
                return 0;
            }
        }
        else
        {
            cost->morph->setExtraCostsAction(this, card);
            game->mExtraPayment = cost->morph->extraCosts;
            card->paymenttype = MTGAbility::MORPH_COST;
            return 0;
        }
    }
    //------------------------------------------------------------------------
    ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
    int payResult = player->getManaPool()->pay(card->getManaCost()->morph);
    card->getManaCost()->morph->doPayExtra();
    payResult = ManaCost::MANA_PAID_WITH_MORPH;
    //if morph has a extra payment thats set, this code pays it.the if statement is 100% needed as it would cause a crash on cards that dont have the morph cost.
    if (morph)
    {
        card->getManaCost()->morph->doPayExtra();
    }
    //---------------------------------------------------------------------------
    ManaCost * spellCost = previousManaPool->Diff(player->getManaPool());
    delete previousManaPool;
    card->morphed = true;
    card->isMorphed = true;
    MTGCardInstance * copy = player->game->putInZone(card, card->currentZone, player->game->stack);
    Spell * spell = NULL;
    spell = game->mLayers->stackLayer()->addSpell(copy, NULL, spellCost, payResult, 0);
    spell->source->morphed = true;
    spell->source->isMorphed = true;
    spell->source->name = "";
    spell->source->power = 2;
    spell->source->toughness = 2;
    copy->morphed = true;
    copy->isMorphed = true;
    copy->power = 2;
    copy->toughness = 2;
    if (!card->has(Constants::STORM))
    {
        copy->X = spell->computeX(copy);
        copy->XX = spell->computeXX(copy);
    }
    return 1;
}

//The morph rule is never destroyed
int MTGMorphCostRule::testDestroy()
{
    return 0;
}

ostream& MTGMorphCostRule::toString(ostream& out) const
{
    out << "MTGMorphCostRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGMorphCostRule * MTGMorphCostRule::clone() const
{
    MTGMorphCostRule * a = NEW MTGMorphCostRule(*this);
    a->isClone = 1;
    return a;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------




bool MTGAttackRule::select(Target* t)
{
    if (CardView* c = dynamic_cast<CardView*>(t))
    {
        MTGCardInstance * card = c->getCard();
        if (card->canAttack() && !card->isPhased)
            return true;
    }
    return false;
}
bool MTGAttackRule::greyout(Target* t)
{
    return true;
}

MTGAttackRule::MTGAttackRule(int _id) :
MTGAbility(_id, NULL)
{
    aType = MTGAbility::MTG_ATTACK_RULE;
}

int MTGAttackRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (currentPhase == Constants::MTG_PHASE_COMBATATTACKERS && card->controller() == game->currentPlayer)
    {
        if(card->isPhased)
            return 0;
        if (card->isAttacker())
            return 1;
        if (card->canAttack())
            return 1;
    }
    return 0;
}

int MTGAttackRule::receiveEvent(WEvent *e)
{
    if (WEventPhaseChange* event = dynamic_cast<WEventPhaseChange*>(e))
    {
        if (Constants::MTG_PHASE_COMBATATTACKERS == event->from->id)
        {
            Player * p = game->currentPlayer;
            MTGGameZone * z = p->game->inPlay;
            for (int i = 0; i < z->nb_cards; i++)
            {
                MTGCardInstance * card = z->cards[i];
                if (!card->isAttacker() && card->has(Constants::MUSTATTACK))
                    reactToClick(card);
                if (!card->isAttacker() && card->has(Constants::TREASON) && p->isAI())
                    reactToClick(card);
                if (card->isAttacker() && card->isTapped())
                    card->setAttacker(0);
                if (card->isAttacker() && !card->has(Constants::VIGILANCE))
                    card->tap();
            }
            return 1;
        }
    }
    return 0;
}

int MTGAttackRule::reactToClick(MTGCardInstance * card)
{
    if (!isReactingToClick(card))
        return 0;

    //Graphically select the next card that can attack
    if (!card->isAttacker())
    {
        CardSelectorSingleton::Instance()->PushLimitor();
        CardSelectorSingleton::Instance()->Limit(this, CardView::playZone);
        CardSelectorSingleton::Instance()->CheckUserInput(JGE_BTN_RIGHT);
        CardSelectorSingleton::Instance()->Limit(NULL, CardView::playZone);
        CardSelectorSingleton::Instance()->PopLimitor();
    }
    card->toggleAttacker();
    return 1;
}

//The Attack rule is never destroyed
int MTGAttackRule::testDestroy()
{
    return 0;
}

ostream& MTGAttackRule::toString(ostream& out) const
{
    out << "MTGAttackRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGAttackRule * MTGAttackRule::clone() const
{
    MTGAttackRule * a = NEW MTGAttackRule(*this);
    a->isClone = 1;
    return a;
}

//this rules handles returning cards to combat triggers for activations.
MTGCombatTriggersRule::MTGCombatTriggersRule(int _id) :
MTGAbility(_id, NULL)
{
    aType = MTGAbility::MTG_COMBATTRIGGERS_RULE;
}

int MTGCombatTriggersRule::receiveEvent(WEvent *e)
{
    if (WEventPhaseChange* event = dynamic_cast<WEventPhaseChange*>(e))
    {
        if (Constants::MTG_PHASE_COMBATATTACKERS == event->from->id)
        {
            Player * p = game->currentPlayer;
            MTGGameZone * z = p->game->inPlay;
            for (int i = 0; i < z->nb_cards; i++)
            {
                MTGCardInstance * card = z->cards[i];
                if (card && card->isAttacker())
                {
                    card->eventattacked();
                }
            }
        }
        if (Constants::MTG_PHASE_COMBATEND == event->from->id)
        {
            Player * p = game->currentPlayer->opponent();
            MTGGameZone * z = p->game->inPlay;
            for (int i = 0; i < z->nb_cards; i++)
            {
                MTGCardInstance * card = z->cards[i];
                if (card)
                {
                    card->didattacked = 0;
                    card->didblocked = 0;
                    card->notblocked = 0;
                }
            }
        }
        //---------------
    }
    if (dynamic_cast<WEventAttackersChosen*>(e))
    {
        MTGCardInstance * lonelyAttacker = NULL;
        int nbattackers = 0;
        Player * p = game->currentPlayer;
        MTGGameZone * z = p->game->inPlay;
        int nbcards = z->nb_cards;
        for (int i = 0; i < nbcards; ++i)
        {
            MTGCardInstance * c = z->cards[i];
            if (c->attacker)
            {
                nbattackers++;
                lonelyAttacker = c;
            }
        }
        if (nbattackers == 1)
        {
            lonelyAttacker->eventattackedAlone();
        }
        else
            lonelyAttacker = NULL;
    }
    if (dynamic_cast<WEventBlockersChosen*>(e))
    {
        Player * p = game->currentPlayer;
        MTGGameZone * z = p->game->inPlay;
        for (int i = 0; i < z->nb_cards; i++)
        {
            MTGCardInstance * card = z->cards[i];
            if (card && card->isAttacker() && !card->blocked)
            {
                card->eventattackednotblocked();
                card->notblocked += 1;
            }
            if (card && card->isAttacker() && card->blocked)
            {
                card->eventattackedblocked();
            }
        }

        MTGGameZone* opponentZone = game->currentPlayer->opponent()->game->inPlay;
        for (int i = 0; i < opponentZone->nb_cards; i++)
        {
            MTGCardInstance* card = opponentZone->cards[i];
            if (card && card->didblocked > 0)
            {
                card->eventblocked();
            }
        }
    }
    return 0;
}

//trigger rules are never distroyed
int MTGCombatTriggersRule::testDestroy()
{
    return 0;
}

ostream& MTGCombatTriggersRule::toString(ostream& out) const
{
    out << "MTGCombatTriggersRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGCombatTriggersRule * MTGCombatTriggersRule::clone() const
{
    MTGCombatTriggersRule * a = NEW MTGCombatTriggersRule(*this);
    a->isClone = 1;
    return a;
}
///------------

OtherAbilitiesEventReceiver::OtherAbilitiesEventReceiver(int _id) :
MTGAbility(_id, NULL)
{
}

int OtherAbilitiesEventReceiver::receiveEvent(WEvent *e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    {
        if (event->to && (event->to != event->from))
        {
            GameObserver * g = GameObserver::GetInstance();
            for (int i = 0; i < 2; ++i)
            {
                if (event->to == g->players[i]->game->inPlay)
                    return 0;
            }
            AbilityFactory af;
            af.magicText(g->mLayers->actionLayer()->getMaxId(), NULL, event->card, 1, 0, event->to);
            return 1;
        }
    }
    return 0;
}

int OtherAbilitiesEventReceiver::testDestroy()
{
    return 0;
}

OtherAbilitiesEventReceiver * OtherAbilitiesEventReceiver::clone() const
{
    OtherAbilitiesEventReceiver * a = NEW OtherAbilitiesEventReceiver(*this);
    a->isClone = 1;
    return a;
}

MTGBlockRule::MTGBlockRule(int _id) :
MTGAbility(_id, NULL)
{
    aType = MTGAbility::MTG_BLOCK_RULE;
}

int MTGBlockRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (currentPhase == Constants::MTG_PHASE_COMBATBLOCKERS && !game->isInterrupting
        && card->controller() == game->currentlyActing()
        )
    {
        if (card->canBlock() && !card->isPhased)
            return 1;
    }
    return 0;
}

int MTGBlockRule::reactToClick(MTGCardInstance * card)
{
    if (!isReactingToClick(card))
        return 0;
    MTGCardInstance * currentOpponent = card->isDefenser();
    bool result = false;
    int canDefend = 0;
    while (!result)
    {
        currentOpponent = game->currentPlayer->game->inPlay->getNextAttacker(currentOpponent);
        canDefend = card->toggleDefenser(currentOpponent);

        DebugTrace("Defenser Toggle: " << card->getName() << endl
            << "- canDefend: " << (canDefend == 0) << endl
            << "- currentOpponent: " << currentOpponent);
        result = (canDefend || currentOpponent == NULL);
    }
    return 1;
}

//The Block rule is never destroyed
int MTGBlockRule::testDestroy()
{
    return 0;
}

ostream& MTGBlockRule::toString(ostream& out) const
{
    out << "MTGBlockRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGBlockRule * MTGBlockRule::clone() const
{
    MTGBlockRule * a = NEW MTGBlockRule(*this);
    a->isClone = 1;
    return a;
}
//
// Attacker chooses blockers order
//

//
// * Momir
//

int MTGMomirRule::initialized = 0;
vector<int> MTGMomirRule::pool[20];

MTGMomirRule::MTGMomirRule(int _id, MTGAllCards * _collection) :
MTGAbility(_id, NULL)
{
    collection = _collection;
    if (!initialized)
    {
        for (size_t i = 0; i < collection->ids.size(); i++)
        {
            MTGCard * card = collection->collection[collection->ids[i]];
            if (card->data->isCreature() && (card->getRarity() != Constants::RARITY_T) && //remove tokens
                card->setId != MTGSets::INTERNAL_SET //remove cards that are defined in primitives. Those are workarounds (usually tokens) and should only be used internally
                )
            {
                int convertedCost = card->data->getManaCost()->getConvertedCost();
                if (convertedCost > 20)
                    continue;
                pool[convertedCost].push_back(card->getMTGId());
            }
        }
        initialized = 1;
    }
    alreadyplayed = 0;
    aType = MTGAbility::MOMIR;
    textAlpha = 0;
}

int MTGMomirRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (alreadyplayed)
        return 0;
    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;
    if (!player->game->hand->hasCard(card))
        return 0;
    if (player == currentPlayer && !game->isInterrupting
        && (game->currentGamePhase == Constants::MTG_PHASE_FIRSTMAIN
        || game->currentGamePhase == Constants::MTG_PHASE_SECONDMAIN)
        )
    {
        return 1;
    }
    return 0;
}

int MTGMomirRule::reactToClick(MTGCardInstance * card_to_discard)
{
    Player * player = game->currentlyActing();
    ManaCost * cost = player->getManaPool();
    int converted = cost->getConvertedCost();
    int id = genRandomCreatureId(converted);
    return reactToClick(card_to_discard, id);
}

int MTGMomirRule::reactToClick(MTGCardInstance * card_to_discard, int cardId)
{
    if (!isReactingToClick(card_to_discard))
        return 0;
    Player * player = game->currentlyActing();
    ManaCost * cost = player->getManaPool();
    player->getManaPool()->pay(cost);
    MTGCardInstance * card = genCreature(cardId);
    player->game->putInZone(card_to_discard, player->game->hand, player->game->graveyard);

    player->game->stack->addCard(card);
    Spell * spell = NEW Spell(card);
    spell->resolve();
    spell->source->isToken = 1;
    delete spell;
    alreadyplayed = 1;
    textAlpha = 255;
    text = card->name;
    return 1;
}

MTGCardInstance * MTGMomirRule::genCreature(int id)
{
    if (!id)
        return NULL;
    Player * p = game->currentlyActing();
    MTGCard * card = collection->getCardById(id);
    return NEW MTGCardInstance(card, p->game);
}

int MTGMomirRule::genRandomCreatureId(int convertedCost)
{
    if (convertedCost >= 20)
        convertedCost = 19;
    int total_cards = 0;
    int i = convertedCost;
    while (!total_cards && i >= 0)
    {
        DebugTrace("Converted Cost in momir: " << i);
        total_cards = pool[i].size();
        convertedCost = i;
        i--;
    }
    if (!total_cards)
        return 0;
    int start = (WRand() % total_cards);
    return pool[convertedCost][start];
}

//The Momir rule is never destroyed
int MTGMomirRule::testDestroy()
{
    return 0;
}

void MTGMomirRule::Update(float dt)
{
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP)
    {
        alreadyplayed = 0;
    }
    if (textAlpha)
    {
        textAlpha -= static_cast<int> (200 * dt);
        if (textAlpha < 0)
            textAlpha = 0;
    }
    MTGAbility::Update(dt);
}

void MTGMomirRule::Render()
{
    if (!textAlpha)
        return;
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MENU_FONT);
    mFont->SetScale(2 - (float) textAlpha / 130);
    mFont->SetColor(ARGB(textAlpha,255,255,255));
    mFont->DrawString(text.c_str(), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, JGETEXT_CENTER);
}

ostream& MTGMomirRule::toString(ostream& out) const
{
    out << "MTGMomirRule ::: pool : " << pool << " ; initialized : " << initialized << " ; textAlpha : " << textAlpha
        << " ; text " << text << " ; alreadyplayed : " << alreadyplayed
        << " ; collection : " << collection << "(";
    return MTGAbility::toString(out) << ")";
}

MTGMomirRule * MTGMomirRule::clone() const
{
    MTGMomirRule * a = NEW MTGMomirRule(*this);
    a->isClone = 1;
    return a;
}

//HUDDisplay
int HUDDisplay::testDestroy()
{
    return 0;
}

void HUDDisplay::Update(float dt)
{
    timestamp += dt;
    popdelay += dt;
    if (events.size())
    {
        list<HUDString *>::iterator it = events.begin();
        HUDString * hs = *it;
        if (popdelay > 1 && timestamp - hs->timestamp > 2)
        {
            events.pop_front();
            delete hs;
            if (events.size())
                popdelay = 0;
        }
    }
    else
    {
        maxWidth = 0;
    }
}

int HUDDisplay::addEvent(string s)
{
    events.push_back(NEW HUDString(s, timestamp));
    float width = f->GetStringWidth(s.c_str());
    if (width > maxWidth)
        maxWidth = width;
    return 1;
}

int HUDDisplay::receiveEvent(WEvent * event)
{

    WEventZoneChange * ezc = dynamic_cast<WEventZoneChange*> (event);
    if (ezc)
    {
        for (int i = 0; i < 2; i++)
        {
            Player * p = game->players[i];
            if (ezc->to == p->game->graveyard)
            {
                char buffer[512];
                sprintf(buffer, _("%s goes to graveyard").c_str(), _(ezc->card->getName()).c_str());
                string s = buffer;
                return addEvent(s);
            }
        }
    }

    WEventDamage * ed = dynamic_cast<WEventDamage*> (event);
    if (ed)
    {
        char buffer[512];
        sprintf(buffer, "%s: %i -> %s", _(ed->damage->source->name).c_str(), ed->damage->damage, _(
            ed->damage->target->getDisplayName()).c_str());
        string s = buffer;
        return addEvent(s);
    }

    return 0;
}
void HUDDisplay::Render()
{
    if (!options[Options::OSD].number)
        return;
    if (!events.size())
        return;

    f->SetColor(ARGB(255,255,255,255));

    list<HUDString *>::reverse_iterator it;

    float x0 = SCREEN_WIDTH - 10 - maxWidth - 10;
    float y0 = 20;
    float size = static_cast<float> (events.size() * 16);
    JRenderer * r = JRenderer::GetInstance();
    r->FillRoundRect(x0, y0, maxWidth + 10, size, 5, ARGB(50,0,0,0));

    int i = 0;
    for (it = events.rbegin(); it != events.rend(); ++it)
    {
        HUDString * hs = *it;
        f->DrawString(hs->value.c_str(), x0 + 5, y0 + 16 * i);
        i++;
    }
}
HUDDisplay::HUDDisplay(int _id) :
MTGAbility(_id, NULL)
{
    timestamp = 0;
    popdelay = 2;
    f = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    maxWidth = 0;
}

HUDDisplay::~HUDDisplay()
{
    list<HUDString *>::iterator it;
    for (it = events.begin(); it != events.end(); ++it)
    {
        HUDString * hs = *it;
        delete hs;
    }
    events.clear();
}

HUDDisplay * HUDDisplay::clone() const
{
    HUDDisplay * a = NEW HUDDisplay(*this);
    a->isClone = 1;
    return a;
}

/* Persist */
MTGPersistRule::MTGPersistRule(int _id) :
MTGAbility(_id, NULL)
{
}
;

int MTGPersistRule::receiveEvent(WEvent * event)
{
    if (event->type == WEvent::CHANGE_ZONE)
    {
        WEventZoneChange * e = (WEventZoneChange *) event;
        MTGCardInstance * card = e->card->previous;
        if (card && card->basicAbilities[Constants::PERSIST] && !card->counters->hasCounter(-1, -1))
        {
            int ok = 0;
            for (int i = 0; i < 2; i++)
            {
                Player * p = game->players[i];
                if (e->from == p->game->inPlay)
                    ok = 1;
            }
            if (!ok)
                return 0;
            for (int i = 0; i < 2; i++)
            {
                Player * p = game->players[i];
                if (e->to == p->game->graveyard)
                {
                    MTGCardInstance * copy = p->game->putInZone(e->card, p->game->graveyard, e->card->owner->game->stack);
                    if (!copy)
                    {
                        DebugTrace("MTGRULES: couldn't move card for persist");
                        return 0;
                    }
                    Spell * spell = NEW Spell(copy);
                    spell->resolve();
                    spell->source->counters->addCounter(-1, -1);
                    delete spell;
                    return 1;
                }
            }
        }
    }
    return 0;
}

ostream& MTGPersistRule::toString(ostream& out) const
{
    out << "MTGPersistRule ::: (";
    return MTGAbility::toString(out) << ")";
}
int MTGPersistRule::testDestroy()
{
    return 0;
}
MTGPersistRule * MTGPersistRule::clone() const
{
    MTGPersistRule * a = NEW MTGPersistRule(*this);
    a->isClone = 1;
    return a;
}

//unearth rule----------------------------------
//if the card leaves play, exile it instead.
MTGUnearthRule::MTGUnearthRule(int _id) :
MTGAbility(_id, NULL)
{
}
;

int MTGUnearthRule::receiveEvent(WEvent * event)
{
    if (event->type == WEvent::CHANGE_ZONE)
    {
        WEventZoneChange * e = (WEventZoneChange *) event;
        MTGCardInstance * card = e->card->previous;
        if (e->from == e->card->controller()->game->battlefield && e->to == e->card->controller()->game->graveyard)
        {
            e->card->fresh = 1;
        }
        if (e->to == e->card->controller()->game->battlefield)
        {
            e->card->fresh = 1;
        }

        if (card && card->basicAbilities[Constants::UNEARTH])
        {
            int ok = 0;
            for (int i = 0; i < 2; i++)
            {
                Player * p = game->players[i];
                if (e->from == p->game->inPlay)
                    ok = 1;
            }
            if (!ok)
                return 0;
            for (int i = 0; i < 2; i++)
            {
                Player * p = game->players[i];
                if (e->to == p->game->graveyard || e->to == p->game->hand || e->to == p->game->library)
                {
                    p->game->putInExile(e->card);
                    return 1;
                }
            }
        }
    }
    return 0;
}

ostream& MTGUnearthRule::toString(ostream& out) const
{
    out << "MTGUnearthRule ::: (";
    return MTGAbility::toString(out) << ")";
}
int MTGUnearthRule::testDestroy()
{
    return 0;
}
MTGUnearthRule * MTGUnearthRule::clone() const
{
    MTGUnearthRule * a = NEW MTGUnearthRule(*this);
    a->isClone = 1;
    return a;
}
//token clean up
MTGTokensCleanup::MTGTokensCleanup(int _id) :
MTGAbility(_id, NULL)
{
}

int MTGTokensCleanup::receiveEvent(WEvent * e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    {
        if (!event->card->isToken)
            return 0;
        if (event->to == game->players[0]->game->inPlay || event->to == game->players[1]->game->inPlay)
            return 0;
        if (event->to == game->players[0]->game->garbage || event->to == game->players[1]->game->garbage)
            return 0;
        MTGCardInstance * c = event->card;
        c->controller()->game->putInZone(c, c->currentZone, c->controller()->game->garbage);
        return 1;
    }
    return 0;
}

int MTGTokensCleanup::testDestroy()
{
    return 0;
}

MTGTokensCleanup * MTGTokensCleanup::clone() const
{
    MTGTokensCleanup * a = NEW MTGTokensCleanup(*this);
    a->isClone = 1;
    return a;
}

/* Legend Rule */
MTGLegendRule::MTGLegendRule(int _id) :
ListMaintainerAbility(_id)
{
}
;

int MTGLegendRule::canBeInList(MTGCardInstance * card)
{
    if(card->isPhased)
        return 0;
    if (card->hasType(Subtypes::TYPE_LEGENDARY) && game->isInPlay(card))
    {
        return 1;
    }
    return 0;
}

int MTGLegendRule::added(MTGCardInstance * card)
{
    map<MTGCardInstance *, bool>::iterator it;
    int destroy = 0;
    for (it = cards.begin(); it != cards.end(); it++)
    {
        MTGCardInstance * comparison = (*it).first;
        if (comparison != card && !(comparison->getName().compare(card->getName())))
        {
            comparison->controller()->game->putInGraveyard(comparison);
            destroy = 1;
        }
    }
    if (destroy)
    {
        card->owner->game->putInGraveyard(card);
    }
    return 1;
}

int MTGLegendRule::removed(MTGCardInstance * card)
{
    return 0;
}

int MTGLegendRule::testDestroy()
{
    return 0;
}

ostream& MTGLegendRule::toString(ostream& out) const
{
    return out << "MTGLegendRule :::";
}
MTGLegendRule * MTGLegendRule::clone() const
{
    MTGLegendRule * a = NEW MTGLegendRule(*this);
    a->isClone = 1;
    return a;
}

/* PlaneWalker Rule */
MTGPlaneWalkerRule::MTGPlaneWalkerRule(int _id) :
ListMaintainerAbility(_id)
{
}
;

int MTGPlaneWalkerRule::canBeInList(MTGCardInstance * card)
{
    if(card->isPhased)
        return 0;
    if (card->hasType("Planeswalker") && game->isInPlay(card))
    {
        return 1;
    }
    return 0;
}

int MTGPlaneWalkerRule::added(MTGCardInstance * card)
{
    map<MTGCardInstance *, bool>::iterator it;
    int destroy = 0;
    for (it = cards.begin(); it != cards.end(); it++)
    {
        MTGCardInstance * comparison = (*it).first;
        if (comparison != card && comparison->types == card->types)
        {
            comparison->controller()->game->putInGraveyard(comparison);
            destroy = 1;
        }
    }
    if (destroy)
    {
        card->owner->game->putInGraveyard(card);
    }
    return 1;
}

int MTGPlaneWalkerRule::removed(MTGCardInstance * card)
{
    return 0;
}

int MTGPlaneWalkerRule::testDestroy()
{
    return 0;
}

ostream& MTGPlaneWalkerRule::toString(ostream& out) const
{
    return out << "MTGLegendRule :::";
}
MTGPlaneWalkerRule * MTGPlaneWalkerRule::clone() const
{
    MTGPlaneWalkerRule * a = NEW MTGPlaneWalkerRule(*this);
    a->isClone = 1;
    return a;
}

/* Lifelink */
MTGLifelinkRule::MTGLifelinkRule(int _id) :
MTGAbility(_id, NULL)
{
}
;

int MTGLifelinkRule::receiveEvent(WEvent * event)
{
    if (event->type == WEvent::DAMAGE)
    {
        WEventDamage * e = (WEventDamage *) event;
        Damage * d = e->damage;
        MTGCardInstance * card = d->source;
        if (d->damage > 0 && card && card->basicAbilities[Constants::LIFELINK])
        {
            card->controller()->gainLife(d->damage);
            return 1;
        }
    }
    return 0;
}

int MTGLifelinkRule::testDestroy()
{
    return 0;
}

ostream& MTGLifelinkRule::toString(ostream& out) const
{
    out << "MTGLifelinkRule ::: (";
    return MTGAbility::toString(out) << ")";
}
MTGLifelinkRule * MTGLifelinkRule::clone() const
{
    MTGLifelinkRule * a = NEW MTGLifelinkRule(*this);
    a->isClone = 1;
    return a;
}

/* Deathtouch */
MTGDeathtouchRule::MTGDeathtouchRule(int _id) :
MTGAbility(_id, NULL)
{
}
;

int MTGDeathtouchRule::receiveEvent(WEvent * event)
{
    if (event->type == WEvent::DAMAGE)
    {
        WEventDamage * e = (WEventDamage *) event;

        Damage * d = e->damage;
        if (d->damage <= 0)
            return 0;

        MTGCardInstance * card = d->source;
        if (!card)
            return 0;

        if (d->target->type_as_damageable != DAMAGEABLE_MTGCARDINSTANCE)
            return 0;
        MTGCardInstance * _target = (MTGCardInstance *) (d->target);

        if (card->basicAbilities[Constants::DEATHTOUCH])
        {
            _target->destroy();
            return 1;
        }
    }
    return 0;
}

int MTGDeathtouchRule::testDestroy()
{
    return 0;
}

MTGDeathtouchRule * MTGDeathtouchRule::clone() const
{
    MTGDeathtouchRule * a = NEW MTGDeathtouchRule(*this);
    a->isClone = 1;
    return a;
}
