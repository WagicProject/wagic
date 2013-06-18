#include "PrecompiledHeader.h"

#include "CardSelector.h"
#include "MTGRules.h"
#include "Translate.h"
#include "Subtypes.h"
#include "Credits.h"
#include "AllAbilities.h"

PermanentAbility::PermanentAbility(GameObserver* observer, int _id) : MTGAbility(observer, _id,NULL)
{
}

MTGEventBonus::MTGEventBonus(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    textAlpha = 0;
    text = "";
    for(int i = 0;i < 2;i++)
    {
        chain[i] = 0;
        highestChain[i] = 0;
        //-----------
        army[i] = 0;
        army1[i] = false;
        army2[i] = false;
        army3[i] = false;
        //--------

        toybonusgranted[i] = false;
        toys[i] = 0;
        beastbonusgranted[i] = false;
        beast[i] = 0;
        zombiebonusgranted[i] = false;
        zombie[i] = 0;
        knightbonusgranted[i] = false;
        knight[i] = 0;
        insectbonusgranted[i] = false;
        insect[i] = 0;
        elementalbonusgranted[i] = false;
        elemental[i] = 0;
        vampirebonusgranted[i] = false;
        vampire[i] = 0;
        clericbonusgranted[i] = false;
        cleric[i] = 0;
        elfbonusgranted[i] = false;
        elf[i] = 0;
        Angelbonusgranted[i] = false;
        Angel[i] = 0;
        dragonbonusgranted[i] = false;
        dragon[i] = 0;
    
    }
}
int MTGEventBonus::receiveEvent(WEvent * event)
{
    Player * currentPlayer = game->currentPlayer;
    //bonus for chain chain casting without tapping for mana or being interupted;
    //note gaining mana from other sources is still possible.
    //only spells going to the stack are counted.
	if(game->turn <2)//this shouldnt trigger on first turn, chances are they are cheating.
		return 0;
    if (dynamic_cast<WEventCardTappedForMana*>(event))
    {

        if(chain[currentPlayer->getId()]/5 > 0)
        {
            text = "Chain Broken!";
            textAlpha = 255;
        }
        chain[currentPlayer->getId()] = 0;

    }
    if (event->type == WEvent::CHANGE_ZONE && !currentPlayer->isAI())
    {
        WEventZoneChange * e = (WEventZoneChange *) event;
        if (e->to == currentPlayer->game->stack)
        {
            chain[currentPlayer->getId()]++;
            if(chain[currentPlayer->getId()] > highestChain[currentPlayer->getId()])
                highestChain[currentPlayer->getId()] = chain[currentPlayer->getId()];
            if(chain[currentPlayer->getId()] > 4)
            {

                if(highestChain[currentPlayer->getId()] > 14)
                {
                    char buffer3[20];
                    sprintf(buffer3,"Killer!-Combo %i",chain[currentPlayer->getId()]);
                    grantAward(buffer3,100);
                    //increase the chains bonus by 100 for every card after playing a chain of 15 in a match.
                    //this is almost impossible would require superior draw and mana production.
                }
                else if(highestChain[currentPlayer->getId()] > 9)
                {
                    char buffer2[30];
                    sprintf(buffer2,"Abundant Resources-Combo %i",chain[currentPlayer->getId()]);
                    grantAward(buffer2,50);
                    //increase the chains bonus by 50 for every card after playing a chain of 10 in a match.
                    //this is extremely hard to do. would require a very well built deck an an abundence of mana
                    //to spend in a single go combined with decent card drawing.
                }
                else if(highestChain[currentPlayer->getId()] > 4)
                {
                    char buffer[20];
                    sprintf(buffer,"Chained-Combo %i",chain[currentPlayer->getId()]);
                    grantAward(buffer,chain[currentPlayer->getId()]);
                    //gain credits for every card played after you played a chain of 5
                    //during the match. this would require a very decent hand to do
                    //and good mana production.
                }
            }
        }
        //end of chain bonuses
        //==========================
        //creatures entering play consecutively will allow you a chance 
        //to gain a bonus for maintaining force sizes, it will trigger every 10th
        //creature which enters play consecutively.
        if (e->to == currentPlayer->game->inPlay && !currentPlayer->isAI())
        {
            if(e->card->hasType(Subtypes::TYPE_CREATURE))
                army[currentPlayer->getId()]++;
            else
                army[currentPlayer->getId()] = 0;
            if(army[currentPlayer->getId()] > 9)
            {
                //this might seem easy at first glance, but you have to both maintain a high
                //creature count, and triggers when 10 or more enter consecutively. if any thing else
                //enters play the count is reset.
                army[currentPlayer->getId()] = 0;
                int forceSize = currentPlayer->inPlay()->countByType("creature");
                if(forceSize > 40 && !army3[currentPlayer->getId()])
                {
                    grantAward("Malignant Conqueror Bonus!",1000);
                    army3[currentPlayer->getId()] = true;
                }
                else if(forceSize > 19 && !army2[currentPlayer->getId()])
                {
                    grantAward("Extreme Infantry Bonus!",500);
                    army2[currentPlayer->getId()] = true;
                }
                else if(forceSize > 9 && !army1[currentPlayer->getId()])
                {
                    grantAward("Deadly Force Bonus!",250);
                    army1[currentPlayer->getId()] = true;
                }
            }
            //////bonus for having a LOT of specific type.
            //not else'd becuase it is possible for a card to contain
            //more then one of the types, and for more then one to trigger.
            if(e->card->hasType(Subtypes::TYPE_ARTIFACT))
                toys[currentPlayer->getId()]++;
            if(e->card->isCreature())
            {
                if(e->card->hasType("beast"))
                    beast[currentPlayer->getId()]++;
                if(e->card->hasType("vampire"))
                    vampire[currentPlayer->getId()]++;
                if(e->card->hasType("insect"))
                    insect[currentPlayer->getId()]++;
                if(e->card->hasType("elemental"))
                    elemental[currentPlayer->getId()]++;
                if(e->card->hasType("zombie"))
                    zombie[currentPlayer->getId()]++;
                if(e->card->hasType("soldier")||e->card->hasType("knight")||e->card->hasType("warrior"))
                    knight[currentPlayer->getId()]++;
                if(e->card->hasType("cleric")||e->card->hasType("shaman")||e->card->hasType("druid"))
                    cleric[currentPlayer->getId()]++;
                if(e->card->hasType("elf"))
                    elf[currentPlayer->getId()]++;
                if(e->card->hasType("angel")||e->card->hasType("spirit"))
                    Angel[currentPlayer->getId()]++;
                if(e->card->hasType("dragon")||e->card->hasType("wurm")||e->card->hasType("drake")||e->card->hasType("snake")||e->card->hasType("hydra"))
                    dragon[currentPlayer->getId()]++;
            }
            if(toys[currentPlayer->getId()] > 30 && !toybonusgranted[currentPlayer->getId()])
            {
                grantAward("Toy Collector!",300);
                toybonusgranted[currentPlayer->getId()] = true;
            }
            if(beast[currentPlayer->getId()] > 30 && !beastbonusgranted[currentPlayer->getId()])
            {
                grantAward("Beast Tamer!",300);
                beastbonusgranted[currentPlayer->getId()] = true;
            }
            if(vampire[currentPlayer->getId()] > 30 && !vampirebonusgranted[currentPlayer->getId()])
            {
                grantAward("Vampire King!",300);
                vampirebonusgranted[currentPlayer->getId()] = true;
            }
            if(insect[currentPlayer->getId()] > 30 && !insectbonusgranted[currentPlayer->getId()])
            {
                grantAward("Lord of Swarms!",300);
                insectbonusgranted[currentPlayer->getId()] = true;
            }
            if(elemental[currentPlayer->getId()] > 30 && !elementalbonusgranted[currentPlayer->getId()])
            {
                grantAward("Master of Elements!",300);
                elementalbonusgranted[currentPlayer->getId()] = true;
            }
            if(zombie[currentPlayer->getId()] > 30 && !zombiebonusgranted[currentPlayer->getId()])
            {
                grantAward("Zombie Apocalypse!",300);
                zombiebonusgranted[currentPlayer->getId()] = true;
            }
            if(knight[currentPlayer->getId()] > 30 && !knightbonusgranted[currentPlayer->getId()])
            {
                grantAward("Sword And Shield!",300);
                knightbonusgranted[currentPlayer->getId()] = true;
            }
            if(cleric[currentPlayer->getId()] > 30 && !clericbonusgranted[currentPlayer->getId()])
            {
                grantAward("Medic!",300);
                clericbonusgranted[currentPlayer->getId()] = true;
            }

            if(elf[currentPlayer->getId()] > 30 && !elfbonusgranted[currentPlayer->getId()])
            {
                grantAward("The Promenade!",300);
                elfbonusgranted[currentPlayer->getId()] = true;
            }
            if(Angel[currentPlayer->getId()] > 30 && !Angelbonusgranted[currentPlayer->getId()])
            {
                grantAward("Heavenly Host!",300);
                Angelbonusgranted[currentPlayer->getId()] = true;
            }
            if(dragon[currentPlayer->getId()] > 30 && !dragonbonusgranted[currentPlayer->getId()])
            {
                grantAward("Teeth And Scales!",300);
                dragonbonusgranted[currentPlayer->getId()] = true;
            }
        }
    }
    //bonus for dealing 100+ damage from a single source
    WEventDamage * damageEvent = dynamic_cast<WEventDamage *> (event);
    if(damageEvent && !currentPlayer->isAI())
    {
        MTGCardInstance * damageSource = (MTGCardInstance*)damageEvent->getTarget(damageEvent->TARGET_FROM);
        if(damageSource && damageSource->controller() == currentPlayer && damageEvent->damage->damage > 99)
            grantAward("Overkill!",500);
    }
    return 1;
}

void MTGEventBonus::grantAward(string awardName,int amount)
{
    text = awardName;
    textAlpha = 255;
    Credits::addCreditBonus(amount);
}

void MTGEventBonus::Update(float dt)
{
    if (textAlpha)
    {
        textAlpha -= static_cast<int> (200 * dt);
        if (textAlpha < 0)
            textAlpha = 0;
    }
    MTGAbility::Update(dt);
}

void MTGEventBonus::Render()
{
    if (!textAlpha)
        return;
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT/*MENU_FONT*/);
    mFont->SetScale(2 - (float) textAlpha / 130);
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(text.c_str(), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, JGETEXT_CENTER);
}

MTGEventBonus * MTGEventBonus::clone() const
{
    return NEW MTGEventBonus(*this);
}
MTGPutInPlayRule::MTGPutInPlayRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    aType = MTGAbility::PUT_INTO_PLAY;
}

int MTGPutInPlayRule::isReactingToClick(MTGCardInstance * card, ManaCost *)
{
    int cardsinhand = game->players[0]->game->hand->nb_cards;
    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;
    if (!player->game->hand->hasCard(card))
        return 0;
    if ((game->turn < 1) && (cardsinhand != 0) && (card->basicAbilities[(int)Constants::LEYLINE])
        && game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
        && game->players[0]->game->graveyard->nb_cards == 0
        && game->players[0]->game->exile->nb_cards == 0
        )
    {

        if (card->basicAbilities[(int)Constants::LEYLINE])
        {
            MTGCardInstance * copy = player->game->putInZone(card, player->game->hand, player->game->temp);
            Spell * spell = NEW Spell(game, copy);
            spell->resolve();
            delete spell;
        }
        return 1;
    }
    if(!allowedToCast(card,player))
        return 0;

    if (card->isLand())
    {
        if (game->currentActionPlayer->game->playRestrictions->canPutIntoZone(card, game->currentActionPlayer->game->inPlay) == PlayRestriction::CANT_PLAY)
            return 0;
        if (player == currentPlayer
            && (game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN || game->getCurrentGamePhase() == MTG_PHASE_SECONDMAIN)
            )
        {
            return 1;
        }
    }
    else if ((card->hasType(Subtypes::TYPE_INSTANT)) || card->has(Constants::FLASH)
        || (player == currentPlayer && !game->isInterrupting
        && (game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
        || game->getCurrentGamePhase() == MTG_PHASE_SECONDMAIN))
        )
    {

        if (game->currentActionPlayer->game->playRestrictions->canPutIntoZone(card, game->currentActionPlayer->game->stack) == PlayRestriction::CANT_PLAY)
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
                            if (card->sunburst < card->getManaCost()->getConvertedCost() || card->getManaCost()->hasX())
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
    if (card->getManaCost()->kicker && (OptionKicker::KICKER_ALWAYS == options[Options::KICKERPAYMENT].number || card->controller()->isAI()))
    {
        ManaCost * withKickerCost= NEW ManaCost(card->getManaCost());
        withKickerCost->add(withKickerCost->kicker);
        if (card->getManaCost()->kicker->isMulti)
        {
            while(previousManaPool->canAfford(withKickerCost))
            {
                withKickerCost->add(withKickerCost->kicker);
                card->kicked += 1;
            }
            for(int i = 0;i < card->kicked;i++)
                player->getManaPool()->pay(card->getManaCost()->kicker);
            payResult = ManaCost::MANA_PAID_WITH_KICKER;
        }
        else if (previousManaPool->canAfford(withKickerCost))
        {
            player->getManaPool()->pay(card->getManaCost()->kicker);
            payResult = ManaCost::MANA_PAID_WITH_KICKER;
        }
        delete withKickerCost;
    }
    card->getManaCost()->doPayExtra();
    ManaCost * spellCost = previousManaPool->Diff(player->getManaPool());

    delete previousManaPool;
    if (card->isLand())
    {
        MTGCardInstance * copy = player->game->putInZone(card, player->game->hand, player->game->temp);
        Spell * spell = NEW Spell(game, 0,copy,NULL,NULL, payResult);
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
            int storm = player->game->stack->seenThisTurn("*", Constants::CAST_ALL) + player->opponent()->game->stack->seenThisTurn("*", Constants::CAST_ALL);
            ManaCost * spellCost = player->getManaPool();
            for (int i = storm; i > 1; i--)
            {
                spell = game->mLayers->stackLayer()->addSpell(copy, NULL, spellCost, payResult, 1);

            }
        }//end of storm
        if (!card->has(Constants::STORM))
        {
            copy->X = spell->computeX(copy);
            copy->castX = copy->X;
        }
    }

    return 1;
}

ostream& MTGPutInPlayRule::toString(ostream& out) const
{
    out << "MTGPutInPlayRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGPutInPlayRule * MTGPutInPlayRule::clone() const
{
    return NEW MTGPutInPlayRule(*this);
}


//kicker
MTGKickerRule::MTGKickerRule(GameObserver* observer, int _id) :
MTGPutInPlayRule(observer, _id)
{
    aType = MTGAbility::PUT_INTO_PLAY_WITH_KICKER;
}
int MTGKickerRule::isReactingToClick(MTGCardInstance * card, ManaCost *)
{
    if(OptionKicker::KICKER_ALWAYS == options[Options::KICKERPAYMENT].number)
        return 0;
    Player * player = game->currentlyActing();
    if(!player->game->hand->hasCard(card))
		return 0;
    ManaCost * kicker = card->getManaCost()->kicker;
    if(!kicker)
    {
		SAFE_DELETE(kicker);
        return 0;
    }
    ManaCost * playerMana = player->getManaPool();
    ManaCost * withKickerCost= NEW ManaCost(card->getManaCost());
    withKickerCost->add(withKickerCost->kicker);
    if(!playerMana->canAfford(withKickerCost))
    {
        delete withKickerCost;
        return 0;
    }
    delete withKickerCost;
    
    
    return 1;
}

int MTGKickerRule::reactToClick(MTGCardInstance * card)
{
    if(!isReactingToClick(card, NULL))
        return 0;
        
    Player * player = game->currentlyActing();
    ManaCost * withKickerCost= NEW ManaCost(card->getManaCost());//using pointers here alters the real cost of the card.
    if (card->getManaCost()->kicker->isMulti)
    {
        while(player->getManaPool()->canAfford(withKickerCost))
        {
            withKickerCost->add(withKickerCost->kicker);
            card->kicked += 1;
        }
        card->kicked -= 1;
        //for(int i = 0;i < card->kicked;i++)
            //player->getManaPool()->pay(card->getManaCost()->kicker);
        card->paymenttype = MTGAbility::PUT_INTO_PLAY_WITH_KICKER;
    }
    else
    {
        withKickerCost->add(withKickerCost->kicker);
        card->paymenttype = MTGAbility::PUT_INTO_PLAY_WITH_KICKER;
    }
    if (withKickerCost->isExtraPaymentSet())
    {
        if (!game->targetListIsSet(card))
        {
        delete withKickerCost;
            return 0;
        }
    }
    else
    {
        withKickerCost->setExtraCostsAction(this, card);
        game->mExtraPayment = withKickerCost->extraCosts;
        delete withKickerCost;
        return 0;
    }

    ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
    player->getManaPool()->pay(withKickerCost);
    withKickerCost->doPayExtra();
    ManaCost * spellCost = previousManaPool->Diff(player->getManaPool());
    delete withKickerCost;
    delete previousManaPool;
    if (card->isLand())
    {
        MTGCardInstance * copy = player->game->putInZone(card, player->game->hand, player->game->temp);
        Spell * spell = NEW Spell(game, 0,copy,NULL,NULL, ManaCost::MANA_PAID_WITH_KICKER);
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
            spell = game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, spellCost, ManaCost::MANA_PAID_WITH_KICKER, 0);
            game->targetChooser = NULL;
        }
        else
        {
            spell = game->mLayers->stackLayer()->addSpell(copy, NULL, spellCost, ManaCost::MANA_PAID_WITH_KICKER, 0);
        }

        if (card->has(Constants::STORM))
        {
            int storm = player->game->stack->seenThisTurn("*", Constants::CAST_ALL) + player->opponent()->game->stack->seenThisTurn("*", Constants::CAST_ALL);
            ManaCost * stormSpellCost = player->getManaPool();
            for (int i = storm; i > 1; i--)
            {
                spell = game->mLayers->stackLayer()->addSpell(copy, NULL, stormSpellCost, ManaCost::MANA_PAID_WITH_KICKER, 1);

            }
        }//end of storm
        if (!card->has(Constants::STORM))
        {
            copy->X = spell->computeX(copy);
            copy->castX = copy->X;
        }
    }

    return 1;
}

ostream& MTGKickerRule::toString(ostream& out) const
{
    out << "MTGKickerRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGKickerRule * MTGKickerRule::clone() const
{
    return NEW MTGKickerRule(*this);
}




//cast from anywhere possible with this??
//Alternative cost rules
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

MTGAlternativeCostRule::MTGAlternativeCostRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
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

int MTGAlternativeCostRule::isReactingToClick(MTGCardInstance * card, ManaCost *, ManaCost *alternateManaCost)
{
    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;

    if (!alternateManaCost)
        return 0;
    if(!allowedToAltCast(card,player))
        return 0;

    if(card->model->data->getManaCost()->alternative && card->model->data->getManaCost()->alternative->alternativeName.size())
        alternativeName = card->model->data->getManaCost()->alternative->alternativeName;

    if (card->isLand())
    {
        if (game->currentActionPlayer->game->playRestrictions->canPutIntoZone(card, game->currentActionPlayer->game->inPlay) == PlayRestriction::CANT_PLAY)
            return 0;
        if (player == currentPlayer
            && (game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
            || game->getCurrentGamePhase() == MTG_PHASE_SECONDMAIN)
            )
            return 1;
    }
    else if ((card->hasType(Subtypes::TYPE_INSTANT)) || card->has(Constants::FLASH) 
        || (player == currentPlayer && !game->isInterrupting
        && (game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
        || game->getCurrentGamePhase() == MTG_PHASE_SECONDMAIN))
        )
    {
        if (game->currentActionPlayer->game->playRestrictions->canPutIntoZone(card, game->currentActionPlayer->game->stack) == PlayRestriction::CANT_PLAY)
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
	card->paymenttype = MTGAbility::ALTERNATIVE_COST;

    return reactToClick(card, alternateCost, ManaCost::MANA_PAID_WITH_ALTERNATIVE);
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
    ManaCost * previousManaPool = NEW ManaCost(playerMana); 
    playerMana->pay(alternateCost);
    alternateCost->doPayExtra();
    ManaCost *spellCost = previousManaPool->Diff(player->getManaPool());
    SAFE_DELETE(previousManaPool);

    card->alternateCostPaid[alternateCostType] = 1;

    if (card->isLand())
    {
        MTGCardInstance * copy = player->game->putInZone(card, card->currentZone, player->game->temp);
        Spell * spell = NEW Spell(game, 0,copy,NULL,NULL, alternateCostType);
        copy->alternateCostPaid[alternateCostType] = 1;
        spell->resolve();
        SAFE_DELETE(spell);
        game->mLayers->stackLayer()->addSpell(copy, NULL, NULL, alternateCostType, 1);
    }
    else
    {   
        MTGCardInstance * copy = player->game->putInZone(card, card->currentZone, player->game->stack);
        copy->alternateCostPaid[alternateCostType] = 1;
        game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, spellCost, alternateCostType, 0);
        game->targetChooser = NULL;

        if (card->has(Constants::STORM))
        {
           int storm = player->game->stack->seenThisTurn("*", Constants::CAST_ALL) + player->opponent()->game->stack->seenThisTurn("*", Constants::CAST_ALL);
           for (int i = storm; i > 1; i--)
            {
                game->mLayers->stackLayer()->addSpell(copy, NULL, playerMana, alternateCostType, 1);
            }
        }//end of storm
        else
        {
            ManaCost * c = spellCost->Diff(alternateCost);
            copy->X = c->getCost(Constants::NB_Colors);
            copy->castX = copy->X;
            delete c;
        }
    }

    
    return 1;
}

ostream& MTGAlternativeCostRule::toString(ostream& out) const
{
    out << "MTGAlternativeCostRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGAlternativeCostRule * MTGAlternativeCostRule::clone() const
{
    return NEW MTGAlternativeCostRule(*this);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//buyback follows its own resolving rules
MTGBuyBackRule::MTGBuyBackRule(GameObserver* observer, int _id) :
MTGAlternativeCostRule(observer, _id)
{
    aType = MTGAbility::BUYBACK_COST;
}

int MTGBuyBackRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    Player * player = game->currentlyActing();
    if (!player->game->hand->hasCard(card))
        return 0;
    if(!allowedToCast(card,player))
        return 0;
    return MTGAlternativeCostRule::isReactingToClick( card, mana, card->getManaCost()->BuyBack );
}

int MTGBuyBackRule::reactToClick(MTGCardInstance * card) 
{
    if (!isReactingToClick(card))
        return 0;

    ManaCost * alternateCost = card->getManaCost()->BuyBack;
    
    card->paymenttype = MTGAbility::BUYBACK_COST;

    return MTGAlternativeCostRule::reactToClick(card, alternateCost, ManaCost::MANA_PAID_WITH_BUYBACK);

}

ostream& MTGBuyBackRule::toString(ostream& out) const
{
    out << "MTGBuyBackRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGBuyBackRule * MTGBuyBackRule::clone() const
{
    return NEW MTGBuyBackRule(*this);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//flashback follows its own resolving rules
MTGFlashBackRule::MTGFlashBackRule(GameObserver* observer, int _id) :
MTGAlternativeCostRule(observer, _id)
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
    ManaCost * alternateCost = card->getManaCost()->FlashBack;
    
    if (!isReactingToClick(card))
        return 0;

    card->paymenttype = MTGAbility::FLASHBACK_COST;

    return MTGAlternativeCostRule::reactToClick(card, alternateCost, ManaCost::MANA_PAID_WITH_FLASHBACK);

}

ostream& MTGFlashBackRule::toString(ostream& out) const
{
    out << "MTGFlashBackRule ::: (";
    return MTGAbility::toString(out) << ")";
}
MTGFlashBackRule * MTGFlashBackRule::clone() const
{
    return NEW MTGFlashBackRule(*this);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//retrace
MTGRetraceRule::MTGRetraceRule(GameObserver* observer, int _id) :
MTGAlternativeCostRule(observer, _id)
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
    
    ManaCost * alternateCost = card->getManaCost()->Retrace;
    
    card->paymenttype = MTGAbility::RETRACE_COST;

    return MTGAlternativeCostRule::reactToClick(card, alternateCost, ManaCost::MANA_PAID_WITH_RETRACE);
}

ostream& MTGRetraceRule::toString(ostream& out) const
{
    out << "MTGRetraceRule ::: (";
    return MTGAbility::toString(out) << ")";
}
MTGRetraceRule * MTGRetraceRule::clone() const
{
    return NEW MTGRetraceRule(*this);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//Suspend
MTGSuspendRule::MTGSuspendRule(GameObserver* observer, int _id) :
MTGAlternativeCostRule(observer, _id)
{
    aType = MTGAbility::SUSPEND_COST;
}

int MTGSuspendRule::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    Player * player = game->currentlyActing();
    ManaCost * alternateManaCost = card->getManaCost()->suspend;

    if (!player->game->hand->hasCard(card) || !alternateManaCost)
        return 0;
    suspendmenu = "suspend";
    char buffer[20];
    if(alternateManaCost->hasX())
        sprintf(buffer,"- X");
    else
        sprintf(buffer,"-%i",card->suspendedTime);
    suspendmenu.append(buffer);
    return MTGAlternativeCostRule::isReactingToClick( card, mana, alternateManaCost  );
}

int MTGSuspendRule::receiveEvent(WEvent *e)
{
    if (WEventPhaseChange* event = dynamic_cast<WEventPhaseChange*>(e))
    {
        if (MTG_PHASE_UNTAP == event->from->id)
        {
            Player * p = game->currentPlayer;
            MTGGameZone * z = p->game->exile;
            int originalAmount = z->nb_cards-1;
            for (int i = originalAmount; i > -1; i--)
            {
                MTGCardInstance * card = z->cards[i];

                if (card->suspended && card->counters->hasCounter("time",0,0))
                    card->counters->removeCounter("time",0,0);
            }
            return 1;
        }
    }
    return 0;
}

int MTGSuspendRule::reactToClick(MTGCardInstance * card)
{
    if (!isReactingToClick(card))
        return 0;
    Player *player = game->currentlyActing();
    ManaCost * playerMana = player->getManaPool();
    ManaCost * alternateCost = card->getManaCost()->suspend;
    //this handles extra cost payments at the moment a card is played.
    if (playerMana->canAfford(alternateCost))
    {
        if(alternateCost->hasX())
        {
            ManaCost * checkXnotZero = NEW ManaCost(alternateCost);//suspend cards with x cost, x can not be zero.
            checkXnotZero->add(0,1);
            if (!playerMana->canAfford(checkXnotZero))
            {
                SAFE_DELETE(checkXnotZero);
                return 0;
            }
            SAFE_DELETE(checkXnotZero);
        }
        if (alternateCost->isExtraPaymentSet())
        {
            if (!game->targetListIsSet(card))
            {
                return 0;
            }
        }
        else
        {
            alternateCost->setExtraCostsAction(this, card);
            game->mExtraPayment = getCost()->suspend->extraCosts;
            return 0;
        }
            card->paymenttype = MTGAbility::SUSPEND_COST;
    }
    //------------------------------------------------------------------------
    if(card->getManaCost()->suspend->hasX())
    {
        ManaCost * pMana = NEW ManaCost(player->getManaPool());
        ManaCost * suspendCheckMana = NEW ManaCost(card->getManaCost()->suspend);
        card->suspendedTime = pMana->getConvertedCost() - suspendCheckMana->getConvertedCost();
        SAFE_DELETE(pMana);
        SAFE_DELETE(suspendCheckMana);
    }
    player->getManaPool()->pay(card->getManaCost()->suspend);
    card->getManaCost()->suspend->doPayExtra();
    //---------------------------------------------------------------------------
    player->game->putInZone(card, card->currentZone, player->game->exile);
    card->next->suspended = true;
    for(signed int i = 0; i < card->suspendedTime;i++)
    card->next->counters->addCounter("time",0,0);
    return 1;
}

const char * MTGSuspendRule::getMenuText()
{
    return suspendmenu.c_str();
}

ostream& MTGSuspendRule::toString(ostream& out) const
{
    out << "MTGSuspendRule ::: (";
    return MTGAbility::toString(out) << ")";
}
MTGSuspendRule * MTGSuspendRule::clone() const
{
    return NEW MTGSuspendRule(*this);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


MTGMorphCostRule::MTGMorphCostRule(GameObserver* observer, int _id) :
    PermanentAbility(observer, _id)
{
    aType = MTGAbility::MORPH_COST;
}
int MTGMorphCostRule::isReactingToClick(MTGCardInstance * card, ManaCost *)
{

    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;
    if (!player->game->hand->hasCard(card))
        return 0;
    if (!card->getManaCost()->morph)
        return 0;
    if(!allowedToAltCast(card,player))
        return 0;
    //note lands can morph too, this is different from other cost types.
    if ((card->hasType(Subtypes::TYPE_INSTANT)) || card->has(Constants::FLASH) || (player == currentPlayer
        && !game->isInterrupting
        && (game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
        || game->getCurrentGamePhase() == MTG_PHASE_SECONDMAIN))
        )
    {
        if (game->currentActionPlayer->game->playRestrictions->canPutIntoZone(card, game->currentActionPlayer->game->stack) == PlayRestriction::CANT_PLAY)
            return 0;
        ManaCost * playerMana = player->getManaPool();
        ManaCost * morph = card->getManaCost()->morph;

#ifdef WIN32
        ManaCost * cost = card->getManaCost();
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
    player->getManaPool()->pay(card->getManaCost()->morph);
    card->getManaCost()->morph->doPayExtra();
    int payResult = ManaCost::MANA_PAID_WITH_MORPH;
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
    spell->source->name = "Morph";
    spell->source->power = 2;
    spell->source->toughness = 2;
    copy->morphed = true;
    copy->isMorphed = true;
    copy->power = 2;
    copy->toughness = 2;
    if (!card->has(Constants::STORM))
    {
        copy->X = spell->computeX(copy);
        copy->castX = copy->X;
    }
    return 1;
}

ostream& MTGMorphCostRule::toString(ostream& out) const
{
    out << "MTGMorphCostRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGMorphCostRule * MTGMorphCostRule::clone() const
{
    return NEW MTGMorphCostRule(*this);
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
bool MTGAttackRule::greyout(Target*)
{
    return true;
}

MTGAttackRule::MTGAttackRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    aType = MTGAbility::MTG_ATTACK_RULE;
}

int MTGAttackRule::isReactingToClick(MTGCardInstance * card, ManaCost *)
{
    if (currentPhase == MTG_PHASE_COMBATATTACKERS && card->controller() == game->currentPlayer && card->controller() == game->currentlyActing())//on my turn and when I am the acting player.
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
        if (MTG_PHASE_COMBATATTACKERS == event->from->id)
        {
            Player * p = game->currentPlayer;
            MTGGameZone * z = p->game->inPlay;
            for (int i = 0; i < z->nb_cards; i++)
            {
                MTGCardInstance * card = z->cards[i];
                if (!card->isAttacker() && !event->from->isExtra && card->has(Constants::MUSTATTACK))//cards are only required to attack in the real attack phase of a turn.
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
        game->getCardSelector()->PushLimitor();
        game->getCardSelector()->Limit(this, CardView::playZone);
        game->getCardSelector()->CheckUserInput(JGE_BTN_RIGHT);
        game->getCardSelector()->Limit(NULL, CardView::playZone);
        game->getCardSelector()->PopLimitor();
    }
    card->toggleAttacker();
    return 1;
}

ostream& MTGAttackRule::toString(ostream& out) const
{
    out << "MTGAttackRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGAttackRule * MTGAttackRule::clone() const
{
    return NEW MTGAttackRule(*this);
}
//handling for planeswalker attacking choice
MTGPlaneswalkerAttackRule::MTGPlaneswalkerAttackRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    aType = MTGAbility::MTG_ATTACK_RULE;
}

int MTGPlaneswalkerAttackRule::isReactingToClick(MTGCardInstance * card, ManaCost *)
{
    if (currentPhase == MTG_PHASE_COMBATATTACKERS && card->controller() == game->currentPlayer && card->controller() == game->currentlyActing())//on my turn and when I am the acting player.
    {
        if(!card->controller()->opponent()->game->inPlay->hasType("planeswalker"))
            return 0;
        if(card->isPhased)
            return 0;
        if (card->isAttacker())
            return 1;
        if (card->canAttack())
            return 1;
    }
    return 0;
}

int MTGPlaneswalkerAttackRule::reactToClick(MTGCardInstance * card)
{
    if (!isReactingToClick(card))
        return 0;
    //Graphically select the next card that can attack
    if (!card->isAttacker())
    {
        game->getCardSelector()->PushLimitor();
        game->getCardSelector()->Limit(this, CardView::playZone);
        game->getCardSelector()->CheckUserInput(JGE_BTN_RIGHT);
        game->getCardSelector()->Limit(NULL, CardView::playZone);
        game->getCardSelector()->PopLimitor();
    }

    vector<MTGAbility*>selection;
    MTGCardInstance * check = NULL;
    int checkWalkers = card->controller()->opponent()->game->battlefield->cards.size();
    for(int i = 0; i < checkWalkers;++i)
    {
        check = card->controller()->opponent()->game->battlefield->cards[i];
        if(check->hasType(Subtypes::TYPE_PLANESWALKER))
        {
            MTGAbility * setPw = NEW AAPlaneswalkerAttacked(game, game->mLayers->actionLayer()->getMaxId(), card,check);
            MTGAbility * setWalker = setPw->clone();
            setWalker->oneShot = true;
            selection.push_back(setWalker);
            SAFE_DELETE(setPw);
        }
    }


    if(selection.size())
    {
        MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), card, card,false,selection);
        game->mLayers->actionLayer()->currentActionCard = card;
        a1->resolve();
    }

    return 1;
}

MTGPlaneswalkerAttackRule * MTGPlaneswalkerAttackRule::clone() const
{
    return NEW MTGPlaneswalkerAttackRule(*this);
}

bool MTGPlaneswalkerAttackRule::select(Target* t)
{
    if (CardView* c = dynamic_cast<CardView*>(t))
    {
        MTGCardInstance * card = c->getCard();
        if (card->canAttack() && !card->isPhased)
            return true;
    }
    return false;
}
bool MTGPlaneswalkerAttackRule::greyout(Target*)
{
    return true;
}

//setting combat against planeswalker menu handling
 AAPlaneswalkerAttacked::AAPlaneswalkerAttacked(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target):
    InstantAbility(observer, id, source)
{
    this->target = _target;
    menuText = _target->name.c_str();
    attacker = card;
}

int AAPlaneswalkerAttacked::resolve()
{
    if(!attacker)
        return 0;
    attacker->isAttacking = this->target;
    attacker->toggleAttacker();
    return 1;
}

const char* AAPlaneswalkerAttacked::getMenuText()
{
    return menuText.c_str();
}

AAPlaneswalkerAttacked * AAPlaneswalkerAttacked::clone() const
{
    return NEW AAPlaneswalkerAttacked(*this);
}

AAPlaneswalkerAttacked::~AAPlaneswalkerAttacked()
{
}

//this rules handles returning cards to combat triggers for activations.
MTGCombatTriggersRule::MTGCombatTriggersRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    aType = MTGAbility::MTG_COMBATTRIGGERS_RULE;
}

int MTGCombatTriggersRule::receiveEvent(WEvent *e)
{
    if (WEventPhaseChange* event = dynamic_cast<WEventPhaseChange*>(e))
    {
        if (MTG_PHASE_COMBATATTACKERS == event->from->id)
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
        if (MTG_PHASE_COMBATEND == event->from->id)
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
        MTGGameZone* opponentZone = game->currentPlayer->opponent()->game->inPlay;
        for (int i = 0; i < opponentZone->nb_cards; i++)
        {
            MTGCardInstance* card = opponentZone->cards[i];
            if (card && card->didblocked)
            {
                card->eventblocked(card->getNextOpponent());
            }
        }
        Player * p = game->currentPlayer;
        MTGGameZone * z = p->game->inPlay;
        for (int i = 0; i < z->nb_cards; i++)
        {
            MTGCardInstance * card = z->cards[i];
            if (card && card->isAttacker() && !card->isBlocked())
            {
                card->eventattackednotblocked();
                card->notblocked = 1;
            }
            if (card && card->isAttacker() && card->isBlocked())
            {

                MTGCardInstance * opponent = card->getNextOpponent();
                while (opponent)
                {
                    card->eventattackedblocked(opponent);
                    opponent = card->getNextOpponent(opponent);
                }

            }
        }
    }
    return 0;
}

ostream& MTGCombatTriggersRule::toString(ostream& out) const
{
    out << "MTGCombatTriggersRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGCombatTriggersRule * MTGCombatTriggersRule::clone() const
{
    return NEW MTGCombatTriggersRule(*this);
}
///------------

OtherAbilitiesEventReceiver::OtherAbilitiesEventReceiver(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
}

int OtherAbilitiesEventReceiver::receiveEvent(WEvent *e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    {
        if (event->to && (event->to != event->from))
        {
            for (int i = 0; i < 2; ++i)
            {
                if (event->to == game->players[i]->game->inPlay)
                    return 0;
            }
            AbilityFactory af(game);
            af.magicText(game->mLayers->actionLayer()->getMaxId(), NULL, event->card, 1, 0, event->to);
            return 1;
        }
    }
    return 0;
}

OtherAbilitiesEventReceiver * OtherAbilitiesEventReceiver::clone() const
{
    return NEW OtherAbilitiesEventReceiver(*this);
}

MTGBlockRule::MTGBlockRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    aType = MTGAbility::MTG_BLOCK_RULE;
    tcb = NULL;
    blocker = NULL;
    blockAbility = NULL;
}

int MTGBlockRule::receiveEvent(WEvent *e)
{
    if (dynamic_cast<WEventBlockersChosen*>(e))
    {

        Player * p = game->currentPlayer;
        MTGCardInstance * lurer = p->game->inPlay->findALurer();
        if(lurer)
        {
            MTGGameZone * z = p->opponent()->game->inPlay;
            for (int i = 0; i < z->nb_cards; i++)
            {
                MTGCardInstance * card = z->cards[i];
                if ((card->defenser && !card->defenser->has(Constants::LURE))||!card->defenser)
                    if(card->canBlock(lurer))
                        card->setDefenser(lurer);
                //force a block on a lurer, the player has a chance to set his own choice on multiple lures
                //but this action can not be ignored.
            }
        }
        return 1;

    }
    return 0;
}

int MTGBlockRule::isReactingToClick(MTGCardInstance * card, ManaCost *)
{
    if (currentPhase == MTG_PHASE_COMBATBLOCKERS && !game->isInterrupting
        && card->controller() != game->currentPlayer
        )
    {
        if (card->canBlock() && !card->isPhased)
        {
            if(card->isDefenser())
                blockmenu = "Remove Blocker";
            else
                blockmenu = "Assign To Block";
            return 1;
        }
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
    if(card->controller()->isHuman())
    {
        if(!currentOpponent)
        {

            SAFE_DELETE(blockAbility);
            TargetChooserFactory tf(card->getObserver());
            tcb = tf.createTargetChooser("blockable",card);
            tcb->targetter = NULL;
            blocker = NEW AABlock(card->getObserver(),-1,card,NULL);
            blocker->oneShot = true;
            blocker->forceDestroy = 1;
            blocker->canBeInterrupted = false;
            blockAbility = NEW GenericTargetAbility(game, "choose attacker","blockers",-1, card,tcb, blocker);
            blockAbility->oneShot = true;
            blockAbility->forceDestroy = 1;
            blockAbility->canBeInterrupted = false;
            return blockAbility->reactToTargetClick(card);
        }
        else
            card->toggleDefenser(NULL);
    }
    else
    {
        bool lured = false;
        MTGCardInstance * lurers = game->currentPlayer->game->inPlay->findALurer();
        if(lurers)
        {
            lured = true;
        }
        while (!result)
        {
            currentOpponent = game->currentPlayer->game->inPlay->getNextAttacker(currentOpponent);

            if(lured && currentOpponent && !currentOpponent->has(Constants::LURE))
                currentOpponent = game->currentPlayer->game->inPlay->getNextLurer(currentOpponent);
                canDefend = card->toggleDefenser(currentOpponent);

            DebugTrace("Defenser Toggle: " << card->getName() << endl
                << "- canDefend: " << (canDefend == 0) << endl
                << "- currentOpponent: " << currentOpponent);
            result = (currentOpponent == NULL || canDefend);
        }
        lured = false;
    }
    return 1;
}

const char * MTGBlockRule::getMenuText()
{
    return blockmenu.c_str();
}

ostream& MTGBlockRule::toString(ostream& out) const
{
    out << "MTGBlockRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGBlockRule * MTGBlockRule::clone() const
{
    return NEW MTGBlockRule(*this);
}

MTGBlockRule::~MTGBlockRule()
{
    SAFE_DELETE(blockAbility);
}

//
// Attacker chooses blockers order
//

//
// * Momir
//

MTGMomirRule::MTGMomirRule(GameObserver* observer, int _id, MTGAllCards * _collection) :
    PermanentAbility(observer, _id), initialized(false)
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

int MTGMomirRule::isReactingToClick(MTGCardInstance * card, ManaCost *)
{
    if (alreadyplayed)
        return 0;
    Player * player = game->currentlyActing();
    Player * currentPlayer = game->currentPlayer;
    if (!player->game->hand->hasCard(card))
        return 0;
    if (player == currentPlayer && !game->isInterrupting
        && (game->getCurrentGamePhase() == MTG_PHASE_FIRSTMAIN
        || game->getCurrentGamePhase() == MTG_PHASE_SECONDMAIN)
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
    Spell * spell = NEW Spell(game, card);
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
    int start = (game->getRandomGenerator()->random() % total_cards);
    return pool[convertedCost][start];
}

void MTGMomirRule::Update(float dt)
{
    if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP)
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
    return NEW MTGMomirRule(*this);
}

//stone hewer game mode
//in stonehewer when ever a creature enters the battlefield
//it enters play with a equipment choosen at random with a converted manacost
//less than or equal to the creature.
//note this can kill your creature if the equipment contains negitive toughness

MTGStoneHewerRule::MTGStoneHewerRule(GameObserver* observer, int _id, MTGAllCards * _collection) :
    PermanentAbility(observer, _id), initialized(false)
{
    collection = _collection;
    if (!initialized)
    {
        for (size_t i = 0; i < collection->ids.size(); i++)
        {
            MTGCard * card = collection->collection[collection->ids[i]];
			if (card->data->hasSubtype("equipment") && (card->getRarity() != Constants::RARITY_T) && //remove tokens
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
}

int MTGStoneHewerRule::receiveEvent(WEvent * event)
{
	WEventZoneChange * e = (WEventZoneChange *) event;
	if (e->to == game->currentlyActing()->game->inPlay && e->card->isCreature())
	{
		int eId = genRandomEquipId(e->card->getManaCost()->getConvertedCost());
		MTGCardInstance * card = genEquip(eId);
		if(card)
		{
			game->currentlyActing()->game->temp->addCard(card);
                        Spell * spell = NEW Spell(game, card);
			spell->resolve();
			spell->source->isToken = 1;
                        for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
			{
                                MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
				AEquip * eq = dynamic_cast<AEquip*> (a);
				if (eq && eq->source == spell->source)
				{
					((AEquip*)a)->unequip();
					((AEquip*)a)->equip(e->card);
				}
			}
			SAFE_DELETE(spell);
		}
		return 1;
	}
	return 0;
}

MTGCardInstance * MTGStoneHewerRule::genEquip(int id)
{
    if (!id)
        return NULL;
    Player * p = game->currentlyActing();
    MTGCard * card = collection->getCardById(id);
    return NEW MTGCardInstance(card, p->game);
}

int MTGStoneHewerRule::genRandomEquipId(int convertedCost)
{
	if (convertedCost >= 20)
		convertedCost = 19;
    int total_cards = 0;
    int i = (game->getRandomGenerator()->random() % int(convertedCost+1));//+1 becuase we want to generate a random "<=" the coverted.
    while (!total_cards && i >= 0)
    {
        total_cards = pool[i].size();
        convertedCost = i;
        i--;
    }
    if (!total_cards)
        return 0;
    int start = (game->getRandomGenerator()->random() % total_cards);
    return pool[convertedCost][start];
}

ostream& MTGStoneHewerRule::toString(ostream& out) const
{
    out << "MTGStoneHewerRule ::: pool : " << pool << " ; initialized : " << initialized 
        << " ; collection : " << collection << "(";
    return MTGAbility::toString(out) << ")";
}

MTGStoneHewerRule * MTGStoneHewerRule::clone() const
{
    return NEW MTGStoneHewerRule(*this);
}

//------------------
//Hermit druid mode places a random land from your deck into play during each of your upkeeps
MTGHermitRule::MTGHermitRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
}

int MTGHermitRule::receiveEvent(WEvent * event)
{
	WEventPhaseChange * e = dynamic_cast<WEventPhaseChange*>(event);
        if (e && e->from->id == MTG_PHASE_UNTAP)
	{
		MTGCardInstance * lcard = NULL;
		vector<MTGCardInstance*>lands = vector<MTGCardInstance*>();
		for(int i = 0; i < game->currentPlayer->game->library->nb_cards-1; i++)
		{
			MTGCardInstance * temp = game->currentPlayer->game->library->cards[i];
			if(temp && temp->isLand())
				lands.push_back(temp);
		}
		if(lands.size())
                        lcard = lands[game->getRandomGenerator()->random() % lands.size()];
		if(lcard)
		{
			MTGCardInstance * copy = game->currentPlayer->game->putInZone(lcard,game->currentPlayer->game->library, game->currentPlayer->game->temp);
            Spell * spell = NEW Spell(game, copy);
            spell->resolve();
            delete spell;
		}
		return 1;
	}
	return 0;
}

MTGHermitRule * MTGHermitRule::clone() const
{
    return NEW MTGHermitRule(*this);
}
//--------------------

//HUDDisplay

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
HUDDisplay::HUDDisplay(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
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
    return NEW HUDDisplay(*this);
}

/* soulbond */
MTGSoulbondRule::MTGSoulbondRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    tcb = NULL;
    pairAbility = NULL;
    targetAbility = NULL;
    mod = NULL;
}
;

int MTGSoulbondRule::receiveEvent(WEvent * event)
{
    if (event->type == WEvent::CHANGE_ZONE)
    {
        WEventZoneChange * e = (WEventZoneChange *) event;
        MTGCardInstance * card = e->card;
        if (!card || !card->isCreature()) return 0;
        int ok = 0;
        if(card->has(Constants::soulbond) || soulbonders.size())
        {
            for (int i = 0; i < 2; i++)
            {
                Player * p = game->players[i];
                if (e->to == p->game->inPlay)
                {
                    ok = 1;
                    if(card->basicAbilities[(int)Constants::soulbond])
                        soulbonders.push_back(e->card);
                }
            }
            if(!soulbonders.size())
                ok = 0;
            else
            {
                MTGCardInstance * pairable = NULL;
                for(unsigned int k = 0;k < soulbonders.size();k++)
                {
                    MTGCardInstance * check = soulbonders[k];
                    if(check->controller() == e->card->controller())
                    {
                        if(!check->myPair)
                        {
                            if(check != card)
                                pairable = check;
                            else
                            {
                                MTGInPlay * zone = check->controller()->game->battlefield;
                                for(unsigned int d = 0;d < zone->cards.size();++d)
                                {
                                    if(zone->cards[d]->isCreature() && !zone->cards[d]->myPair && zone->cards[d] != check)
                                    {
                                        pairable = check;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                }
                if(!pairable)
                    ok = 0;
            }
            if (!ok)
                return 0;

            for (int i = 0; i < 2; i++)
            {
                Player * p = game->players[i];
                if (e->to == p->game->inPlay)
                {
                    TargetChooserFactory tf(card->getObserver());
                    tcb = tf.createTargetChooser("pairable",card);
                    tcb->targetter = NULL;
                    pairAbility = NEW PairCard(game, game->mLayers->actionLayer()->getMaxId(), card,NULL);
                    targetAbility = NEW GenericTargetAbility(game, "Pair Creature","",game->mLayers->actionLayer()->getMaxId(), card,tcb,pairAbility);
                    targetAbility1 = NEW MayAbility(game,game->mLayers->actionLayer()->getMaxId(),targetAbility,card,false);
                    activatePairing = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), card,NULL,targetAbility1);
                    //this next line is ugly, but fixes a long running memleak which seems to be unfixable while maintaining the same order of activation.
                    game->mLayers->actionLayer()->garbage.push_back(activatePairing);
                    activatePairing->fireAbility();
                    
                    return 1;
                }
            }
        }
    }
    return 0;
}

ostream& MTGSoulbondRule::toString(ostream& out) const
{
    out << "MTGSoulbondRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGSoulbondRule * MTGSoulbondRule::clone() const
{
    return NEW MTGSoulbondRule(*this);
}
/*dredge*/
MTGDredgeRule::MTGDredgeRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
    tcb = NULL;
    dredgeAbility = NULL;
    targetAbility = NULL;
    mod = NULL;
}
;

WEvent * MTGDredgeRule::replace(WEvent * event)
{
    WEventDraw * e = dynamic_cast<WEventDraw*> (event);
    if (e)
    {
        MTGCardInstance * card = NULL;
        if(e->player->game->library->nb_cards)
        card = e->player->game->library->cards[e->player->game->library->nb_cards-1];
        if(!card)
            return event;
        TargetChooserFactory tf(e->player->getObserver());
        tcb = tf.createTargetChooser("dredgeable",card);
        tcb->targetter = NULL;
        if(!tcb->validTargetsExist())
        {
            SAFE_DELETE(tcb);
            return event;
        }
        SAFE_DELETE(tcb);
        for (int i = 0; i < 2; i++)
        {
            Player * p = game->players[i];
            if (e->player == p)
            {
                for(int draw = 0;draw < e->nb_cards;draw++)
                {
                    tcb = tf.createTargetChooser("dredgeable",card);
                    tcb->targetter = NULL;

                    vector<MTGAbility*>selection;
                    //look for other draw replacement effects
                    list<ReplacementEffect *>::iterator it;
                    for (it = game->replacementEffects->modifiers.begin(); it != game->replacementEffects->modifiers.end(); it++)
                    {
                        if(REDrawReplacement * DR = dynamic_cast<REDrawReplacement *>(*it))
                        {
                            MTGAbility * otherA = NULL;
                            if(DR->DrawerOfCard == p)
                            {
                                if(DR->replacementAbility->oneShot)
                                {
                                    selection.push_back(DR->replacementAbility->clone());
                                }
                                else
                                {
                                    otherA = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(),DR->replacementAbility->source,NULL,DR->replacementAbility->clone());
                                    selection.push_back(otherA);
                                }
                            }
                        }
                    }

                    //there is a memleak here that i have no idea what causes it.
                    dredgeAbility = NEW dredgeCard(game, game->mLayers->actionLayer()->getMaxId(), card,NULL);
                    dredgeAbility->oneShot = true;
                    targetAbility = NEW GenericTargetAbility(game, "Dredge A Card","",game->mLayers->actionLayer()->getMaxId(), card,tcb->clone(),dredgeAbility->clone());
                    targetAbility->oneShot = true;
                    SAFE_DELETE(dredgeAbility);

                    targetAbilityAdder = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), card,NULL,targetAbility->clone());
                    targetAbilityAdder->oneShot = true;
                    SAFE_DELETE(targetAbility);
                    MTGAbility * setDredge = targetAbilityAdder->clone();
                    SAFE_DELETE(targetAbilityAdder);
                    setDredge->oneShot = true;

                    selection.push_back(setDredge);
                    targetAbility1 = NEW AADrawer(game, this->GetId(), card,card,NULL, "1",TargetChooser::CONTROLLER,true);
                    selection.push_back(targetAbility1);
                    MTGAbility * menuChoice = NEW MenuAbility(game, this->GetId(), card, card,true,selection,card->controller(),"Dredge or Draw");

                    menuChoice->addToGame();
                    SAFE_DELETE(tcb);
                }
                
                SAFE_DELETE(event);
                return event;
            }
        }
    }
    return event;
}

ostream& MTGDredgeRule::toString(ostream& out) const
{
    out << "MTGDredgeRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGDredgeRule * MTGDredgeRule::clone() const
{
    return NEW MTGDredgeRule(*this);
}
/* Persist */
MTGPersistRule::MTGPersistRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
}
;

int MTGPersistRule::receiveEvent(WEvent * event)
{
    if (event->type == WEvent::CHANGE_ZONE)
    {
        WEventZoneChange * e = (WEventZoneChange *) event;
        MTGCardInstance * card = e->card->previous;
        if (!card) return 0;
        int ok = 0;
        if((card->basicAbilities[(int)Constants::PERSIST] && !card->counters->hasCounter(-1, -1))||(card->basicAbilities[(int)Constants::UNDYING] && !card->counters->hasCounter(1, 1)))
        {
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
                    MTGCardInstance * copy = p->game->putInZone(e->card, p->game->graveyard, e->card->owner->game->temp);
                    if (!copy)
                    {
                        DebugTrace("MTGRULES: couldn't move card for persist/undying");
                        return 0;
                    }
                    Spell * spell = NEW Spell(game, copy);
                    spell->resolve();
                    if(card->basicAbilities[(int)Constants::PERSIST])
                        spell->source->counters->addCounter(-1, -1);
                    else
                        spell->source->counters->addCounter(1,1);
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

MTGPersistRule * MTGPersistRule::clone() const
{
    return NEW MTGPersistRule(*this);
}

//vampires rule
//handled seperately as a rule since we only want one object to send out events that a card was "vampired".
//otherwise vampire event is sent per instance of @vampired on the battlefield, multipling the results.
MTGVampireRule::MTGVampireRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
}
;

int MTGVampireRule::receiveEvent(WEvent * event)
{
    if (WEventDamage * e = dynamic_cast<WEventDamage *> (event))
    {
        if(!e->damage->damage)
            return 0;
        if (!e->damage->target) 
            return 0;

        MTGCardInstance * newVictim = dynamic_cast<MTGCardInstance*>(e->damage->target);
        if(!newVictim)
            return 0;
        
        MTGCardInstance * vampire = (MTGCardInstance*)(e->damage->source);

        victims[newVictim].push_back(vampire);

    }
    else if ( WEventZoneChange * z = dynamic_cast<WEventZoneChange *> (event))
    {
        MTGCardInstance * card = z->card->previous;
        if(card && victims[card].empty())
            return 0;

        //sort and remove duplicates, we only want one event of a vampire damaging a card stored per victem.
        std::sort(victims[card].begin(), victims[card].end());
        victims[card].erase(std::unique(victims[card].begin(), victims[card].end()), victims[card].end());

        for(unsigned int w = 0;w < victims[card].size();w++)
        {
            if(victims[card].at(w) == NULL) 
                continue;
            Player * p = card->controller();
            if (z->from == p->game->inPlay && z->to == p->game->graveyard)
            {
                if(card == z->card->previous)
                {
                    WEvent * e = NEW WEventVampire(card,victims[card].at(w),card);
                    game->receiveEvent(e);
                }
            }
        }
        return 0;
    }
    else if (WEventPhaseChange * pe = dynamic_cast<WEventPhaseChange*>(event))
    {
        if( pe->from->id == MTG_PHASE_ENDOFTURN)
        {
            victims.clear();
        }
    }
    return 0;
}

ostream& MTGVampireRule::toString(ostream& out) const
{
    out << "MTGVampireRule ::: (";
    return MTGAbility::toString(out) << ")";
}

MTGVampireRule * MTGVampireRule::clone() const
{
    return NEW MTGVampireRule(*this);
}
/////////////////////////////////////////////////
//unearth rule----------------------------------
//if the card leaves play, exile it instead.
MTGUnearthRule::MTGUnearthRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
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

        if (card && card->basicAbilities[(int)Constants::UNEARTH])
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

MTGUnearthRule * MTGUnearthRule::clone() const
{
    return NEW MTGUnearthRule(*this);
}
//token clean up
MTGTokensCleanup::MTGTokensCleanup(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
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

MTGTokensCleanup * MTGTokensCleanup::clone() const
{
    return NEW MTGTokensCleanup(*this);
}

/* Legend Rule */
MTGLegendRule::MTGLegendRule(GameObserver* observer, int _id) :
ListMaintainerAbility(observer, _id)
{
}
;

int MTGLegendRule::canBeInList(MTGCardInstance * card)
{
    if(card->isPhased)
        return 0;
    if (card->hasType(Subtypes::TYPE_LEGENDARY) && card->controller()->game->inPlay->hasCard(card))
    {
        if(card->has(Constants::NOLEGEND))
            return 0;
        else
            return 1;
    }
    return 0;
}

int MTGLegendRule::added(MTGCardInstance * card)
{
    map<MTGCardInstance *, bool>::iterator it;
    int destroy = 0;

    vector<MTGCardInstance*>oldCards;
    for (it = cards.begin(); it != cards.end(); it++)
    {
        MTGCardInstance * comparison = (*it).first;
        if (comparison != card && comparison->controller() == card->controller() && !(comparison->getName().compare(card->getName())))
        {
            oldCards.push_back(comparison);
            destroy = 1;
        }
    }
    if(destroy)
    {
        vector<MTGAbility*>selection;
        MultiAbility * multi = NEW MultiAbility(game, game->mLayers->actionLayer()->getMaxId(), card, card, NULL);
        for(unsigned int i = 0;i < oldCards.size();i++)
        {
            AABuryCard *a = NEW AABuryCard(game, game->mLayers->actionLayer()->getMaxId(), card, oldCards[i]);
            a->menu = "Keep New";
            a->oneShot = true;
            multi->Add(a);
        }
        multi->oneShot = 1;
        MTGAbility * a1 = multi;
        selection.push_back(a1);
        AABuryCard *b = NEW AABuryCard(game, game->mLayers->actionLayer()->getMaxId(), card, card);
        b->menu = "Keep Old";
        b->oneShot = true;
        MTGAbility * b1 = b;
        selection.push_back(b1);
        MTGAbility * menuChoice = NEW MenuAbility(game, game->mLayers->actionLayer()->getMaxId(), card, card,true,selection,card->controller(),"Choose Legend");
        menuChoice->addToGame();
    }
    return 1;
}

int MTGLegendRule::removed(MTGCardInstance *)
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
    return NEW MTGLegendRule(*this);
}

/* PlaneWalker Rule */
MTGPlaneWalkerRule::MTGPlaneWalkerRule(GameObserver* observer, int _id) :
ListMaintainerAbility(observer, _id)
{
}
;

int MTGPlaneWalkerRule::canBeInList(MTGCardInstance * card)
{
    if(card->isPhased)
        return 0;
    if (card->hasType(Subtypes::TYPE_PLANESWALKER) && card->controller()->game->inPlay->hasCard(card))
    {
        return 1;
    }
    return 0;
}

int MTGPlaneWalkerRule::added(MTGCardInstance * card)
{
    map<MTGCardInstance *, bool>::iterator it;
    int destroy = 0;
    vector<MTGCardInstance*>oldCards;
    for (it = cards.begin(); it != cards.end(); it++)
    {
        MTGCardInstance * comparison = (*it).first;
        if (comparison != card && comparison->types == card->types && comparison->controller() == card->controller())
        {
            oldCards.push_back(comparison);
            destroy = 1;
        }
    }
    if (destroy)
    {
        vector<MTGAbility*>selection;

        MultiAbility * multi = NEW MultiAbility(game,game->mLayers->actionLayer()->getMaxId(), card, card, NULL);
        for(unsigned int i = 0;i < oldCards.size();i++)
        {
            AABuryCard *a = NEW AABuryCard(game, game->mLayers->actionLayer()->getMaxId(), card, oldCards[i]);
            a->menu = "Keep New";
            a->oneShot = true;
            multi->Add(a);
        }
        multi->oneShot = 1;
        MTGAbility * a1 = multi;
        selection.push_back(a1);
        AABuryCard *b = NEW AABuryCard(game, game->mLayers->actionLayer()->getMaxId(), card, card);
        b->menu = "Keep Old";
        b->oneShot = true;
        MTGAbility * b1 = b;
        selection.push_back(b1);
        MTGAbility * menuChoice = NEW MenuAbility(game, game->mLayers->actionLayer()->getMaxId(), card, card,true,selection,card->controller(),"Choose Planeswalker");
        menuChoice->addToGame();
    }
    return 1;
}

int MTGPlaneWalkerRule::removed(MTGCardInstance *)
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
    return NEW MTGPlaneWalkerRule(*this);
}
/* planeswalker damage rule */
MTGPlaneswalkerDamage::MTGPlaneswalkerDamage(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
}
;

int MTGPlaneswalkerDamage::receiveEvent(WEvent * event)
{
    
    if (event->type == WEvent::DAMAGE)
    {
        WEventDamage * e = (WEventDamage *) event;
        Damage * d = e->damage;
        MTGCardInstance * card = dynamic_cast<MTGCardInstance*>(e->getTarget(WEvent::TARGET_TO));
        if (d->damage > 0 && card && card->hasType(Subtypes::TYPE_PLANESWALKER))
        {
            int howMany = d->damage;
            for(int k = 0;k < howMany;k++)
            {
                card->counters->removeCounter("loyalty",0,0);
            }
            d->damage = 0;
            return 1;
        }
    }
    if (WEventCounters * removel = dynamic_cast<WEventCounters*>(event))
    {
        if(removel->removed && removel->targetCard && removel->targetCard->hasType(Subtypes::TYPE_PLANESWALKER))
            if(!removel->targetCard->counters->hasCounter("loyalty",0,0))
            {
                removel->targetCard->bury();
                return 1;
            }

    }
    return 0;
}

MTGPlaneswalkerDamage * MTGPlaneswalkerDamage::clone() const
{
    return NEW MTGPlaneswalkerDamage(*this);
}

/* Lifelink */
MTGLifelinkRule::MTGLifelinkRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
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
        if (d->damage > 0 && card && card->basicAbilities[(int)Constants::LIFELINK])
        {
            card->controller()->gainLife(d->damage);
            return 1;
        }
    }
    return 0;
}

ostream& MTGLifelinkRule::toString(ostream& out) const
{
    out << "MTGLifelinkRule ::: (";
    return MTGAbility::toString(out) << ")";
}
MTGLifelinkRule * MTGLifelinkRule::clone() const
{
    return NEW MTGLifelinkRule(*this);
}

/* Deathtouch */
MTGDeathtouchRule::MTGDeathtouchRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
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

        if (card->basicAbilities[(int)Constants::DEATHTOUCH])
        {
            _target->destroy();
            return 1;
        }
    }
    return 0;
}

MTGDeathtouchRule * MTGDeathtouchRule::clone() const
{
    return NEW MTGDeathtouchRule(*this);
}
//
//kai mod
ParentChildRule::ParentChildRule(GameObserver* observer, int _id) :
PermanentAbility(observer, _id)
{
}
;

int ParentChildRule::receiveEvent(WEvent * event)
{
    WEventZoneChange * z = dynamic_cast<WEventZoneChange *> (event);
    if (z)
    {
        MTGCardInstance * card = z->card->previous;
        if(!card)
        {
            return 0;
        }
        Player * p = card->controller();
        if (z->from == p->game->inPlay)
        {
            for(size_t myChildCheck = 0;myChildCheck < card->parentCards.size();myChildCheck++)
            {
                MTGCardInstance * pCard = card->parentCards[myChildCheck];
                for(size_t myC = 0;myC < pCard->childrenCards.size();myC++)
                {
                    if(pCard->childrenCards[myC] == card)
                    {
                        pCard->childrenCards.erase(pCard->childrenCards.begin() + myC);
                    }
                }
            }

            for(size_t w = 0;w < card->childrenCards.size();w++)
            {
                MTGCardInstance * child = card->childrenCards[w];
                if(child == NULL) 
                    continue;
                if(child->parentCards.size() < 2)
                    child->controller()->game->putInGraveyard(child);
                else//allows a card to declare 2 homes, as long as it has a home it can stay inplay.
                {
                    for(size_t myParent = 0;myParent < child->parentCards.size();myParent++)
                    {
                        if(child->parentCards[myParent] == card)
                        {
                            child->parentCards.erase(child->parentCards.begin() + myParent);
                        }
                    }
                }
            }
        }
        return 1;
    }
    return 0;
}

ostream& ParentChildRule::toString(ostream& out) const
{
    out << "ParentChildRule ::: (";
    return MTGAbility::toString(out) << ")";
}

ParentChildRule * ParentChildRule::clone() const
{
    return NEW ParentChildRule(*this);
}
