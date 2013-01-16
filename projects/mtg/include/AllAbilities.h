#ifndef _CARDS_H_
#define _CARDS_H_

#include "DebugRoutines.h" 
#include "MTGAbility.h"
#include "ManaCost.h"
#include "CardDescriptor.h"
#include "AIPlayer.h"
#include "CardDisplay.h"
#include "Subtypes.h"
#include "CardGui.h"
#include "Token.h"
#include "Counters.h"
#include "WEvent.h"
#include "GuiStatic.h"
#include "GameObserver.h"
#include "Subtypes.h"
#include "ThisDescriptor.h"
#include <JGui.h>
#include <hge/hgeparticle.h>
#include "IconButton.h"

#include <map>
using std::map;

//
// Misc classes
//
class MTGEventText: public MTGAbility
{
public:
    int textAlpha;
    string text;
    void Update(float dt);
    void Render();
    MTGEventText(GameObserver* observer, int _id,MTGCardInstance * card, string text);
    virtual MTGEventText * clone() const;
};

class WParsedInt
{
public:
    int intValue;

    int computeX(Spell * spell, MTGCardInstance * card)
    {
        if (spell) return spell->computeX(card);
        if (card) return card->X;
        return 1; //this should only hapen when the ai calls the ability. This is to give it an idea of the "direction" of X (positive/negative)
    }

private:
    void init(string s, Spell * spell, MTGCardInstance * card)
    {
        if(!s.size())
            return;
        if(!card)
            return;
        MTGCardInstance * target = card->target;
        if(!card->storedCard)
            card->storedCard = card->storedSourceCard;
        intValue = 0;
        bool halfup = false;
        bool halfdown = false;
        if (!target) target = card;
        int multiplier = 1;
        if (s[0] == '-')
        {
            s = s.substr(1);
            multiplier = -1;
            if(s.find("stored") != string::npos)
            {
                string altered ="-";
                altered.append(s.substr(+6));
                return init(altered,spell,card->storedCard);
            }
        }
        if(s[0] == '+')
        {
            //ignore "+" signs....
            s = s.substr(1);
        }
        if(s.find("stored") != string::npos)
        {
            return init(s.substr(+6),spell,card->storedCard);
        }
        //rounding values, the words can be written anywhere in the line,
        //they are erased after parsing.
        if(s.find("halfup") != string::npos)
        {
            halfup = true;
            size_t hU = s.find("halfup");
            s.erase(hU,hU + 6);
        }
        if(s.find("halfdown") != string::npos)
        {
            halfdown = true;
            size_t hD = s.find("halfdown");
            s.erase(hD,hD + 8);
        }
        if(s == "prex")
        {
            ManaCost * cX = card->controller()->getManaPool()->Diff(card->getManaCost());
            intValue = cX->getCost(Constants::NB_Colors);
            delete cX;
        }
        else if (s == "x" || s == "X")
        {
            intValue = computeX(spell, card);
            if(intValue < 0)
                intValue = 0;
        }
        else if (s == "xx" || s == "XX")
        {
            intValue = computeX(spell, card) / 2;
            if(intValue < 0)
                intValue = 0;
        }
        else if (s == "castx")
        {
            intValue = card->castX;
        }
        else if (s == "gear")
        {
            intValue = target->equipment;
        }
        else if (s == "colors")
        {
            intValue = target->countColors();
        }
        else if (s == "auras")
        {
            intValue = target->auras;
        }
        else if (s == "manacost")
        {
            intValue = target->getManaCost()->getConvertedCost();
        }
        else if (s.find("type:") != string::npos)
        {
            size_t begins = s.find("type:");
            string theType = s.substr(begins + 5);
            size_t zoned = theType.find(":");
            if(zoned == string::npos)
            {
                theType.append("|mybattlefield");
            }
            else
            {
            replace(theType.begin(), theType.end(), ':', '|');
            }
            TargetChooserFactory tf(card->getObserver());
            TargetChooser * tc = tf.createTargetChooser(theType.c_str(),NULL);
            for (int i = 0; i < 2; i++)
            {
                Player * p = card->getObserver()->players[i];
                MTGGameZone * zones[] = { p->game->inPlay, p->game->graveyard, p->game->hand, p->game->library };
                for (int k = 0; k < 4; k++)
                {
                    MTGGameZone * zone = zones[k];
                    if(tc->targetsZone(zone,target))
                    intValue += zone->countByCanTarget(tc);
                }
            }
            SAFE_DELETE(tc);
        }
        else if (s.find("counter{") != string::npos)
        {
            intValue = 0;
            vector<string>counterName = parseBetween(s,"counter{","}");
            if(counterName.size())
            {
                AbilityFactory abf(target->getObserver());
                Counter * counter = abf.parseCounter(counterName[1], NULL);
                if(counter && target->counters && target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness))
                {
                    Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);
                    intValue = targetCounter->nb;
                }
                SAFE_DELETE(counter);
            }
        }
        else if (s.find("convertedcost:") != string::npos || s.find("power:") != string::npos || s.find("toughness:") != string::npos)
        {
            bool powerCheck = false;
            bool toughnessCheck = false;
            bool costCheck = false;
            intValue = 0;
            vector<string>convertedType = parseBetween(s,"convertedcost:",":");
            if(convertedType.size())
                costCheck = true;
            else
            {
                convertedType = parseBetween(s,"power:",":");
                if(convertedType.size())
                    powerCheck = true;
                else
                {
                    convertedType = parseBetween(s,"toughness:",":");
                    if(convertedType.size())
                        toughnessCheck = true;
                }
            }
            if(!convertedType.size())
                return;
            bool high = false;
            int highest = 0;
            int lowest = 5000;
            if(convertedType[1].find("highest") != string::npos)
                high = true;

            string theType = convertedType[2];
            size_t zoned = theType.find(":");
            if(zoned == string::npos)
            {
                theType.append("|mybattlefield");
            }
            else
            {
                replace(theType.begin(), theType.end(), ':', '|');
            }
            TargetChooserFactory tf(card->getObserver());
            TargetChooser * tc = tf.createTargetChooser(theType.c_str(),NULL);
            int check = 0;
            for (int i = 0; i < 2; i++)
            {
                Player * p = card->getObserver()->players[i];
                MTGGameZone * zones[] = { p->game->battlefield, p->game->graveyard, p->game->hand, p->game->library };
                for (int k = 0; k < 4; k++)
                {
                    MTGGameZone * zone = zones[k];
                    if(tc->targetsZone(zone,target))
                    {
                        for(unsigned int w = 0;w < zone->cards.size();++w)
                        {
                            MTGCardInstance * cCard = zone->cards[w];
                            if(tc->canTarget(cCard))
                            {
                                if(costCheck)
                                    check = cCard->getManaCost()->getConvertedCost();
                                if(powerCheck)
                                    check = cCard->power;
                                if(toughnessCheck)
                                    check = cCard->toughness;

                                check > highest?highest = check:highest = highest;
                                check <= lowest?lowest = check:lowest = lowest;
                            }
                        }
                    }
                }
            }
            if(lowest == 5000)
                lowest = 0;
            SAFE_DELETE(tc);
            intValue = high?highest:lowest;
        }
        else if (s == "sunburst")
        {
            intValue = 0;
            if (card && card->previous && card->previous->previous)
            {
                intValue = card->previous->previous->sunburst;
            }
        }
        else if (s == "targetedcurses")
        {
            if(card->playerTarget)
                intValue = card->playerTarget->curses.size();
        }
        else if (s == "lifetotal")
        {
            intValue = target->controller()->life;
        }
        else if (s == "highestlifetotal")
        {
            intValue = target->controller()->life <= target->controller()->opponent()->life? target->controller()->opponent()->life:target->controller()->life;
        }
        else if (s == "lowestlifetotal")
        {
            intValue = target->controller()->life <= target->controller()->opponent()->life? target->controller()->life:target->controller()->opponent()->life;
        }
        else if (s == "thatmuch")
        {
            //the value that much is a variable to be used with triggered abilities.
            //ie:when ever you gain life, draw that many cards. when used in a trigger draw:thatmuch, will return the value
            //that the triggered event stored in the card for "that much".
            intValue = 0;
            intValue = target->thatmuch;
            int checkagain = 0;
            if(target->hasSubtype(Subtypes::TYPE_AURA) || target->hasSubtype(Subtypes::TYPE_EQUIPMENT))
            {
            if(target->target)
            {
            checkagain = target->target->thatmuch;
            }
            }
            if(checkagain > intValue)
                intValue = checkagain;
            if(card && card->thatmuch > intValue)
                intValue = card->thatmuch;
        }
        else if (s == "oplifelost")
        {
            intValue = target->controller()->opponent()->lifeLostThisTurn;
        }
        else if (s == "lifelost")
        {
            intValue = target->controller()->lifeLostThisTurn;
        }
        else if (s == "pdcount")
        {
            intValue = target->controller()->damageCount;
        }
        else if (s == "odcount")
        {
            intValue = target->controller()->opponent()->damageCount;
        }
        else if (s == "playerpoisoncount")
        {
            intValue = target->controller()->poisonCount;
        }
        else if (s == "opponentpoisoncount")
        {
            intValue = target->controller()->opponent()->poisonCount;
        }
        else if (s == "opponentlifetotal")
        {
            intValue = target->controller()->opponent()->life;
        }
        else if (s == "p" || s == "power")
        {
            intValue = target->getPower();
        }
        else if (s == "t" || s == "toughness")
        {
            intValue = target->getToughness();
        }
        else if (s == "kicked")
        {
            intValue = target->kicked;
        }
        else if (s == "handsize")
        {
            intValue = target->controller()->handsize;
        }
        else if (s == "phandcount")
        {
            intValue = target->controller()->game->hand->nb_cards;
        }
        else if (s == "ohandcount")
        {
            intValue = target->controller()->opponent()->game->hand->nb_cards;
        }
        else
        {
            intValue = atoi(s.c_str());
        }
        if(intValue > 0)
        {
            if(halfup)
            {
                if(intValue%2 == 1)
                    intValue++;
                intValue = intValue/2;
            }
            if(halfdown)
                intValue = intValue/2;
        }
        intValue *= multiplier;
    }
public:

    WParsedInt(int value = 0)
    {
        intValue = value;
    }

    WParsedInt(string s, Spell * spell, MTGCardInstance * card)
    {
        init(s, spell, card);
    }

    WParsedInt(string s, MTGCardInstance * card)
    {
        init(s, NULL, card);
    }

    int getValue()
    {
        return intValue;
    }
};

class WParsedPT
{
public:
    bool ok;
    WParsedInt power, toughness;

    WParsedPT(int p, int t)
    {
        power.intValue = p;
        toughness.intValue = t;
        ok = true;
    }

    WParsedPT(string s, Spell * spell, MTGCardInstance * card)
    {
        size_t found = s.find("/");
        ok = false;
        if (found != string::npos)
        {
            size_t end = s.find(" ", found);
            if (end == string::npos) end = s.size();
            size_t start = s.find_last_of(" ", found);
            if (start == string::npos)
                start = 0;
            else
                start++;
            power = WParsedInt(s.substr(start, found - start), spell, card);
            toughness = WParsedInt(s.substr(found + 1, end - found - 1), spell, card);

            ok = true;
        }
    }
};

//
//Triggers
//


// Triggers When a card gets added to a zone (@movedTo)
class TrCardAddedToZone: public Trigger
{
public:
    TargetZoneChooser * toTcZone, *fromTcZone;
    TargetChooser * toTcCard, *fromTcCard;
    bool sourceUntapped;
    bool isSuspended;
    TrCardAddedToZone(GameObserver* observer, int id, MTGCardInstance * source, TargetZoneChooser * toTcZone, TargetChooser * toTcCard,
            TargetZoneChooser * fromTcZone = NULL, TargetChooser * fromTcCard = NULL,bool once = false,bool sourceUntapped = false,bool isSuspended = false) :
        Trigger(observer, id, source, once), toTcZone(toTcZone), fromTcZone(fromTcZone), toTcCard(toTcCard), fromTcCard(fromTcCard),sourceUntapped(sourceUntapped),isSuspended(isSuspended)
    {
    };


    int triggerOnEventImpl(WEvent * event)
    {
        WEventZoneChange * e = dynamic_cast<WEventZoneChange*> (event);
        if (!e) return 0;
        if(sourceUntapped && source->isTapped() == 1)
            return 0;
        if(isSuspended && !source->suspended)
            return 0;
        if (!toTcZone->targetsZone(e->to)) return 0;
        if (!toTcCard->canTarget(e->card)) return 0;
        if (fromTcZone && !fromTcZone->targetsZone(e->from)) return 0;
        if (fromTcCard && !fromTcCard->canTarget(e->card->previous)) return 0;

        //Battlefield is a special case. We usually don't want to trigger when a card comes from battlefield to battlefield
        // http://code.google.com/p/wagic/issues/detail?id=179
        if ((e->from == game->players[0]->game->battlefield || e->from == game->players[1]->game->battlefield) && (e->to
                == game->players[0]->game->battlefield || e->to == game->players[1]->game->battlefield))
        {
            return 0;
        }

        return 1;
    }

    ~TrCardAddedToZone()
    {
        SAFE_DELETE(toTcZone);
        SAFE_DELETE(toTcCard);
        SAFE_DELETE(fromTcZone);
        SAFE_DELETE(fromTcCard);
    }

    TrCardAddedToZone * clone() const
    {
        return NEW TrCardAddedToZone(*this);
    }
};

class TrCardTapped: public Trigger
{
public:
    bool tap;
    TrCardTapped(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc, bool tap = true, bool once = false) :
        Trigger(observer, id, source, once, tc), tap(tap)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventCardTap * e = dynamic_cast<WEventCardTap *> (event);
        if (!e) return 0;
        if (e->before == e->after) return 0;
        if (e->after != tap) return 0;
        if (!tc->canTarget(e->card)) return 0;
        return 1;
    }

    TrCardTapped * clone() const
    {
        return NEW TrCardTapped(*this);
    }
};

class TrCardTappedformana: public Trigger
{
public:
    bool tap;
    TrCardTappedformana(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc, bool tap = true, bool once = false) :
        Trigger(observer, id, source, once, tc), tap(tap)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventCardTappedForMana * e = dynamic_cast<WEventCardTappedForMana *> (event);
        if (!e) return 0;
        if (e->before == e->after) return 0;
        if (e->after != tap) return 0;
        if (!tc->canTarget(e->card)) return 0;
        return 1;
    }

    TrCardTappedformana * clone() const
    {
        return NEW TrCardTappedformana(*this);
    }
};

class TrCombatTrigger: public Trigger
{
public:
    TargetChooser * fromTc;//from(card)
    bool limitOnceATurn;//can activate one time per turn
    int triggeredTurn;//the turn it last activated
    bool sourceUntapped;
    bool opponentPoisoned;
    //trigger types
    bool attackingTrigger;
    bool attackedAloneTrigger;
    bool notBlockedTrigger;
    bool attackBlockedTrigger;
    bool blockingTrigger;
    TrCombatTrigger(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc,TargetChooser * fromTc = NULL,
    bool once = false,bool limitOnceATurn = false,bool sourceUntapped = false,bool opponentPoisoned = false,
    bool attackingTrigger = false,bool attackedAloneTrigger = false,bool notBlockedTrigger = false,bool attackBlockedTrigger = false,bool blockingTrigger = false) :
    Trigger(observer, id, source,once, tc), fromTc(fromTc),limitOnceATurn(limitOnceATurn),sourceUntapped(sourceUntapped),opponentPoisoned(opponentPoisoned),
    attackingTrigger(attackingTrigger),attackedAloneTrigger(attackedAloneTrigger),notBlockedTrigger(notBlockedTrigger),
    attackBlockedTrigger(attackBlockedTrigger),blockingTrigger(blockingTrigger)
    {
        triggeredTurn = -1;
    }

    int triggerOnEventImpl(WEvent * event)
    {
        //general restrictions
        if (opponentPoisoned && !source->controller()->opponent()->isPoisoned())
            return 0;
        if (sourceUntapped  && source->isTapped() == 1)
            return 0;
        if (limitOnceATurn && triggeredTurn == game->turn)
            return 0;
        //the follow cases are not "else'd" on purpose, triggers which are conjoined such as
        //"whenever this card attacks, or attacks and is not blocked, are supposed to gernerally
        //trigger only once MTG rule 509.a-d, from either/or..not else'ing the statements and
        //listing them in order allows just that, a return on an event before hitting the 
        //next trigger condiational.
        //when triggers are not conjoined you can simply add another combat trigger to the card as normal.
        //an attacking creature can not also be a blocking creature.
        WEventCardAttacked * attacked = dynamic_cast<WEventCardAttacked *> (event);
        //event when a card was declared an attacker.
        if (attacked && attackingTrigger && !attacked->card->didblocked)
        {
            if (!attacked->card->didattacked) 
                return 0;
            if (!tc->canTarget(attacked->card)) 
                return 0;
            return returnResult();
        }
        WEventCardAttackedAlone * attackedAlone = dynamic_cast<WEventCardAttackedAlone *> (event);
        //event when a card was declared an attacker, and attacked alone.
        if (attackedAlone && attackedAloneTrigger && !attackedAlone->card->didblocked)
        {
            if (!attackedAlone->card->didattacked) 
                return 0;
            if (!tc->canTarget(attackedAlone->card)) 
                return 0;
            return returnResult();
        }
        WEventCardBlocked * blocked = dynamic_cast<WEventCardBlocked *> (event);
        //event when a card was declared a blocker.
        if (blocked && blockingTrigger && !blocked->card->didattacked)
        {
            if(!blocked->card->didblocked)
                return 0;
            if (fromTc && !fromTc->canTarget(blocked->opponent)) 
                return 0;
            if (!tc->canTarget(blocked->card)) 
                return 0;
            return returnResult();
        }
        WEventCardAttackedNotBlocked * notblocked = dynamic_cast<WEventCardAttackedNotBlocked *> (event);
        //event when a card was declared an attacker, but the attack was not blocked.
        if (notblocked && notBlockedTrigger && !notblocked->card->didblocked)
        {
            if (!notblocked->card->didattacked) 
                return 0;
            if (notblocked->card->isBlocked()) 
                return 0;
            if (!tc->canTarget(notblocked->card))
                return 0;
            return returnResult();
        }
        WEventCardAttackedBlocked * attackblocked = dynamic_cast<WEventCardAttackedBlocked *> (event);
        //event when a card was declared an attacker, then it became "blocked".
        if (attackblocked && attackBlockedTrigger && !attackblocked->card->didblocked)
        {
            if (!attackblocked->card->didattacked) 
                return 0;
            if (!attackblocked->card->isBlocked()) 
                return 0;
            if (fromTc && !fromTc->canTarget(attackblocked->opponent)) 
                return 0;
            if (!tc->canTarget(attackblocked->card)) 
                return 0;
            return returnResult();
        }
        //default return is 0 || not triggered.
        return 0;
    }
    
    int returnResult()
    {
        triggeredTurn = game->turn;
        return 1;
    }
    
    ~TrCombatTrigger()
    {
        SAFE_DELETE(fromTc);
    }

    TrCombatTrigger * clone() const
    {
        return NEW TrCombatTrigger(*this);
    }
};

class TrcardDrawn: public Trigger
{
public:

    TrcardDrawn(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc,bool once = false) :
        Trigger(observer, id, source,once, tc)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventcardDraw * e = dynamic_cast<WEventcardDraw *> (event);
        if (!e) return 0;
        if (!tc->canTarget(e->player)) return 0;

        return 1;
    }

    TrcardDrawn * clone() const
    {
        return NEW TrcardDrawn(*this);
    }
};

class TrCardSacrificed: public Trigger
{
public:
    TrCardSacrificed(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc,bool once = false) :
        Trigger(observer, id, source, once, tc)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventCardSacrifice * e = dynamic_cast<WEventCardSacrifice *> (event);
        if (!e) return 0;
        MTGCardInstance * check = e->cardAfter->next;
        MTGGameZone * oldZone = e->cardAfter->next->currentZone;
        check->currentZone = check->previousZone;
        if (!tc->canTarget(check,true))
        {
            check->currentZone = oldZone;
            return 0;
        }
        check->currentZone = oldZone;
        return 1;
    }

    TrCardSacrificed * clone() const
    {
        return NEW TrCardSacrificed(*this);
    }
};

class TrCardDiscarded: public Trigger
{
public:
    bool cycledTrigger;
    TrCardDiscarded(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc,bool once = false, bool cycledTrigger = false) :
    Trigger(observer, id, source, once, tc),cycledTrigger(cycledTrigger)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        MTGCardInstance * targetCard = NULL;
        if(cycledTrigger)
        {
            WEventCardCycle * c = dynamic_cast<WEventCardCycle *> (event);
            if (!c) return 0;
            targetCard = c->card;
        }
        else
        {
            WEventCardDiscard * e = dynamic_cast<WEventCardDiscard *> (event);
            if (!e) return 0;
            targetCard = e->card;
        }
        if (!targetCard || !tc->canTarget(targetCard)) return 0;
        return 1;
    }

    TrCardDiscarded * clone() const
    {
        return NEW TrCardDiscarded(*this);
    }
};

class TrDamaged: public Trigger
{
public:
    TargetChooser * fromTc;
    int type;//this allows damagenoncombat and combatdamage to share this trigger
    bool sourceUntapped;
    bool limitOnceATurn;
    int triggeredTurn;
    TrDamaged(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc, TargetChooser * fromTc = NULL, int type = 0,bool sourceUntapped = false,bool limitOnceATurn = false,bool once = false) :
        Trigger(observer, id, source, once, tc), fromTc(fromTc), type(type) , sourceUntapped(sourceUntapped),limitOnceATurn(limitOnceATurn)
    {
        triggeredTurn = -1;
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventDamage * e = dynamic_cast<WEventDamage *> (event);
        if (!e) return 0;
        if (limitOnceATurn && triggeredTurn == game->turn)
            return 0;
        if (sourceUntapped  && source->isTapped() == 1)
            return 0;
        if (!tc->canTarget(e->damage->target)) return 0;
        if (fromTc && !fromTc->canTarget(e->damage->source)) return 0;
        if (type == 1 && e->damage->typeOfDamage != DAMAGE_COMBAT) return 0;
        if (type == 2 && e->damage->typeOfDamage == DAMAGE_COMBAT) return 0;
        e->damage->target->thatmuch = e->damage->damage;
        e->damage->source->thatmuch = e->damage->damage;
        this->source->thatmuch = e->damage->damage;
        triggeredTurn = game->turn;

        return 1;
    }

    ~TrDamaged()
    {
        SAFE_DELETE(fromTc);
    }

    TrDamaged * clone() const
    {
        return NEW TrDamaged(*this);
    }
};

class TrLifeGained: public Trigger
{
public:
    TargetChooser * fromTc;
    int type;//this allows damagenoncombat and combatdamage to share this trigger
    bool sourceUntapped;
    TrLifeGained(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc, TargetChooser * fromTc = NULL, int type = 0,bool sourceUntapped = false,bool once = false) :
        Trigger(observer, id, source, once , tc),  fromTc(fromTc), type(type) , sourceUntapped(sourceUntapped)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventLife * e = dynamic_cast<WEventLife *> (event);
        if (!e) return 0;
        if (sourceUntapped  && source->isTapped() == 1)
        return 0;
        if (!tc->canTarget(e->player)) return 0;
        if (fromTc && !fromTc->canTarget(e->player)) return 0;
        if (type == 1 && (e->amount > 0)) return 0;
        if (type == 0 && (e->amount < 0)) return 0;
        e->player->thatmuch = abs(e->amount);
        this->source->thatmuch = abs(e->amount);

        return 1;
    }

    ~TrLifeGained()
    {
        SAFE_DELETE(fromTc);
    }

    TrLifeGained * clone() const
    {
        return NEW TrLifeGained(*this);
    }
};

//vampire trigger
class TrVampired: public Trigger
{
public:
    TargetChooser * fromTc;
    TrVampired(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc, TargetChooser * fromTc = NULL,bool once = false) :
    Trigger(observer, id, source, once, tc), fromTc(fromTc)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventVampire * vamp = dynamic_cast<WEventVampire*>(event);
        if (!vamp)
            return 0;

        if(fromTc && !fromTc->canTarget(vamp->source))
            return 0;
        tc->setAllZones();
        //creature that were "vampired" only happens in battlefield, and event sent when they hit a grave.
        //setting allzones, as we don't care since we know the preexisting condiations cover the zones.
        if(!tc->canTarget(vamp->victem))
            return 0;
        return 1;

    }

    ~TrVampired()
    {
        SAFE_DELETE(fromTc);
    }

    TrVampired * clone() const
    {
        return NEW TrVampired(*this);
    }
};

//targetted trigger
class TrTargeted: public Trigger
{
public:
    TargetChooser * fromTc;
    int type;
    TrTargeted(GameObserver* observer, int id, MTGCardInstance * source, TargetChooser * tc, TargetChooser * fromTc = NULL, int type = 0,bool once = false) :
    Trigger(observer, id, source, once, tc), fromTc(fromTc), type(type)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventTarget * e = dynamic_cast<WEventTarget *> (event);
        if (!e) return 0;
        if (!tc->canTarget(e->card)) return 0;
        if (fromTc && !fromTc->canTarget(e->source)) return 0;

        return 1;
    }

    ~TrTargeted()
    {
        SAFE_DELETE(fromTc);
    }

    TrTargeted * clone() const
    {
        return NEW TrTargeted(*this);
    }
};

//targetted trigger
class TrCounter: public Trigger
{
public:
    Counter * counter;
    int type;
    TrCounter(GameObserver* observer, int id, MTGCardInstance * source, Counter * counter, TargetChooser * tc, int type = 0,bool once = false) :
    Trigger(observer, id, source, once, tc),counter(counter), type(type)
    {
    }

    int triggerOnEventImpl(WEvent * event)
    {
        WEventCounters * e = dynamic_cast<WEventCounters *> (event);
        if (!e) return 0;
        if (type == 0 && !e->removed) return 0;
        if (type == 1 && !e->added) return 0;
        if (!(e->power == counter->power && e->toughness == counter->toughness && e->name == counter->name)) return 0;
        if (tc && !tc->canTarget(e->targetCard)) return 0;
        return 1;
    }

    ~TrCounter()
    {
        SAFE_DELETE(counter);
    }

    TrCounter * clone() const
    {
        TrCounter * mClone = NEW TrCounter(*this);
        mClone->counter = NEW Counter(*this->counter);
        return mClone;
    }
};

//Tutorial Messaging
class ATutorialMessage: public MTGAbility, public IconButtonsController
{
public:
    string mMessage;
    float mElapsed, mSH, mSW;
    JTexture * mBgTex;
    JQuad * mBg[9];
    bool mUserCloseRequest, mDontShow;
    bool mIsImage;
    int mLimit;

    ATutorialMessage(GameObserver* observer, MTGCardInstance * source, string message, int limit = 1);

    void Update(float dt);
    bool CheckUserInput(JButton key);
    void Render();
    string getOptionName();
    int alreadyShown();

    ATutorialMessage * clone() const;
    ~ATutorialMessage();

    //JGuiListener Implementation
    void ButtonPressed(int controllerId, int controlId);
};


//counters
class AACounter: public ActivatedAbility
{
public:
    string counterstring;
    int nb;
    int maxNb;
    int power;
    int toughness;
    string name;
    string menu;

    AACounter(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target,string counterstring, const char * _name, int power, int toughness, int nb,int maxNb = 0,
            ManaCost * cost = NULL);

    int resolve();
    const char* getMenuText();
    AACounter * clone() const;
};

//counters
class AARemoveAllCounter: public ActivatedAbility
{
public:
    int nb;
    int power;
    int toughness;
    string name;
    string menu;
    bool all;

    AARemoveAllCounter(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, const char * _name, int power, int toughness, int nb,
            bool all,ManaCost * cost = NULL);

    int resolve();
    const char* getMenuText();
    AARemoveAllCounter * clone() const;
};


class AAResetDamage: public ActivatedAbility
{
public:
    AAResetDamage(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, ManaCost * cost = NULL);
    int resolve();
    const char* getMenuText();
    AAResetDamage * clone() const;
};

class AAFizzler: public ActivatedAbility
{

public:
    AAFizzler(GameObserver* observer, int _id, MTGCardInstance * card, Spell * _target, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AAFizzler* clone() const;
};

/*
 Generic classes
 */

class ANewAffinity: public MTGAbility
{
public:
    string tcString;
    string manaString;
    ANewAffinity(GameObserver* observer, int _id, MTGCardInstance * _source,string Tc = "", string mana ="");
    void Update(float dt);
    int testDestroy();
    ANewAffinity * clone() const;
};

//if/ifnot Cond then EFFECT
class IfThenAbility: public InstantAbility
{
public:
    MTGAbility * delayedAbility;
    MTGAbility * delayedElseAbility;
    int type;
    string Cond;
    IfThenAbility(GameObserver* observer, int _id,MTGAbility * delayedAbility = NULL,MTGAbility * delayedElseAbility = NULL, MTGCardInstance * _source=NULL, Targetable * target = NULL, int type = 1,string Cond = "");
    int resolve();
    const char * getMenuText();
    IfThenAbility * clone() const;
    ~IfThenAbility();
};

//MayAbility: May do ...
class MayAbility: public MTGAbility, public NestedAbility
{
public:
    int triggered;
    bool must;
    Player * previousInterrupter;
    MTGAbility * mClone;
    ManaCost * optionalCost;

    MayAbility(GameObserver* observer, int _id, MTGAbility * _ability, MTGCardInstance * _source, bool must = false);

    void Update(float dt);

    const char * getMenuText();
    int testDestroy();

    int isReactingToTargetClick(Targetable * card);

    int reactToTargetClick(Targetable * object);

    MayAbility * clone() const;
    ~MayAbility();

};

//MayAbility with custom menues.
class MenuAbility: public MayAbility
{
public:
    int triggered;
    bool removeMenu;
    bool must;
    MTGAbility * mClone;
    vector<MTGAbility*>abilities;
    Player * who;
    MenuAbility(GameObserver* observer, int _id, Targetable * target, MTGCardInstance * _source, bool must = false, vector<MTGAbility*>abilities = vector<MTGAbility*>(),Player * who = NULL);
    void Update(float dt);
    int resolve();
    const char * getMenuText();
    int testDestroy();
    int isReactingToTargetClick(Targetable * card);
    int reactToTargetClick(Targetable * object);
    int reactToChoiceClick(Targetable * object,int choice,int control);
    MenuAbility * clone() const;
    ~MenuAbility();

};

class AAProliferate: public ActivatedAbility
{
public:
    AAProliferate(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target,ManaCost * cost = NULL);
    int resolve();
    const char* getMenuText();
    AAProliferate * clone() const;
    ~AAProliferate();
};

//MultiAbility : triggers several actions for a cost
class MultiAbility: public ActivatedAbility
{
public:
    vector<MTGAbility *> abilities;
    //Maintains abilities created by this instance, for cleanup
    vector<MTGAbility *> clones;

    MultiAbility(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost);
    int Add(MTGAbility * ability);
    int resolve();
    int addToGame();
    int destroy();
    const char * getMenuText();
    MultiAbility * clone() const;
    ~MultiAbility();
};

//Generic Activated Ability

class GenericActivatedAbility: public ActivatedAbility, public NestedAbility
{
public:
    MTGGameZone * activeZone;
    string newName;

    GenericActivatedAbility(GameObserver* observer, string newName,string castRestriction,int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, string limit = "",MTGAbility * sideEffects = NULL,string usesBeforeSideEffects = "",
            int restrictions = 0, MTGGameZone * dest = NULL);
    int resolve();
    const char * getMenuText();
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    void Update(float dt);
    int testDestroy();
    GenericActivatedAbility * clone() const;
    ~GenericActivatedAbility();

};

//place a card on the bottom of owners library
class AALibraryBottom: public ActivatedAbility
{
public:
    AALibraryBottom(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target = NULL, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AALibraryBottom * clone() const;
};

//Copier. ActivatedAbility
class AACopier: public ActivatedAbility
{
public:
    AACopier(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target = NULL, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AACopier * clone() const;
};
//imprint
class AAPhaseOut: public ActivatedAbility
{
public:
    AAPhaseOut(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target = NULL, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AAPhaseOut * clone() const;
};
//cloning...this makes a token thats a copy of the target.
class AACloner: public ActivatedAbility
{
public:
    int who;
    string with;
    string types;
    list<int> awith;
    list<int> colors;
    list<int> typesToAdd;

    AACloner(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target = NULL, ManaCost * _cost = NULL, int who = 0,
            string abilitiesStringList = "",string typeslist = "");
    int resolve();
    const char * getMenuText();
    virtual ostream& toString(ostream& out) const;
    AACloner * clone() const;
    ~AACloner();
};

// AAMover
class AAMover: public ActivatedAbility
{
public:
    string destination;
    MTGAbility * andAbility;
    AAMover(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, string dest, ManaCost * _cost = NULL);
    MTGGameZone * destinationZone(Targetable * target = NULL);
    int resolve();
    const char * getMenuText();
    const char * getMenuText(TargetChooser * fromTc);
    AAMover * clone() const;
    ~AAMover();
};

// AARandomMover
class AARandomMover: public ActivatedAbility
{
public:
    string abilityTC;
    string fromZone;
    string toZone;
    AARandomMover(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, string tcs, string from, string to);
    MTGGameZone * destinationZone(Targetable * target = NULL,string zone = "");
    int resolve();
    const char * getMenuText();
    AARandomMover * clone() const;
    ~AARandomMover();
};

//-----------------------------------------------------------------------------------------------

class AABuryCard: public ActivatedAbility
{
public:
    MTGAbility * andAbility;
    AABuryCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target);
    int resolve();
    const char * getMenuText();
    AABuryCard * clone() const;
    ~AABuryCard();
};

class AADestroyCard: public ActivatedAbility
{
public:
    MTGAbility * andAbility;
    AADestroyCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target);
    int resolve();
    const char * getMenuText();
    AADestroyCard * clone() const;
    ~AADestroyCard();
};

class AASacrificeCard: public ActivatedAbility
{
public:
    MTGAbility * andAbility;
    AASacrificeCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target);
    int resolve();
    const char * getMenuText();
    AASacrificeCard * clone() const;
    ~AASacrificeCard();
};

class AADiscardCard: public ActivatedAbility
{
public:
    MTGAbility * andAbility;
    AADiscardCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target);
    int resolve();
    const char * getMenuText();
    AADiscardCard * clone() const;
    ~AADiscardCard();
};

/* Generic Target Ability */
class GenericTargetAbility: public TargetAbility
{

public:
    int limitPerTurn;
    string limit;
    int counters;
    MTGGameZone * activeZone;
    string newName;
    MTGAbility * sideEffects;
    string usesBeforeSideEffects;
    string tcString;

    GenericTargetAbility(GameObserver* observer, string newName, string castRestriction, int _id, MTGCardInstance * _source, TargetChooser * _tc, MTGAbility * a, ManaCost * _cost = NULL, string limit = "",MTGAbility * sideEffects = NULL,string usesBeforeSideEffects = "", int restrictions = 0, MTGGameZone * dest = NULL,string tcString ="");
    const char * getMenuText();
    ~GenericTargetAbility();
    GenericTargetAbility * clone() const;
    int resolve();
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    void Update(float dt);
    int testDestroy();

};

//ninjutsu

class ANinja: public ActivatedAbility
{
public:
    ANinja(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target) :
        ActivatedAbility(observer, _id, card)
    {
        target = _target;
    }

    int resolve()
    {
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(!_target)
    return 0;
        MTGCardInstance * copy = _target->controller()->game->putInZone(_target,_target->currentZone,
                source->controller()->game->temp);
        Spell * spell = NEW Spell(game, copy);
        spell->resolve();
        MTGCardInstance * newcard = spell->source;
        newcard->summoningSickness = 0;
        newcard->tap();
        newcard->setAttacker(1);
        delete spell;
        return 1;
    }

    const char * getMenuText()
    {
        return "Ninjutsu";
    }

    ANinja * clone() const
    {
        return NEW ANinja(*this);
    }
};

//remove from combat

class ACombatRemoval: public ActivatedAbility
{
public:
    ACombatRemoval(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target) :
        ActivatedAbility(observer, _id, card)
    {
        target = _target;
    }

    int resolve()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (_target)
        {
            _target->initAttackersDefensers();
        }
        return 1;
    }

    const char * getMenuText()
    {
        return "Remove From Combat";
    }

    ACombatRemoval * clone() const
    {
        return NEW ACombatRemoval(*this);
    }
};

//Drawer, allows to draw a card for a cost:

class AADrawer: public ActivatedAbilityTP
{
public:

    string nbcardsStr;

    AADrawer(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost,string nbcardsStr, int who =
            TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AADrawer * clone() const;
    int getNumCards();
};



class ACastRestriction: public AbilityTP
{
public:
    TargetChooser * restrictionsScope; //a minimalist TargetChooser object describing the cards impacted by the restriction (for example: lands)
    WParsedInt *value; //"maxPerTurn" value
    MaxPerTurnRestriction * existingRestriction; // a pointer to the restriction that is being modified or that has been created (for restriction deletion purpose)
    bool modifyExisting; //if set to true, means we want to modify an existing restriction, otherwise we create a new one
    int zoneId; // identifier of the zone id impacted by the restriction
    Player * targetPlayer; // Reference to the player impacted by the restriction (for restriction deletion purpose)

    ACastRestriction(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who = TargetChooser::UNSET);
    int addToGame();
    int destroy();
    const char * getMenuText();
    ACastRestriction * clone() const;
    ~ACastRestriction();

};


class  AInstantCastRestrictionUEOT: public InstantAbilityTP
{
public:
    ACastRestriction * ability;


    AInstantCastRestrictionUEOT(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AInstantCastRestrictionUEOT * clone() const;
    ~AInstantCastRestrictionUEOT();
};

/*Gives life to target controller*/
class AALifer: public ActivatedAbilityTP
{
public:
    string life_s;
    AALifer(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string life_s, ManaCost * _cost = NULL,
            int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AALifer * clone() const;
    int getLife();

};

/*Player Wins Game*/
class AAWinGame: public ActivatedAbilityTP
{
public:
    AAWinGame(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost = NULL, int who =
            TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AAWinGame * clone() const;
};

/*Changes one of the basic abilities of target
 source : spell
 target : spell target (creature)
 modifier : 1 to add the ability, 0 to remove it
 _ability : Id of the ability, as described in mtgdefinitions
 */
class ABasicAbilityModifier : public MTGAbility
{
public:
    int modifier;
    int ability;
    bool value_before_modification;
    ABasicAbilityModifier(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _ability, int _modifier = 1) :
        MTGAbility(observer, _id, _source, _target), modifier(_modifier), ability(_ability)
    {
        aType = MTGAbility::STANDARDABILITYGRANT;
        abilitygranted = ability;
    }

    int addToGame()
    {
        value_before_modification = ((MTGCardInstance *) target)->basicAbilities.test(ability);

        assert(modifier < 2);
        ((MTGCardInstance *) target)->basicAbilities.set(ability, modifier > 0);

        return MTGAbility::addToGame();
    }

    int destroy()
    {
         assert(modifier < 2);
        ((MTGCardInstance *) target)->basicAbilities.set(ability, value_before_modification);

        return 1;
    }

    const char * getMenuText()
    {
        return Constants::MTGBasicAbilities[ability];
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "ABasicAbilityModifier ::: modifier : " << modifier << " ; ability : " << ability
                << " ; value_before_modification : " << value_before_modification << " (";
        return MTGAbility::toString(out) << ")";
    }

    ABasicAbilityModifier * clone() const
    {
        return NEW ABasicAbilityModifier(*this);
    }
};

/*Instants that modifies a basic ability until end of turn */
class AInstantBasicAbilityModifierUntilEOT : public InstantAbility
{
public:
    bool stateBeforeActivation;
    int ability;
    int value;
    AInstantBasicAbilityModifierUntilEOT(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _ability, int value)
        : InstantAbility(observer, _id, _source, _target), ability(_ability), value(value)
        {
            aType = MTGAbility::STANDARDABILITYGRANT;
            abilitygranted = ability;
        }

        int addToGame()
        {
            MTGCardInstance * _target = (MTGCardInstance *) target;
            stateBeforeActivation = _target->basicAbilities.test(ability);

            assert(value < 2);
            _target->basicAbilities.set(ability, value > 0);
            return InstantAbility::addToGame();
        }

        const char * getMenuText()
        {
            return Constants::MTGBasicAbilities[ability];
        }

    int destroy()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (_target)
            _target->basicAbilities.set(ability, stateBeforeActivation);
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "ABasicAbilityModifierUntilEOT ::: stateBeforeActivation : " << stateBeforeActivation << " ability : " << ability
                << " (";
        return InstantAbility::toString(out) << ")";
    }

    AInstantBasicAbilityModifierUntilEOT * clone() const
    {
        return NEW AInstantBasicAbilityModifierUntilEOT(*this);
    }
};

//Alteration of Ability until of turn (Aura)
class ABasicAbilityAuraModifierUntilEOT: public ActivatedAbility
{
public:
    AInstantBasicAbilityModifierUntilEOT * ability;
    ABasicAbilityAuraModifierUntilEOT(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost,
            int _ability, int _value = 1) :
        ActivatedAbility(observer, _id, _source, _cost, 0)
    {
        target = _target;
        ability = NEW AInstantBasicAbilityModifierUntilEOT(observer, _id, _source, _target, _ability, _value);
        aType = MTGAbility::STANDARDABILITYGRANT;
        abilitygranted = _ability;
    }

    int isReactingToClick(MTGCardInstance * card, ManaCost * cost = NULL)
    {
        //The upper level "GenericTargetAbility" takes care of the click so we always return 0 here
        return 0;
    }

    int resolve()
    {
        MTGAbility * a = ability->clone();
        a->target = target;
        a->addToGame();
        return 1;
    }

    int addToGame()
    {
        resolve();
        return ActivatedAbility::addToGame();
    }

    const char * getMenuText()
    {
        return ability->getMenuText();
    }

    ABasicAbilityAuraModifierUntilEOT * clone() const
    {
        ABasicAbilityAuraModifierUntilEOT * a = NEW ABasicAbilityAuraModifierUntilEOT(*this);
        a->ability = ability->clone();
        return a;
    }

    ~ABasicAbilityAuraModifierUntilEOT()
    {
        SAFE_DELETE(ability);
    }
};

/*Gives life each time a spell matching CardDescriptor's criteria are match . Optionnal manacost*/
class ASpellCastLife: public MTGAbility
{
public:
    CardDescriptor trigger;
    ManaCost * cost;
    int life;
    MTGCardInstance * lastUsedOn;
    MTGCardInstance * lastChecked;
    ASpellCastLife(GameObserver* observer, int id, MTGCardInstance * _source, CardDescriptor _trigger, ManaCost * _cost, int _life) :
        MTGAbility(observer, id, _source), trigger(_trigger), cost(_cost), life(_life), lastUsedOn(NULL), lastChecked(NULL)
    {
        aType = MTGAbility::LIFER;
    }
    ASpellCastLife(GameObserver* observer, int id, MTGCardInstance * _source, int color, ManaCost * _cost, int _life) :
        MTGAbility(observer, id, _source), cost(_cost), life(_life), lastUsedOn(NULL), lastChecked(NULL)
    {
        trigger.setColor(color);
    }

    int isReactingToClick(MTGCardInstance * _card, ManaCost * mana = NULL)
    {
        if (_card == source && game->currentlyActing()->game->inPlay->hasCard(source))
        {
            if (game->currentlyActing()->getManaPool()->canAfford(cost))
            {
                Interruptible * laststackitem = game->mLayers->stackLayer()->getAt(-1);
                if (laststackitem && laststackitem->type == ACTION_SPELL)
                {
                    Spell * spell = (Spell*) laststackitem;
                    if (spell->source != lastUsedOn && trigger.match(spell->source))
                    {
                        lastChecked = spell->source;
                        return 1;
                    }
                }
            }
        }
        return 0;
    }

    int reactToClick(MTGCardInstance * _card)
    {
        if (!isReactingToClick(_card)) return 0;
        game->currentlyActing()->getManaPool()->pay(cost);
        game->currentlyActing()->life += life;
        lastUsedOn = lastChecked;
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "ASpellCastLife ::: trigger : ? " // << trigger
                << " ; cost : " << cost << " ; life : " << life << " ; lastUsedOn : " << lastUsedOn << " ; lastChecked : "
                << lastChecked << " (";
        return MTGAbility::toString(out) << ")";
    }

    ASpellCastLife * clone() const
    {
        return NEW ASpellCastLife(*this);
    }

    ~ASpellCastLife()
    {
        SAFE_DELETE(cost);
    }

};

//Allows to untap at any moment for an amount of mana
class AUnBlocker: public MTGAbility
{
public:
    ManaCost * cost;
    AUnBlocker(GameObserver* observer, int id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
        MTGAbility(observer, id, _source, _target), cost(_cost)
    {
    }

    int isReactingToClick(MTGCardInstance * _card, ManaCost * mana = NULL)
    {
        if (_card == target && game->currentlyActing()->game->inPlay->hasCard(source) && _card->isTapped())
        {
            if (game->currentlyActing()->getManaPool()->canAfford(cost))
            {
                return 1;
            }
        }
        return 0;
    }

    int reactToClick(MTGCardInstance * _card)
    {
        if (!isReactingToClick(_card)) return 0;
        game->currentlyActing()->getManaPool()->pay(cost);
        _card->attemptUntap();
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AUnBlocker ::: cost : " << cost << " (";
        return MTGAbility::toString(out) << ")";
    }

    AUnBlocker * clone() const
    {
        return NEW AUnBlocker(*this);
    }

};

//Protection From (creature/aura)
class AProtectionFrom: public MTGAbility
{
public:
    TargetChooser * fromTc;
    string tcstr;
    AProtectionFrom(GameObserver* observer, int id, MTGCardInstance * _source, MTGCardInstance * _target, TargetChooser *fromTc,string tcstr) :
        MTGAbility(observer, id, _source, _target), fromTc(fromTc),tcstr(tcstr)
    {

    }

    int addToGame()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        _target->addProtection(fromTc);
        return MTGAbility::addToGame();
    }

    int destroy()
    {
        ((MTGCardInstance *) target)->removeProtection(fromTc);
        return 1;
    }

    const char * getMenuText()
    {
        sprintf(menuText,"Protection from %s",tcstr.c_str());
        return menuText;
    }
    
    AProtectionFrom * clone() const
    {
        AProtectionFrom * a = NEW AProtectionFrom(*this);
        a->fromTc = fromTc->clone();
        return a;
    }

    ~AProtectionFrom()
    {
        SAFE_DELETE(fromTc);
    }

};

//cant be target of...
class ACantBeTargetFrom: public MTGAbility
{
public:
    TargetChooser * fromTc;
    ACantBeTargetFrom(GameObserver* observer, int id, MTGCardInstance * _source, MTGCardInstance * _target, TargetChooser *fromTc) :
        MTGAbility(observer, id, _source, _target), fromTc(fromTc)
    {

    }

    int addToGame()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        _target->addCantBeTarget(fromTc);
        return MTGAbility::addToGame();
    }

    int destroy()
    {
        ((MTGCardInstance *) target)->removeCantBeTarget(fromTc);
        return 1;
    }

    ACantBeTargetFrom * clone() const
    {
        ACantBeTargetFrom * a = NEW ACantBeTargetFrom(*this);
        a->fromTc = fromTc->clone();
        return a;
    }

    ~ACantBeTargetFrom()
    {
        SAFE_DELETE(fromTc);
    }

};
//Can't be blocked by...
class ACantBeBlockedBy: public MTGAbility
{
public:
    TargetChooser * fromTc;
    ACantBeBlockedBy(GameObserver* observer, int id, MTGCardInstance * _source, MTGCardInstance * _target, TargetChooser *fromTc) :
        MTGAbility(observer, id, _source, _target), fromTc(fromTc)
    {

    }

    int addToGame()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        _target->addCantBeBlockedBy(fromTc);
        return MTGAbility::addToGame();
    }

    int destroy()
    {
        ((MTGCardInstance *) target)->removeCantBeBlockedBy(fromTc);
        return 1;
    }

    ACantBeBlockedBy * clone() const
    {
        ACantBeBlockedBy * a = NEW ACantBeBlockedBy(*this);
        a->fromTc = fromTc->clone();
        return a;
    }

    ~ACantBeBlockedBy()
    {
        SAFE_DELETE(fromTc);
    }

};

//Alteration of Power and Toughness  (enchantments)
class APowerToughnessModifier: public MTGAbility
{
public:
    WParsedPT * wppt;
    string PT;
    bool nonstatic;
    APowerToughnessModifier(GameObserver* observer, int id, MTGCardInstance * _source, MTGCardInstance * _target, WParsedPT * wppt,string PT,bool nonstatic) :
        MTGAbility(observer, id, _source, _target), wppt(wppt),PT(PT),nonstatic(nonstatic)
    {
        aType = MTGAbility::STANDARD_PUMP;
    }
    
        void Update(float dt)
        {
            if(!nonstatic)
                return;
            ((MTGCardInstance *) target)->power -= wppt->power.getValue();
            ((MTGCardInstance *) target)->addToToughness(-wppt->toughness.getValue());
            if(PT.size())
            {
                SAFE_DELETE(wppt);
                wppt = NEW WParsedPT(PT,NULL,(MTGCardInstance *) source);
            }
            MTGCardInstance * _target = (MTGCardInstance *) target;
            _target->power += wppt->power.getValue();
            _target->addToToughness(wppt->toughness.getValue());
        }
        
    int addToGame()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if(PT.size())
        {
            SAFE_DELETE(wppt);
            wppt = NEW WParsedPT(PT,NULL,(MTGCardInstance *) source);
        }
        _target->power += wppt->power.getValue();
        _target->addToToughness(wppt->toughness.getValue());
        if(_target->has(Constants::INDESTRUCTIBLE) && wppt->toughness.getValue() < 0 && _target->toughness <= 0)
        {
        _target->controller()->game->putInGraveyard(_target);
        }
        return MTGAbility::addToGame();
    }

    int destroy()
    {
        ((MTGCardInstance *) target)->power -= wppt->power.getValue();
        ((MTGCardInstance *) target)->addToToughness(-wppt->toughness.getValue());
        return 1;
    }
    const char * getMenuText()
    {                
        if(PT.size())
        {
            SAFE_DELETE(wppt);
            wppt = NEW WParsedPT(PT,NULL,(MTGCardInstance *) source);
        }
        sprintf(menuText, "%i/%i", wppt->power.getValue(), wppt->toughness.getValue());
        return menuText;
    }
    APowerToughnessModifier * clone() const
    {
        APowerToughnessModifier * a = NEW APowerToughnessModifier(*this);
        a->wppt = NEW WParsedPT(*(a->wppt));
        return a;
    }

    ~APowerToughnessModifier()
    {
        delete (wppt);
    }

};

class GenericInstantAbility: public InstantAbility, public NestedAbility
{
public:
    GenericInstantAbility(GameObserver* observer, int _id, MTGCardInstance * _source, Damageable * _target, MTGAbility * ability) :
        InstantAbility(observer, _id, _source, _target), NestedAbility(ability)
    {
        ability->target = _target;
    }

    int addToGame()
    {
        ability->forceDestroy = -1;
        ability->target = target; //Might have changed since initialization
        ability->addToGame();
        return InstantAbility::addToGame();
    }

    int destroy()
    {
        if (game->removeObserver(ability))
            ability = NULL;
        else
            SAFE_DELETE(ability);
        return InstantAbility::destroy();
    }

    GenericInstantAbility * clone() const
    {
        GenericInstantAbility * a = NEW GenericInstantAbility(*this);
        a->ability = ability->clone();
        return a;
    }

    ~GenericInstantAbility()
    {
        SAFE_DELETE(ability);
    }
};

//this generic ability assumes that what is added will take care of its own removel.
class GenericAbilityMod: public InstantAbility, public NestedAbility
{
public:
    GenericAbilityMod(GameObserver* observer, int _id, MTGCardInstance * _source, Damageable * _target, MTGAbility * ability) :
      InstantAbility(observer, _id, _source,_target), NestedAbility(ability)
      {
          ability->target = _target;
      }

      int addToGame()
      {
          InstantAbility::addToGame();
          return 1;
      }

      int resolve()
      {
          MTGAbility * toAdd = ability->clone();
          toAdd->forceDestroy = -1;
          toAdd->target = target;
          if(toAdd->getActionTc())
          {
              toAdd->reactToTargetClick(source);
              return 1;
          }
          toAdd->addToGame();
          return 1;
      }

      const char * getMenuText()
      {
          return ability->getMenuText();
      }

      GenericAbilityMod * clone() const
      {
          GenericAbilityMod * a = NEW GenericAbilityMod(*this);
          a->ability = ability->clone();
          return a;
      }

      ~GenericAbilityMod()
      {
          SAFE_DELETE(ability);
      }
};
//generic addtogame
class GenericAddToGame: public InstantAbility, public NestedAbility
{
public:
    GenericAddToGame(GameObserver* observer, int _id, MTGCardInstance * _source, Damageable * _target, MTGAbility * ability) :
      InstantAbility(observer, _id, _source,_target), NestedAbility(ability)
      {
          ability->target = _target;
      }

      int addToGame()
      {
          InstantAbility::addToGame();
          return 1;
      }

      int resolve()
      {
          MTGAbility * toAdd = ability->clone();
          toAdd->target = target;
          if(toAdd->getActionTc())
              return toAdd->reactToTargetClick(source);
          return toAdd->addToGame();
      }

      const char * getMenuText()
      {
          return ability->getMenuText();
      }

      GenericAddToGame * clone() const
      {
          GenericAddToGame * a = NEW GenericAddToGame(*this);
          a->ability = ability->clone();
          return a;
      }

      ~GenericAddToGame()
      {
          SAFE_DELETE(ability);
      }
};
//Circle of Protections
class ACircleOfProtection: public TargetAbility
{
protected:
    map<ReplacementEffect*, int> current;
public:
    ACircleOfProtection(GameObserver* observer, int _id, MTGCardInstance * source, int _color) :
        TargetAbility(observer, _id, source, NEW SpellOrPermanentTargetChooser(source->owner->getObserver(), source, _color), NEW ManaCost(), 0)
    {
        getCost()->add(Constants::MTG_COLOR_ARTIFACT, 1);
        tc->targetter = NULL; //Circle of Protection doesn't use the word "source"
    }

    int resolve()
    {
        MTGCardInstance * _target = NULL;
        if (!(_target = tc->getNextCardTarget()))
        {
            Spell * starget = tc->getNextSpellTarget();
            _target = starget->source;
        }
        if (!_target) return 0;
        REDamagePrevention * re = NEW REDamagePrevention(this, NEW CardTargetChooser(game, _target, NULL), NEW PlayerTargetChooser(game, 0, 1,
                source->controller()));
        current[re] = 1;
        game->replacementEffects->add(re);
        return 1;
    }

    void clear()
    {
        for (map<ReplacementEffect*, int>::iterator it = current.begin(); it != current.end(); it++)
        {
            ReplacementEffect* re = (*it).first;
            game->replacementEffects->remove(re);
            delete re;
        }
        current.clear();
    }

    void Update(float dt)
    {
        if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP) clear();
        TargetAbility::Update(dt);
    }

    ~ACircleOfProtection()
    {
        clear();
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "ACircleOfProtection ::: (";
        return TargetAbility::toString(out) << ")";
    }
    ACircleOfProtection * clone() const
    {
        return NEW ACircleOfProtection(*this);
    }
};

//Basic regeneration mechanism for a Mana cost
class AStandardRegenerate: public ActivatedAbility
{
public:
    AStandardRegenerate(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL) :
        ActivatedAbility(observer, _id, _source, _cost, 0)
    {
        target = _target;
        aType = MTGAbility::STANDARD_REGENERATE;
    }

    int resolve()
    {

        MTGCardInstance * _target = (MTGCardInstance *) target;
        _target->regenerate();
        return 1;
    }

    const char * getMenuText()
    {
        return "Regenerate";
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AStandardRegenerate ::: (";
        return ActivatedAbility::toString(out) << ")";
    }
    AStandardRegenerate * clone() const
    {
        return NEW AStandardRegenerate(*this);
    }
};

//Aura Enchantments that provide controller of target life or damages at a given phase of their turn
class ARegularLifeModifierAura: public MTGAbility
{
public:
    int life;
    int phase;
    int onlyIfTargetTapped;
    ARegularLifeModifierAura(GameObserver* observer, int id, MTGCardInstance * _source, MTGCardInstance * _target, int _phase, int _life,
            int _onlyIfTargetTapped = 0) :
        MTGAbility(observer, id, _source, _target), life(_life), phase(_phase), onlyIfTargetTapped(_onlyIfTargetTapped)
    {
    }

    void Update(float dt)
    {
        if (newPhase != currentPhase && newPhase == phase && game->currentPlayer == ((MTGCardInstance *) target)->controller())
        {
            if (!onlyIfTargetTapped || ((MTGCardInstance *) target)->isTapped())
            {
                if (life > 0)
                {
                    game->currentPlayer->life += life;
                }
                else
                {
                    game->mLayers->stackLayer()->addDamage(source, game->currentPlayer, -life);
                }
            }
        }
    }
    virtual ostream& toString(ostream& out) const
    {
        out << "ARegularLifeModifierAura ::: life : " << life << " ; phase : " << phase << " ; onlyIfTargetTapped : "
                << onlyIfTargetTapped << " (";
        return MTGAbility::toString(out) << ")";
    }
    ARegularLifeModifierAura * clone() const
    {
        return NEW ARegularLifeModifierAura(*this);
    }
};

//Generic Kird Ape
class AAsLongAs: public ListMaintainerAbility, public NestedAbility
{
public:
    MTGAbility * a;
    int includeSelf;
    int mini, maxi;
    bool miniFound, maxiFound, compareZone;
    int amount[2];
    AAsLongAs(GameObserver* observer, int _id, MTGCardInstance * _source, Damageable * _target, TargetChooser * _tc, int _includeSelf,
        MTGAbility * ability, int mini = 0, int maxi = 0,bool miniFound = false,bool maxiFound = false,bool compareZone = false) :
    ListMaintainerAbility(observer, _id, _source, _target), NestedAbility(ability), mini(mini), maxi(maxi),miniFound(miniFound),maxiFound(maxiFound),compareZone(compareZone)
    {
        for (int j = 0; j < 2; j++)
            amount[j] = 0;
        tc = _tc;
        includeSelf = _includeSelf;
        tc->targetter = NULL;
        ability->source = source;
        ability->target = target;
        a = NULL;
    }

    void Update(float dt)
    {
        ListMaintainerAbility::Update(dt);
        if(!ability->oneShot) {
            SorterFunction();
        }
    }

    void findMatchingAmount()
    {
        int Value = 0;
        for (int i = 0; i < 2; i++)
        {
            Player * p = game->players[i];
            MTGGameZone * zones[] = { p->game->inPlay, p->game->graveyard, p->game->hand, p->game->library };
            for (int k = 0; k < 4; k++)
            {
                MTGGameZone * zone = zones[k];
                Value = zone->countByCanTarget(tc);
            }
            amount[i] = Value;
        }
    }

    int SorterFunction()
    {
        updateTargets();
        int size = 0;
        size = (int) cards.size();
        if(compareZone)
            findMatchingAmount();
        //compare like tc zones to find a matching amount.
        //if any player has less/more of Tc targetable cards, ability is valid.
        /////////////////DO NOT REFACTOR THIS SECTION/////////////////////////////////////////
        //these were seperated becuase previous methods were far too confusing to understand//
        //////////////////////////////////////////////////////////////////////////////////////
        if (miniFound)
        {
            if (size > mini || (compareZone && (amount[0] > mini || amount[1] > mini))) 
            {
                addAbilityToGame();
            }
            else
            {
                removeAbilityFromGame();
            }
        }
        if (maxiFound)
        {
            if (size < maxi || (compareZone && (amount[0] < maxi || amount[1] < maxi))) 
            {
                addAbilityToGame();
            }
            else
            {
                removeAbilityFromGame();
            }
        }
        /////////////////////////////////////////////////////////////////////////
        cards.clear();
        players.clear();
        return 1;
    }

    int canBeInList(MTGCardInstance * card)
    {
        if(card->isPhased || source->isPhased)
            return 0;
        if ((includeSelf || card != source) && tc->canTarget(card)) 
            return 1;

        return 0;
    }
    //////////resolve is used when the aslongas is nested, or used on instants and sorceries/////////
    int resolve()
    {
        SorterFunction();
        if (ability->oneShot)
        {
            a = NULL; //allows to call the effect several times
        }
        return 1;
    }

    int addAbilityToGame()
    {
        if (a) return 0;
        a = ability->clone();
        if (a->oneShot)
        {
            a->resolve();
            SAFE_DELETE(a);
        }
        else
        {
            a->addToGame();
        }
        return 1;
    }

    int removeAbilityFromGame()
    {
        if (!a) return 0;
        game->removeObserver(a);
        a = NULL;
        return 1;
    }
    /////////////////section required/////////////////////
    int added(MTGCardInstance * card)
    {
        return 1;
    }
    int added(Player * p)
    {
        return 1;
    }
    int removed(MTGCardInstance * card)
    {
        return 1;
    }
    ///////////////////////////////////////////////////////
    ~AAsLongAs()
    {
        SAFE_DELETE(ability);
    }

    const char * getMenuText()
    {
        if(ability)
        {
            return ability->getMenuText();
        }
        else
        {
            return "Ability";
        }
    }

    AAsLongAs * clone() const
    {
        AAsLongAs * a = NEW AAsLongAs(*this);
        a->ability = ability->clone();
        return a;
    }
};

//Lords (Merfolk lord...) give power and toughness to OTHER creatures of their type, they can give them special abilities, regeneration
class ALord: public ListMaintainerAbility, public NestedAbility
{
public:
    int includeSelf;
    map<Damageable *, MTGAbility *> abilities;

    ALord(GameObserver* observer, int _id, MTGCardInstance * card, TargetChooser * _tc, int _includeSelf, MTGAbility * a) :
        ListMaintainerAbility(observer, _id, card), NestedAbility(a)
        {
            tc = _tc;
            tc->targetter = NULL;
            includeSelf = _includeSelf;
            if(ability->aType == MTGAbility::STANDARD_PREVENT)
                aType = MTGAbility::STANDARD_PREVENT;
        }
     
    //returns true if it is me who created ability a attached to Damageable d
    bool isParentOf(Damageable * d, MTGAbility * a)
    {
        if (abilities.find(d) != abilities.end())
            return (abilities[d] == a);
        return false;
    }

    int canBeInList(Player *p)
    {
        if (tc->canTarget(p)) return 1;
        return 0;
    }

    int canBeInList(MTGCardInstance * card)
    {
        if(card->isPhased || source->isPhased)
            return 0;
        if ((includeSelf || card != source) && tc->canTarget(card)) 
            return 1;
        return 0;
    }

    int resolve()
    {
        //TODO check if ability is oneShot ?
        updateTargets();
        cards.clear();
        players.clear();
        return 1;
    }

    int _added(Damageable * d)
    {
        MTGAbility * a = ability->clone();
        a->target = d;
        if (a->oneShot)
        {
            a->resolve();
            SAFE_DELETE(a);
        }
        else
        {
            if (d->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
            {
                a->source = (MTGCardInstance *) d;
            }
            if (oneShot)
            {
                MTGAbility * wrapper = NEW GenericInstantAbility(game, 1, source, d, a);
                wrapper->addToGame();
            }
            else
            {
                a->addToGame();
                abilities[d] = a;
            }
        }
        return 1;
    }

    int added(MTGCardInstance * card)
    {
        return _added(card);
    }

    int added(Player * p)
    {
        return _added(p);
    }

    int removed(MTGCardInstance * card)
    {
        if (abilities.find(card) != abilities.end() && !(forceDestroy == -1 && forcedAlive == 1))//only embelms have forcedestroy = -1 and forcedalive = 1
        { 
            game->removeObserver(abilities[card]);
            abilities.erase(card);
        }
        return 1;
    }

    ~ALord()
    {
        SAFE_DELETE(ability);
    }

    const char * getMenuText()
    {
        //Special case for move
        if (AAMover * move = dynamic_cast<AAMover *>(ability))
            return move->getMenuText(tc);

        return ability->getMenuText();
    }

    ALord * clone() const
    {
        ALord * a = NEW ALord(*this);
        a->ability = ability->clone();
        return a;
    }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//a different lord for auras and enchantments. http://code.google.com/p/wagic/issues/detail?id=244
class ATeach: public ListMaintainerAbility, public NestedAbility
{
public:
    int includeSelf;
    map<Damageable *, MTGAbility *> skills;

    ATeach(GameObserver* observer, int _id, MTGCardInstance * card, TargetChooser * _tc, int _includeSelf, MTGAbility * a) :
        ListMaintainerAbility(observer, _id, card), NestedAbility(a)
    {
        tc = _tc;
        tc->targetter = NULL;
        includeSelf = 0;
        aType = MTGAbility::STANDARD_TEACH;
    }

        int canBeInList(MTGCardInstance * card)
        {
            if(card->isPhased || source->isPhased)
                return 0;
            if(tc->canTarget(card) && card != tc->source)
            {
                if ((tc->source->hasSubtype(Subtypes::TYPE_AURA) || tc->source->hasSubtype(Subtypes::TYPE_EQUIPMENT) || tc->source->hasSubtype("instant")
                    || tc->source->hasSubtype("sorcery")) && card == tc->source->target ) 
                    return 1;
                if(tc->source->hasSubtype(Subtypes::TYPE_CREATURE))
                {
                    for(size_t myChild = 0; myChild < tc->source->parentCards.size();++myChild)
                    {
                        if(tc->source->parentCards[myChild] == card)
                            return 1;
                    }
                }
            }
            return 0;
        }

    int resolve()
    {
        updateTargets();
        cards.clear();
        players.clear();
        return 1;
    }

    int added(MTGCardInstance * card)
    {
        return _added(card);
    }

    int removed(MTGCardInstance * card)
    {
        if (skills.find(card) != skills.end())
        {
            if(!game->removeObserver(skills[card]))
            {
                skills[card]->destroy();
            }
            if(skills[card])
                skills.erase(card);
        }
        return 1;
    }

    int _added(Damageable * d)
    {
        MTGAbility * a = ability->clone();

        if (a->source->hasSubtype(Subtypes::TYPE_AURA) || a->source->hasSubtype(Subtypes::TYPE_EQUIPMENT) || a->source->hasSubtype("instant")
                || a->source->hasSubtype("sorcery"))
        {
            a->target = a->source->target;
        }
        else
        {
            if(tc->source->hasSubtype(Subtypes::TYPE_CREATURE))
                a->target = d;
            else
                return 0;
        }

        if (a->oneShot)
        {
            a->resolve();
            SAFE_DELETE(a);
        }
        else
        {
            if (d->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
            {
                a->source = (MTGCardInstance *) d;
            }
            if (oneShot)
            {
                MTGAbility * wrapper = NEW GenericInstantAbility(game, 1, source, d, a);
                wrapper->addToGame();
            }
            else
            {
                skills[d] = a;
                a->addToGame();
            }
        }
        return 1;
    }
    ~ATeach()
    {
        SAFE_DELETE(ability);
    }

    ATeach * clone() const
    {
        ATeach * a =  NEW ATeach(*this);
        a->ability = ability->clone();
        return a;
    }

};
//

/* assign a creature to block a target */
class AABlock: public InstantAbility
{
public:
    AABlock(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost = NULL);
    int resolve();
    AABlock * clone() const;
};

/* assign a creature as a pair to target */
class PairCard: public InstantAbility
{
public:
    PairCard(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost = NULL);
    int resolve();
    PairCard * clone() const;
};

/* create a parent child association between cards */
class AAConnect: public InstantAbility
{
public:
    AAConnect(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost = NULL);
    int resolve();
    AAConnect * clone() const;
};

//equipment
class AEquip: public TargetAbility
{
public:
    vector<MTGAbility *> currentAbilities;
    AEquip(GameObserver* observer, int _id, MTGCardInstance * _source, ManaCost * _cost = NULL, int restrictions =
            ActivatedAbility::AS_SORCERY) :
        TargetAbility(observer, _id, _source, NULL, _cost, restrictions)
    {
        aType = MTGAbility::STANDARD_EQUIP;
    }
    
    int unequip()
    {
        if (source->target)
        {
            source->target->equipment -= 1;
            source->parentCards.clear();
            for(unsigned int w = 0;w < source->target->childrenCards.size();w++)
            {
                MTGCardInstance * child = source->target->childrenCards[w];
                if(child == source)
                    source->target->childrenCards.erase(source->target->childrenCards.begin() + w);
            }
        }
        source->target = NULL;
        for (size_t i = 0; i < currentAbilities.size(); ++i)
        {
            MTGAbility * a = currentAbilities[i];
            if (dynamic_cast<AEquip *> (a) || dynamic_cast<ATeach *> (a) || dynamic_cast<AAConnect *> (a) || (a->aType == MTGAbility::STANDARD_TOKENCREATOR && a->oneShot))
            {
                SAFE_DELETE(a);
                continue;
            }
            game->removeObserver(currentAbilities[i]);
        }
        currentAbilities.clear();
        return 1;
    }

    int equip(MTGCardInstance * equipped)
    {
        source->target = equipped;
        source->target->equipment += 1;
        source->parentCards.push_back(equipped);
        source->target->childrenCards.push_back((MTGCardInstance*)source);
        AbilityFactory af(game);
        af.getAbilities(&currentAbilities, NULL, source);
        for (size_t i = 0; i < currentAbilities.size(); ++i)
        {
            MTGAbility * a = currentAbilities[i];
            if (dynamic_cast<AEquip *> (a)) continue;
            if (dynamic_cast<ATeach *> (a)) continue;
            if (dynamic_cast<AAConnect *> (a)) continue;
            if (a->aType == MTGAbility::STANDARD_TOKENCREATOR && a->oneShot)
            {
            a->forceDestroy = 1;
            continue;
            }
            //we generally dont want to pass oneShot tokencreators to the cards
            //we equip...
            a->addToGame();
        }
        return 1;

    }

    int resolve()
    {
        MTGCardInstance * mTarget = tc->getNextCardTarget();
        if (!mTarget) return 0;
        if (mTarget == source) return 0;
        unequip();
        equip(mTarget);
        return 1;
    }

    const char * getMenuText()
    {
        return "Equip";
    }

    int testDestroy()
    {
        if (source->target && !game->isInPlay(source->target)) 
        unequip();
        if(!game->connectRule)
        {
        if (source->target && TargetAbility::tc && !TargetAbility::tc->canTarget((Targetable *)source->target,true)) 
        unequip();
        }
        return TargetAbility::testDestroy();
    }

    int destroy()
    {
        unequip();
        return TargetAbility::destroy();
    }

    AEquip * clone() const
    {
        return NEW AEquip(*this);
    }

};

class ATokenCreator: public ActivatedAbility
{
public:
    list<int> abilities;
    list<int> types;
    list<int> colors;
    int power, toughness;
    int tokenId;
    string name;
    string sabilities;
    string starfound;
    WParsedInt * multiplier;
    int who;
    bool aLivingWeapon;
    string spt;
    bool battleReady;
    MTGCardInstance * myToken;
    vector<MTGAbility *> currentAbilities;
    Player * tokenReciever;
    ATokenCreator(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost, int tokenId,string starfound, WParsedInt * multiplier = NULL,
        int who = 0,bool aLivingWeapon = false) :
    ActivatedAbility(observer, _id, _source, _cost, 0), tokenId(tokenId), starfound(starfound),multiplier(multiplier), who(who),aLivingWeapon(aLivingWeapon)
    {
        if (!multiplier) this->multiplier = NEW WParsedInt(1);
        MTGCard * card = MTGCollection()->getCardById(tokenId);
        if (card) name = card->data->getName();
        battleReady = false;
    }

    ATokenCreator(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost, string sname, string stypes, int _power, int _toughness,
        string sabilities, string starfound,WParsedInt * multiplier = NULL, int who = 0,bool aLivingWeapon = false,string spt = "") :
    ActivatedAbility(observer, _id, _source, _cost, 0),sabilities(sabilities),starfound(starfound), multiplier(multiplier), who(who),aLivingWeapon(aLivingWeapon),spt(spt)
    {
        power = _power;
        toughness = _toughness;
        name = sname;
        who = who;
        tokenId = 0;
        aType = MTGAbility::STANDARD_TOKENCREATOR;
        battleReady = false;
        if (!multiplier) this->multiplier = NEW WParsedInt(1);
        //TODO this is a copy/past of other code that's all around the place, everything should be in a dedicated parser class;

        for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++)
        {
            size_t found = sabilities.find(Constants::MTGBasicAbilities[j]);
            if (found != string::npos)
            {
                abilities.push_back(j);
            }
        }

        if(sabilities.find("battleready") != string::npos)
            battleReady = true;

        if(sabilities.find("chosencolor") != string::npos)
        {
            colors.push_back(source->chooseacolor);
        }

        for (int j = 0; j < Constants::NB_Colors; j++)
        {
            size_t found = sabilities.find(Constants::MTGColorStrings[j]);
            if (found != string::npos)
            {
                colors.push_back(j);
            }
        }

        string s = stypes;
        while (s.size())
        {
            size_t found = s.find(" ");
            if (found != string::npos)
            {
                string toCheck = s.substr(0, found);
                if(toCheck.find("chosentype") != string::npos || toCheck.find("Chosentype") != string::npos)
                {
                    toCheck = source->chooseasubtype;
                }
                int id = MTGAllCards::findType(toCheck);
                types.push_back(id);
                s = s.substr(found + 1);
            }
            else
            {
                if(s.find("chosentype") != string::npos || s.find("Chosentype") != string::npos)
                {
                    s = source->chooseasubtype;
                }
                int id = MTGAllCards::findType(s);
                types.push_back(id);
                s = "";
            }
        }
    }

    int resolve()
    {
        if(!starfound.empty())
        {
            SAFE_DELETE(multiplier);
            multiplier = NEW WParsedInt(starfound, NULL, (MTGCardInstance *)source);
        }
        if(!spt.empty())
        {
            vector<string> powertoughness = split( spt, '/');
            WParsedInt * NewPow = NEW WParsedInt(powertoughness[0].c_str(),NULL,source);
            WParsedInt * NewTou = NEW WParsedInt(powertoughness[1].c_str(),NULL,source);
            power = NewPow->getValue();
            toughness = NewTou->getValue();
            SAFE_DELETE(NewPow);
            SAFE_DELETE(NewTou);
        }
        for (int i = 0; i < multiplier->getValue(); ++i)
        {
            //MTGCardInstance * myToken;
            if (tokenId)
            {
                MTGCard * card = MTGCollection()->getCardById(tokenId);
                setTokenOwner();
                myToken = NEW MTGCardInstance(card, tokenReciever->game);
            }
            else
            {
                myToken = NEW Token(name, source, power, toughness);
                list<int>::iterator it;
                for (it = types.begin(); it != types.end(); it++)
                {
                    myToken->addType(*it);
                }
                for (it = colors.begin(); it != colors.end(); it++)
                {
                    myToken->setColor(*it);
                }
                for (it = abilities.begin(); it != abilities.end(); it++)
                {
                    myToken->basicAbilities[*it] = 1;
                }
            }
            string tokenText = "";
            if(sabilities.find("token(") == string::npos)
            {
                tokenText = "(";
                size_t endAbility = sabilities.find(")");
                string words = sabilities.substr(0,endAbility);
                tokenText.append(words);
                string sourcename = ((MTGCardInstance*)source)->name;
                tokenText.append(") source: ");
                tokenText.append( sourcename);
                myToken->setText(tokenText);
            }
            setTokenOwner();
            tokenReciever->game->temp->addCard(myToken);
            Spell * spell = NEW Spell(game, myToken);
            spell->resolve();
            myToken = spell->source;
            spell->source->owner = tokenReciever;
            spell->source->isToken = 1;
            spell->source->fresh = 1;
            if(aLivingWeapon)
            {
                livingWeaponToken(spell->source);
            }
            if(battleReady)
            {
                battleReadyToken(spell->source);
            }
            delete spell;
        }
        return 1;
    }

    void setTokenOwner()
    {
        switch(who)
        {
        case TargetChooser::CONTROLLER:
            tokenReciever = source->controller();
            break;
        case TargetChooser::OPPONENT:
            tokenReciever = source->controller()->opponent();
            break;
        case TargetChooser::TARGET_CONTROLLER:
            if(target)
            {
                tokenReciever = ((MTGCardInstance*)target)->controller();
                break;
            }
        case TargetChooser::TARGETED_PLAYER:
            {
                tokenReciever = source->playerTarget;
                break;
            }
        default:
            tokenReciever = source->controller();
            break;
        }
    }
    
    void livingWeaponToken(MTGCardInstance * card)
    {
        for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
        {
            MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
            if (a->aType == MTGAbility::STANDARD_EQUIP && a->source == source)
            {
                AEquip* ae = dynamic_cast<AEquip*>(a);
                ae->unequip();
                ae->equip(card);  
            }
        }
    }

    void battleReadyToken(MTGCardInstance * card)
    {
        card->summoningSickness = 0;
        card->tap();
        card->setAttacker(1);
    }

    const char * getMenuText()
    {
        sprintf(menuText, "Create %s", name.c_str());
        return menuText;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "ATokenCreator ::: abilities : ?" // << abilities
                << " ; types : ?" // << types
                << " ; colors : ?" // << colors
                << " ; power : " << power << " ; toughness : " << toughness << " ; name : " << name << " ; who : " << who << " (";
        return ActivatedAbility::toString(out) << ")";
    }

    ATokenCreator * clone() const
    {
        ATokenCreator * a = NEW ATokenCreator(*this);
        a->multiplier = NEW WParsedInt(*(multiplier));
        return a;
    }

    ~ATokenCreator()
    {
        SAFE_DELETE(multiplier);
    }

};

//targetable abilities which are added to targeted players game.
class ATargetedAbilityCreator: public ActivatedAbility
{
public:
    string name;
    string sabilities;
    int who;
    MTGCardInstance * myDummy;
    Player * abilityReciever;
    ATargetedAbilityCreator(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,string _name, string abilityToAdd, int who = 0) :
    ActivatedAbility(observer, _id, _source, _cost, 0),name(_name),sabilities(abilityToAdd), who(who)
    {
    }

    int resolve()
    {
        setAbilityOwner();
        myDummy = NEW MTGCardInstance();
        setAbilityOwner();
        myDummy->setObserver(abilityReciever->getObserver());
        myDummy->owner = abilityReciever;
        myDummy->storedSourceCard = source;
        vector<string>magictextlines = split(sabilities,'_');
        if(magictextlines.size())
        {
            string newMagicText = "";
            for(unsigned int i = 0; i < magictextlines.size(); i++)
            {
                newMagicText.append(magictextlines[i]);
                newMagicText.append("\n");
            }
            myDummy->magicText = newMagicText;
        }
        else
        myDummy->magicText = sabilities;
        abilityReciever->game->garbage->addCard(myDummy);
        Spell * spell = NEW Spell(game, myDummy);
        spell->resolve();
        myDummy = spell->source;
        spell->source->owner = abilityReciever;
        delete spell;
        return 1;
    }

    void setAbilityOwner()
    {
        switch(who)
        {
        case TargetChooser::CONTROLLER:
            abilityReciever = source->controller();
            break;
        case TargetChooser::OPPONENT:
            abilityReciever = source->controller()->opponent();
            break;
        case TargetChooser::TARGET_CONTROLLER:
            if(target)
            {
                abilityReciever = ((MTGCardInstance*)target)->controller();
                break;
            }
        case TargetChooser::TARGETED_PLAYER:
            {
                abilityReciever = source->playerTarget;
                break;
            }
        default:
            abilityReciever = source->controller()->opponent();
            break;
        }
    }
    
    const char * getMenuText()
    {
        if(name.size())
            return name.c_str();
        return "Ability";
    }

    ATargetedAbilityCreator * clone() const
    {
        ATargetedAbilityCreator * a = NEW ATargetedAbilityCreator(*this);
        return a;
    }

    ~ATargetedAbilityCreator()
    {
    }

};
///
//a paired lord
class APaired: public MTGAbility, public NestedAbility
{
public:
    MTGAbility * a;
    MTGAbility * b;
    APaired(GameObserver* observer, int _id, MTGCardInstance * _source, Damageable * _target, MTGAbility * ability) :
        MTGAbility(observer, _id, _source, _target), NestedAbility(ability)
    {
        ability->source = source;
        ability->target = target;
        a = NULL;
    }

    int removeFromGame()
    {
        return removeAbilityFromGame();
    }

    int addToGame()
    {
        return MTGAbility::addToGame();
    }

    void Update(float dt)
    {
        resolve();
    }

    int resolve()
    {
        if (source->myPair)
        {
            addAbilityToGame();
        }
        else
        {
            removeAbilityFromGame();
        }
        if (ability->oneShot) a = NULL; //allows to call the effect several times
        return 1;
    }

    int addAbilityToGame()
    {
        if (a && b) return 0;
        a = ability->clone();
        b = ability->clone();
        a->source = source;
        a->target = source;
        b->source = source->myPair;
        b->target = source->myPair;
        if (a->oneShot)
        {
            a->resolve();
            b->resolve();
            SAFE_DELETE(a);
            SAFE_DELETE(b);
        }
        else
        {
            a->addToGame();
            b->addToGame();
        }
        return 1;
    }

    int destroy()
    {
        return removeAbilityFromGame();
    }

    int removeAbilityFromGame()
    {
        if (!a && !b) return 0;
        game->removeObserver(a);
        a = NULL;
        game->removeObserver(b);
        b = NULL;
        return 1;
    }

    ~APaired()
    {
        SAFE_DELETE(ability);
    }

    APaired * clone() const
    {
        APaired * a =  NEW APaired(*this);
        a->ability = ability->clone();
        return a;
    }
};

//Foreach (plague rats...)
class AForeach: public ListMaintainerAbility, public NestedAbility
{
public:
    int includeSelf;
    int mini;
    int maxi;
    map<Damageable *, MTGAbility *> abilities;
    AForeach(GameObserver* observer, int _id, MTGCardInstance * card, Damageable * _target, TargetChooser * _tc, int _includeSelf, MTGAbility * a,
            int mini = 0, int maxi = 0) :
        ListMaintainerAbility(observer, _id, card, _target), NestedAbility(a), mini(mini), maxi(maxi)
    {
        tc = _tc;
        tc->targetter = NULL;
        includeSelf = _includeSelf;
        ability->target = _target;
        aType = MTGAbility::FOREACH;
        naType = ability->aType;
    }

        int canBeInList(MTGCardInstance * card)
        {
            if(card->isPhased || source->isPhased)
                return 0;
            if ((includeSelf || card != source) && tc->canTarget(card)) return 1;
            return 0;
        }

    int added(MTGCardInstance * card)
    {
        if (mini && cards.size() <= (size_t) mini) return 0;
        if (maxi && cards.size() >= (size_t) maxi) return 0;

        MTGAbility * a = ability->clone();
        a->target = target;
        if (a->oneShot)
        {
            a->resolve();
            SAFE_DELETE(a);
        }
        else
        {
            a->addToGame();
            abilities[card] = a;
        }
        return 1;
    }

    int removed(MTGCardInstance * card)
    {
        if (abilities.find(card) != abilities.end())
        {
            game->removeObserver(abilities[card]);
            abilities.erase(card);
            return 1;
        }
        return 0;
    }

    const char * getMenuText()
    {
        return ability->getMenuText();
    }

    AForeach * clone() const
    {
        AForeach * a = NEW AForeach(*this);
        a->ability = ability->clone();
        return a;
    }

    int resolve()
    {
        //TODO check if ability is oneShot ?
        updateTargets();
        cards.clear();
        players.clear();
        return 1;
    }

    int checkActivation()
    {
        checkCards.clear();
        checkTargets();
        return checkCards.size();
    }

    ~AForeach()
    {
        SAFE_DELETE(ability);
    }

};

class AThis: public MTGAbility, public NestedAbility
{
public:
    MTGAbility * a;
    ThisDescriptor * td;
    AThis(GameObserver* observer, int _id, MTGCardInstance * _source, Damageable * _target, ThisDescriptor * _td, MTGAbility * ability) :
        MTGAbility(observer, _id, _source, _target), NestedAbility(ability)
    {
        td = _td;
        ability->source = source;
        ability->target = target;
        a = NULL;
        SAFE_DELETE(tc);
    }

    int removeFromGame()
    {
        return removeAbilityFromGame();
    }

    int addToGame()
    {
        return MTGAbility::addToGame();
    }

    void Update(float dt)
    {
        resolve();
    }

    int resolve()
    {
        //TODO check if ability is oneShot ?
        int match;
        match = td->match(source);
        if (match > 0)
        {
            addAbilityToGame();
        }
        else
        {
            removeAbilityFromGame();
        }
        if (ability->oneShot) a = NULL; //allows to call the effect several times
        return 1;
    }

    int addAbilityToGame()
    {
        if (a) return 0;
        a = ability->clone();
        if (a->oneShot)
        {
            a->resolve();
            SAFE_DELETE(a);
        }
        else
        {
            a->addToGame();
        }
        return 1;
    }

    int removeAbilityFromGame()
    {
        if (!a) return 0;
        game->removeObserver(a);
        a = NULL;
        return 1;
    }

    ~AThis()
    {
        SAFE_DELETE(ability);
        SAFE_DELETE(td);
    }

    AThis * clone() const
    {
        AThis * a =  NEW AThis(*this);
        a->ability = ability->clone();
        a->td = td->clone();
        return a;
    }
};

class AThisForEach: public MTGAbility, public NestedAbility
{
public:
    ThisDescriptor * td;
    vector<MTGAbility *> abilities;
    AThisForEach(GameObserver* observer, int _id, MTGCardInstance * _source, Damageable * _target, ThisDescriptor * _td, MTGAbility * ability) :
        MTGAbility(observer, _id, _source, _target), NestedAbility(ability)
    {
        td = _td;
        ability->source = source;
        ability->target = target;
        SAFE_DELETE(tc);
        aType = FOREACH;
    }

    int removeFromGame()
    {
        return removeAbilityFromGame();
    }

    int addToGame()
    {
        return MTGAbility::addToGame();
    }

    void Update(float dt)
    {
        resolve();
    }

    int resolve()
    {
        //TODO check if ability is oneShot ?
        int matches;
        matches = td->match(source);
        if (matches > 0)
        {
            if (abilities.size() > (size_t)matches)
            {
                removeAbilityFromGame();
            }
            // i will equal abilities size, then we increment from there
            //abilities size was previously being subtracted from matches
            //tho since it was a nonstatic number, it would stop adding abilities prematurely.
            for (size_t i = abilities.size(); i < (size_t)matches; i++)
            {
                addAbilityToGame();
            }
        }
        return 1;
    }

    int addAbilityToGame()
    {
        MTGAbility * a = ability->clone();
        a->target = target;
        if (a->oneShot)
        {
            a->resolve();
            SAFE_DELETE(a);
        }
        else
        {
            a->addToGame();
            abilities.push_back(a);
            //abilities[abilities.size()] = a;
        }
        return 1;
    }

    int removeAbilityFromGame()
    {
        for (int i = abilities.size(); i > 0; i--)
        {
            game->removeObserver(abilities[i - 1]);
        }
        abilities.clear();
        return 1;
    }

    ~AThisForEach()
    {
        SAFE_DELETE(ability);
        SAFE_DELETE(td);

        if (abilities.size())
        {
            removeAbilityFromGame();
        }
    }

    const char * getMenuText()
    {
        return ability->getMenuText();
    }

    AThisForEach * clone() const
    {
        AThisForEach * a =  NEW AThisForEach(*this);
        a->ability = ability->clone();
        a->td = td->clone();
        return a;
    }
};

//set a players hand size
class AASetHand: public ActivatedAbilityTP
{
public:
    int hand;

    AASetHand(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int hand, ManaCost * _cost = NULL,
            int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AASetHand * clone() const;

};

//lifeset
class AALifeSet: public ActivatedAbilityTP
{
public:
    WParsedInt * life;

    AALifeSet(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, WParsedInt * life, ManaCost * _cost = NULL,
            int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AALifeSet * clone() const;
    ~AALifeSet();

};

//lifesetend

class AADamager: public ActivatedAbilityTP
{
public:
    string d;
    bool redirected;

    AADamager(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, string d, ManaCost * _cost = NULL,
             int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    int getDamage();
    AADamager * clone() const;

};

//prevent next damage
class AADamagePrevent: public ActivatedAbilityTP
{
public:
    int preventing;

    AADamagePrevent(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int preventing, ManaCost * _cost = NULL, int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AADamagePrevent * clone() const;
    ~AADamagePrevent();
};

//poison removel
class AAAlterPoison: public ActivatedAbilityTP
{
public:
    int poison;

    AAAlterPoison(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int poison, ManaCost * _cost = NULL,
            int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AAAlterPoison * clone() const;
    ~AAAlterPoison();
};
/* Standard Damager, can choose a NEW target each time the price is paid */
class TADamager: public TargetAbility
{
public:

    TADamager(GameObserver* observer, int id, MTGCardInstance * card, ManaCost * _cost, string d, TargetChooser * _tc = NULL) :
        TargetAbility(observer, id, card, _tc, _cost, 0)
    {
        if (!tc) tc = NEW DamageableTargetChooser(game, card);
        ability = NEW AADamager(game, id, card, NULL, d);
    }

    TADamager * clone() const
    {
        return NEW TADamager(*this);
    }
};

/* Can tap a target for a cost */
class AATapper: public ActivatedAbility
{
public:
    AATapper(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AATapper * clone() const;
};

/* Can untap a target for a cost */
class AAUntapper: public ActivatedAbility
{
public:
    AAUntapper(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AAUntapper * clone() const;
};

/* set max level up on a levelup creature this is an Ai hint ability, no effect for players.*/
class AAWhatsMax: public ActivatedAbility
{
public:
    int value;

    AAWhatsMax(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * source, ManaCost * _cost = NULL, int value = 0);
    int resolve();
    AAWhatsMax * clone() const;
};

/* Can prevent a card from untapping next untap */
class AAFrozen: public ActivatedAbility
{
public:
    AAFrozen(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AAFrozen * clone() const;
};
/* ghetto new target*/
class AANewTarget: public ActivatedAbility
{
public:
bool retarget;
    AANewTarget(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target,bool retarget = false, ManaCost * _cost = NULL);
    int resolve();
    const char * getMenuText();
    AANewTarget * clone() const;
};
/* morph*/
class AAMorph: public ActivatedAbility
{
public:
    vector<MTGAbility *> currentAbilities;
    AAMorph(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost = NULL);
    int resolve();
    int testDestroy();
    const char * getMenuText();
    AAMorph * clone() const;
};
/* flip*/
class AAFlip: public InstantAbility
{
public:
    vector<MTGAbility *> currentAbilities;
    string flipStats;
    AAFlip(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target,string flipStats);
    int resolve();
    int testDestroy();
    const char * getMenuText();
    AAFlip * clone() const;
};
/* dynamic ability build*/
class AADynamic: public ActivatedAbility
{
public:
enum
{
DYNAMIC_SOURCE_AMOUNT = 1,
DYNAMIC_MYTGT_AMOUNT = 2,
DYNAMIC_MYSELF_AMOUNT = 3,
DYNAMIC_MYFOE_AMOUNT = 4,
DYNAMIC_NB_AMOUNT = 5,

DYNAMIC_ABILITY_TYPE_POWER = 0,
DYNAMIC_ABILITY_TYPE_TOUGHNESS = 1,
DYNAMIC_ABILITY_TYPE_MANACOST = 2,
DYNAMIC_ABILITY_TYPE_COLORS = 3,
DYNAMIC_ABILITY_TYPE_AGE = 4,
DYNAMIC_ABILITY_TYPE_CHARGE = 5,
DYNAMIC_ABILITY_TYPE_ONEONECOUNTERS = 6,
DYNAMIC_ABILITY_TYPE_THATMUCH = 7,
DYNAMIC_ABILITY_TYPE_NB = 8,

DYNAMIC_ABILITY_EFFECT_STRIKE = 0,
DYNAMIC_ABILITY_EFFECT_DRAW = 1,
DYNAMIC_ABILITY_EFFECT_LIFEGAIN = 2,
DYNAMIC_ABILITY_EFFECT_PUMPPOWER = 3,
DYNAMIC_ABILITY_EFFECT_PUMPTOUGHNESS = 4,
DYNAMIC_ABILITY_EFFECT_PUMPBOTH = 5,
DYNAMIC_ABILITY_EFFECT_LIFELOSS = 6,
DYNAMIC_ABILITY_EFFECT_DEPLETE = 7,
DYNAMIC_ABILITY_EFFECT_COUNTERSONEONE = 8,
DYNAMIC_ABILITY_EFFECT_NB = 9,

DYNAMIC_ABILITY_WHO_EACHOTHER = 1,
DYNAMIC_ABILITY_WHO_ITSELF = 2,
DYNAMIC_ABILITY_WHO_TARGETCONTROLLER = 3,
DYNAMIC_ABILITY_WHO_TARGETOPPONENT = 4,
DYNAMIC_ABILITY_WHO_TOSOURCE = 5,
DYNAMIC_ABILITY_WHO_SOURCECONTROLLER = 6,
DYNAMIC_ABILITY_WHO_SOURCEOPPONENT = 7,
DYNAMIC_ABILITY_WHO_NB = 8,

};
int type;
int effect;
int who;
int sourceamount;
int targetamount;
int amountsource;
bool eachother;
bool tosrc;
MTGCardInstance * OriginalSrc;
MTGCardInstance * storedTarget;
MTGAbility * storedAbility;
MTGAbility * clonedStored;
MTGAbility * mainAbility;
string menu;

    AADynamic(GameObserver* observer, int id, MTGCardInstance * card, Damageable * _target,int type = 0,int effect = 0,int who = 0,int amountsource = 1,MTGAbility * storedAbility = NULL, ManaCost * _cost = NULL);
    int resolve();
    int activateMainAbility(MTGAbility * toActivate,MTGCardInstance * source , Damageable * target);
    int activateStored();
    const char * getMenuText();
    AADynamic * clone() const;
    ~AADynamic();
};

/* switch power and toughness of target */
class ASwapPT: public InstantAbility
{
public:
    int oldpower;
    int oldtoughness;
    ASwapPT(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
        InstantAbility(observer, _id, _source, _target)
    {
        target = _target;
    }

    int resolve()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (_target)
        {
            while (_target->next)
                _target = _target->next; //This is for cards such as rampant growth
            oldpower = _target->power;
            oldtoughness = _target->toughness;

            _target->addToToughness(oldpower);
            _target->addToToughness(-oldtoughness);
            _target->power = oldtoughness;

        }
        return 1;
    }

    int destroy()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (_target)
        {
            while (_target->next)
                _target = _target->next; //This is for cards such as rampant growth
            oldpower = _target->power;
            oldtoughness = _target->toughness;

            _target->addToToughness(oldpower);
            _target->addToToughness(-oldtoughness);
            _target->power = oldtoughness;

        }
        return 1;
    }

    const char * getMenuText()
    {
        return "Swap power and toughness";
    }
    ASwapPT * clone() const
    {
        return NEW ASwapPT(*this);
    }
};

class AAExchangeLife: public ActivatedAbilityTP
{
public:
    AAExchangeLife(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost = NULL,
             int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AAExchangeLife * clone() const;

};

// Add life of gives damage if a given zone has more or less than [condition] cards at the beginning of [phase]
//Ex : the rack, ivory tower...
class ALifeZoneLink: public MTGAbility
{
public:
    int phase;
    int condition;
    int life;
    int controller;
    int nbcards;
    MTGGameZone * zone;
    ALifeZoneLink(GameObserver* observer, int _id, MTGCardInstance * card, int _phase, int _condition, int _life = -1, int _controller = 0,
            MTGGameZone * _zone = NULL) :
        MTGAbility(observer, _id, card)
    {
        phase = _phase;
        condition = _condition;
        controller = _controller;
        life = _life;
        zone = _zone;
        if (zone == NULL)
        {
            if (controller)
            {
                zone = game->currentPlayer->game->hand;
            }
            else
            {
                zone = game->opponent()->game->hand;
            }
        }
    }

    void Update(float dt)
    {
        if (newPhase != currentPhase && newPhase == phase)
        {
            if ((controller && game->currentPlayer == source->controller()) || (!controller && game->currentPlayer
                    != source->controller()))
            {
                if ((condition < 0 && zone->nb_cards < -condition) || (condition > 0 && zone->nb_cards > condition))
                {
                    int diff = zone->nb_cards - condition;
                    if (condition < 0) diff = -condition - zone->nb_cards;
                    if (life > 0)
                    {
                        game->currentPlayer->life += life * diff;
                    }
                    else
                    {
                        game->mLayers->stackLayer()->addDamage(source, game->currentPlayer, -life * diff);
                    }
                }
            }
        }
    }
    virtual ostream& toString(ostream& out) const
    {
        out << "ALifeZoneLink ::: phase : " << phase << " ; condition : " << condition << " ; life : " << life
                << " ; controller : " << controller << " ; nbcards : " << nbcards << " (";
        return MTGAbility::toString(out) << ")";
    }
    ALifeZoneLink * clone() const
    {
        return NEW ALifeZoneLink(*this);
    }
};

//Creatures that cannot attack if opponent has not a given type of land, and die if controller has not this type of land
//Ex : pirate ship...
class AStrongLandLinkCreature: public MTGAbility
{
public:
    char land[20];
    AStrongLandLinkCreature(GameObserver* observer, int _id, MTGCardInstance * _source, const char * _land) :
        MTGAbility(observer, _id, _source)
    {
        sprintf(land, "%s", _land);
    }

    void Update(float dt)
    {
        if (newPhase != currentPhase && (newPhase == MTG_PHASE_COMBATBEGIN || newPhase == MTG_PHASE_COMBATATTACKERS))
        {
            if (source->controller()->opponent()->game->inPlay->hasType(land))
            {
                source->basicAbilities[(int)Constants::CANTATTACK] = 0;
            }
            else
            {
                source->basicAbilities[(int)Constants::CANTATTACK] = 1;
            }
        }
        Player * player = source->controller();
        if (!player->game->inPlay->hasType(land))
        {
            player->game->putInGraveyard(source);
        }
    }
    virtual ostream& toString(ostream& out) const
    {
        out << "AStrongLandLinkCreature ::: land : " << land << " (";
        return MTGAbility::toString(out) << ")";
    }
    AStrongLandLinkCreature * clone() const
    {
        return NEW AStrongLandLinkCreature(*this);
    }
};

//Steal control of a target
class AControlStealAura: public MTGAbility
{
public:
    Player * originalController;
    AControlStealAura(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
        MTGAbility(observer, _id, _source, _target)
    {
        originalController = _target->controller();
        MTGCardInstance * copy = _target->changeController(game->currentlyActing());
        target = copy;
        source->target = copy;
    }

    int destroy()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        Player * p = _target->controller();
        if (p && p->game->inPlay->hasCard(_target))
        { //if the target is still in game -> spell was destroyed
            _target->changeController(originalController);
        }
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AControlStealAura ::: originalController : " << originalController << " (";
        return MTGAbility::toString(out) << ")";
    }
    AControlStealAura * clone() const
    {
        return NEW AControlStealAura(*this);
    }
};
//bloodthirst ability------------------------------------------
class ABloodThirst: public MTGAbility
{
public:
    int amount;
    ABloodThirst(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int amount) :
        MTGAbility(observer, id, source, target), amount(amount)
    {
    }

    int addToGame()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        for (int i = 0; i < amount; i++)
        {
            if (_target->controller()->opponent()->damaged() > 0)
            {
                _target->counters->addCounter(1, 1);
            }
        }
        return MTGAbility::addToGame();
    }

    ABloodThirst * clone() const
    {
        return NEW ABloodThirst(*this);
    }

    ~ABloodThirst()
    {
    }
};

//reduce or increase manacost of target by color:amount------------------------------------------
class AAlterCost: public MTGAbility
{
public:
MTGCardInstance * manaReducer;
    int amount;
    int tempAmount;
    int type;
    AAlterCost(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int amount, int type);
    int addToGame();
    int testDestroy();
    void refreshCost(MTGCardInstance * card = NULL);
    void increaseTheCost(MTGCardInstance * card = NULL);
    void decreaseTheCost(MTGCardInstance * card = NULL);
    AAlterCost * clone() const;
    ~AAlterCost();
};

//------------------------------------
class ATransformer: public MTGAbility
{
public:
    list<int> abilities;
    list<int> types;
    list<int> colors;
    list<int> oldcolors;
    list<int> oldtypes;
    vector<int> dontremove;
    bool addNewColors;
    bool remove;
	bool removeCreatureSubtypes;
    bool removeTypes;
    string menu;
    string newpower;
    bool newpowerfound;
    int oldpower;
    string newtoughness;
    bool newtoughnessfound;
    int oldtoughness;
    map<Damageable *, vector<MTGAbility *> > newAbilities;
    vector<string> newAbilitiesList;
    bool newAbilityFound;
    bool aForever;
    bool UYNT;
    int myCurrentTurn;

    ATransformer(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, string sabilities,string newpower,bool newpowerfound,string newtoughness,bool newtoughnessfound,vector<string> newAbilitiesList,bool newAbilityFound = false,bool aForever = false ,bool UYNT = false);
    int addToGame();
    int reapplyCountersBonus(MTGCardInstance * rtarget= NULL,bool powerapplied=false,bool toughnessapplied=false);
    int testDestroy();
    int destroy();
    const char * getMenuText();
    ATransformer * clone() const;
    ~ATransformer();
};

//Adds types/abilities/changes color to a card (generally until end of turn)
class ATransformerInstant: public InstantAbility
{
public:
    ATransformer * ability;
    string newpower;
    bool newpowerfound;
    string newtoughness;
    bool newtoughnessfound;
    vector<string> newAbilitiesList;
    map<Damageable *, vector<MTGAbility *> > newAbilities;
    bool newAbilityFound;
    bool aForever;
    bool UYNT;

    ATransformerInstant(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string types = "", string abilities = "",string newpower = "",bool newpowerfound = false,string newtoughness = "",bool newtoughnessfound = false,vector<string>newAbilitiesList = vector<string>(),bool newAbilityFound = false,bool aForever = false, bool UYNT = false);
    int resolve();
    const char * getMenuText();
    ATransformerInstant * clone() const;
    ~ATransformerInstant();
};

//Adds types/abilities/changes color to a card (generally until end of turn)
class PTInstant: public InstantAbility
{
public:
    APowerToughnessModifier * ability;
    WParsedPT * wppt;
    string s;
    bool nonstatic;
    WParsedPT * newWppt;
    PTInstant(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, WParsedPT * wppt,string s = "",bool nonstatic = false);
    int resolve();
    const char * getMenuText();
    PTInstant * clone() const;
    ~PTInstant();
};

//ExaltedAbility (Shards of Alara)
class AExalted: public TriggeredAbility
{
public:
    int power, toughness;
    MTGCardInstance * luckyWinner;
    AExalted(GameObserver* observer, int _id, MTGCardInstance * _source, int _power = 1, int _toughness = 1) :
        TriggeredAbility(observer, _id, _source), power(_power), toughness(_toughness)
    {
        luckyWinner = NULL;
    }

    int triggerOnEvent(WEvent * event)
    {
        if (WEventPhaseChange* pe = dynamic_cast<WEventPhaseChange*>(event))
        {
            if (luckyWinner && MTG_PHASE_AFTER_EOT == pe->from->id)
            {
                luckyWinner = NULL;
            }

            if (MTG_PHASE_COMBATATTACKERS == pe->from->id)
            {
                int nbattackers = 0;
                MTGGameZone * z = source->controller()->game->inPlay;
                int nbcards = z->nb_cards;
                for (int i = 0; i < nbcards; ++i)
                {
                    MTGCardInstance * c = z->cards[i];
                    if (c->attacker)
                    {
                        nbattackers++;
                        luckyWinner = c;
                    }
                }
                if (nbattackers == 1)
                    return 1;
                else
                    luckyWinner = NULL;
            }
        }
        return 0;
    }

    int resolve()
    {
        if (!luckyWinner) return 0;
        PTInstant * a = NEW PTInstant(game, this->GetId(), source, luckyWinner,NEW WParsedPT(1,1));
        GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source,luckyWinner, a);
        wrapper->addToGame();
        luckyWinner = NULL;
        return 1;
    }

    const char * getMenuText()
    {
        return "Exalted";
    }

    AExalted * clone() const
    {
        return NEW AExalted(*this);
    }
};

//switch p/t ueot
class ASwapPTUEOT: public InstantAbility
{
public:
    ASwapPT * ability;
    ASwapPTUEOT(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target);
    int resolve();
    const char * getMenuText();
    ASwapPTUEOT * clone() const;
    ~ASwapPTUEOT();
};

class APreventDamageTypes: public MTGAbility
{
public:
    string to, from;
    REDamagePrevention * re;
    int type;

    APreventDamageTypes(GameObserver* observer, int id, MTGCardInstance * source, string to, string from, int type = 0);
    int addToGame();
    int destroy();
    APreventDamageTypes * clone() const;
    ~APreventDamageTypes();
};
//prevent counters
class ACounterShroud: public MTGAbility
{
public:
    TargetChooser * csTc;
    Counter * counter;
    RECountersPrevention * re;
    ACounterShroud(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target,TargetChooser * tc, Counter * counter = NULL);
    int addToGame();
    int destroy();
    ACounterShroud * clone() const;
    ~ACounterShroud();
};
//track an effect using counters.
class ACounterTracker: public MTGAbility
{
public:
    string scounter;
    int removed;
    ACounterTracker(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string scounter = "");
    int addToGame();
    int destroy();
    int testDestroy();
    ACounterTracker * clone() const;
    ~ACounterTracker();
};
//Remove all abilities from target
class ALoseAbilities: public MTGAbility
{
public:
    vector <MTGAbility *> storedAbilities;
    ALoseAbilities(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target);
    int addToGame();
    int destroy();
    ALoseAbilities * clone() const;
};

//Remove subtypes (of a given type) from target
class ALoseSubtypes: public MTGAbility
{
public:
    int parentType;
    vector <int> storedSubtypes;
    ALoseSubtypes(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int parentType);
    int addToGame();
    int destroy();
    ALoseSubtypes * clone() const;
};

//Adds types/abilities/P/T to a card (until end of turn)
class APreventDamageTypesUEOT: public InstantAbility
{
public:
    APreventDamageTypes * ability;
    vector<APreventDamageTypes *> clones;
    int type;
    APreventDamageTypesUEOT(GameObserver* observer, int id, MTGCardInstance * source, string to, string from, int type = 0);
    int resolve();
    int destroy();
    const char * getMenuText();
    APreventDamageTypesUEOT * clone() const;
    ~APreventDamageTypesUEOT();
};

//vanishing
class AVanishing: public MTGAbility
{
public:
    int timeLeft;
    int amount;
    string counterName;
    int next;

    AVanishing(GameObserver* observer, int _id, MTGCardInstance * card, ManaCost * _cost, int restrictions = 0,int amount = 0,string counterName = "");
    void Update(float dt);
    int resolve();
    const char * getMenuText();
    AVanishing * clone() const;
    ~AVanishing();
};

//Upkeep Cost
class AUpkeep: public ActivatedAbility, public NestedAbility
{
public:
    int paidThisTurn;
    int phase;
    int once;
    bool Cumulative;
    int currentage;
    ManaCost * backupMana;

    AUpkeep(GameObserver* observer, int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int restrictions = 0, int _phase =
        MTG_PHASE_UPKEEP, int _once = 0,bool Cumulative = false);
    int receiveEvent(WEvent * event);
    void Update(float dt);
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int resolve();
    const char * getMenuText();
    virtual ostream& toString(ostream& out) const;
    AUpkeep * clone() const;
    ~AUpkeep();
};

//phase based actions
class APhaseAction: public MTGAbility
{
public:
    string psMenuText;
    int abilityId;
    string sAbility;
    int phase;
    MTGAbility * ability;
    bool forcedestroy;
    bool next;
    bool myturn;
    bool opponentturn;
    bool once;
    Player * abilityOwner;

    APhaseAction(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * target, string sAbility, int restrictions = 0, int _phase =
        MTG_PHASE_UPKEEP,bool forcedestroy = false,bool next = true,bool myturn = true,bool opponentturn = true,bool once = false);
    void Update(float dt);
    int resolve();
    const char * getMenuText();
    APhaseAction * clone() const;
    ~APhaseAction();
};

//Adds types/abilities/P/T to a card (until end of turn)
class APhaseActionGeneric: public InstantAbility
{
public:
    string sAbility;
    APhaseAction * ability;
    APhaseActionGeneric(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * target, string sAbility, int restrictions = 0, int _phase =
            MTG_PHASE_UPKEEP,bool forcedestroy = false,bool next = true,bool myturn = false,bool opponentturn = false,bool once = false);
    int resolve();
    const char * getMenuText();
    APhaseActionGeneric * clone() const;
    ~APhaseActionGeneric();

};

//ABlink
class ABlink: public MTGAbility
{
public:
    bool blinkueot;
    bool blinkForSource;
    bool blinkhand;
    MTGCardInstance * Blinked;
    bool resolved;
    MTGAbility * stored;
    ABlink(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target,bool blinkueot=false,bool blinkForSource = false,bool blinkhand = false,MTGAbility * stored = NULL);
    void Update(float dt);
    void resolveBlink();
    int resolve();
    const char * getMenuText();
    ABlink * clone() const;
    ~ABlink();
};

//blinkinstant
class ABlinkGeneric: public InstantAbility
{
public:
    bool blinkueot;
    bool blinkForSource;
    bool blinkhand;
    ABlink * ability;
    MTGAbility * stored;
    ABlinkGeneric(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target,bool blinkueot=false,bool blinkForSource = false,bool blinkhand = false,MTGAbility * stored = NULL);
    int resolve();
    const char * getMenuText();
    ABlinkGeneric * clone() const;
    ~ABlinkGeneric();

};

/*
 Specific Classes
 */

// 1092 Specific to Aladdin's Lamp
class AAladdinsLamp: public TargetAbility
{
public:
    CardDisplay cd;
    int nbcards;
    int init;

    AAladdinsLamp(GameObserver* observer, int id, MTGCardInstance * card) :
        TargetAbility(observer, id, card), cd(1, game, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, NULL)
    {
        setCost(NEW ManaCost(), true);
        getCost()->x();
        int zones[] = { MTGGameZone::MY_LIBRARY };
        tc = NEW TargetZoneChooser(game, zones, 1, source);
        nbcards = 0;
        init = 0;
    }

    void Update(float dt)
    {
        if (waitingForAnswer)
        {
            if (!init)
            {
                cd.resetObjects();
                int wished = game->currentlyActing()->getManaPool()->getConvertedCost();
                game->currentlyActing()->getManaPool()->pay(getCost());
                nbcards = 0;
                MTGGameZone * library = game->currentlyActing()->game->library;
                while (nbcards < wished)
                {
                    cd.AddCard(library->cards[library->nb_cards - 1 - nbcards]);
                    nbcards++;
                }
                init = 1;
                Render(dt);
            }
            cd.Update(dt);

            //      cd.CheckUserInput(dt);
        }
    }

    void Render(float dt)
    {
        if (waitingForAnswer)
        {
            cd.Render();
        }
    }

    int fireAbility()
    {
        source->tap();
        MTGLibrary * library = game->currentlyActing()->game->library;
                MTGHand * hand = game->currentlyActing()->game->hand;
        MTGCardInstance * card = library->removeCard(tc->getNextCardTarget());
        //library->shuffleTopToBottom(nbcards - 1);
        hand->addCard(card);
        init = 0;
        return 1;
    }

    int resolve()
    {
        return 1;
    }
    ;

    virtual ostream& toString(ostream& out) const
    {
        out << "AAladdinsLamp ::: cd : " << cd << " ; nbcards  : " << nbcards << " ; init : " << init << " (";
        return TargetAbility::toString(out) << ")";
    }
    AAladdinsLamp * clone() const
    {
        return NEW AAladdinsLamp(*this);
    }
};

// Armageddon Clock
class AArmageddonClock: public MTGAbility
{
public:
    int counters;
    ManaCost cost;
    AArmageddonClock(GameObserver* observer, int id, MTGCardInstance * _source) :
        MTGAbility(observer, id, _source)
    {
        counters = 0;
        std::vector<int16_t> _cost;
        _cost.push_back(Constants::MTG_COLOR_ARTIFACT);
        _cost.push_back(4);
        cost = ManaCost(_cost, 1);
    }

    void Update(float dt)
    {
        if (newPhase != currentPhase)
        {
            if (newPhase == MTG_PHASE_UPKEEP && game->currentPlayer->game->inPlay->hasCard(source))
            {
                counters++;
            }
            else if (newPhase == MTG_PHASE_DRAW && counters > 0 && game->currentPlayer->game->inPlay->hasCard(source))
            { //End of upkeep = beginning of draw
                game->mLayers->stackLayer()->addDamage(source, game->players[0],
                        counters);
                game->mLayers->stackLayer()->addDamage(source, game->players[1],
                        counters);
            }
        }
    }
    int isReactingToClick(MTGCardInstance * _card, ManaCost * mana = NULL)
    {
        if (counters > 0 && _card == source && currentPhase == MTG_PHASE_UPKEEP)
        {
            if (game->currentlyActing()->getManaPool()->canAfford(&cost))
            {
                return 1;
            }
        }
        return 0;
    }

    int reactToClick(MTGCardInstance * _card)
    {
        if (!isReactingToClick(_card)) return 0;
        game->currentlyActing()->getManaPool()->pay(&cost);
        counters--;
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AArmageddonClock ::: counters : " << counters << " ; cost : " << cost << " (";
        return MTGAbility::toString(out) << ")";
    }
    AArmageddonClock * clone() const
    {
        return NEW AArmageddonClock(*this);
    }
};

//1102: Conservator
class AConservator: public MTGAbility
{
public:
    int canprevent;
    ManaCost cost;
    AConservator(GameObserver* observer, int _id, MTGCardInstance * _source) :
        MTGAbility(observer, _id, _source)
    {
        canprevent = 0;
        std::vector<int16_t> _cost;
        _cost.push_back(Constants::MTG_COLOR_ARTIFACT);
        _cost.push_back(2);
        cost = ManaCost(_cost, 1);
    }

    int alterDamage(Damage * damage)
    {
        if (canprevent && damage->target == source->controller())
        {
            if (damage->damage >= canprevent)
            {
                damage->damage -= canprevent;
                canprevent = 0;
            }
            else
            {
                canprevent -= damage->damage;
                damage->damage = 0;
            }
        }
        return 1;
    }
    int alterDamage()
    {
        if (canprevent)
        {
            ActionStack * stack = game->mLayers->stackLayer();
            for (int i = stack->mObjects.size() - 1; i >= 0; i--)
            {
                if (!canprevent) return 1;
                Interruptible * current = ((Interruptible *) stack->mObjects[i]);
                if (current->type == ACTION_DAMAGE && current->state == NOT_RESOLVED)
                {
                    Damage * damage = (Damage *) current;
                    alterDamage(damage);
                }
                else if (current->type == ACTION_DAMAGES && current->state == NOT_RESOLVED)
                {
                    DamageStack * damages = (DamageStack *) current;
                    for (int j = damages->mObjects.size() - 1; j >= 0; j--)
                    {
                        alterDamage(((Damage *) damages->mObjects[j]));
                    }
                }
            }
        }
        return 1;
    }

    void Update(float dt)
    {
        alterDamage();
    }

    int isReactingToClick(MTGCardInstance * _card, ManaCost * mana = NULL)
    {
        if (_card == source && game->currentlyActing()->game->inPlay->hasCard(source) && !_card->isTapped())
        {
            if (game->currentlyActing()->getManaPool()->canAfford(&cost))
            {
                return 1;
            }
        }
        return 0;
    }

    int reactToClick(MTGCardInstance * _card)
    {
        if (!isReactingToClick(_card)) return 0;
        game->currentlyActing()->getManaPool()->pay(&cost);
        source->tap();
        canprevent = 2;
        alterDamage();
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AConservator ::: canprevent : " << canprevent << " ; cost : " << cost << " (";
        return MTGAbility::toString(out) << ")";
    }
    AConservator * clone() const
    {
        return NEW AConservator(*this);
    }
};

//Kjeldoran Frostbeast
class AKjeldoranFrostbeast: public MTGAbility
{
public:
    MTGCardInstance * opponents[20];
    int nbOpponents;
    AKjeldoranFrostbeast(GameObserver* observer, int _id, MTGCardInstance * _source) :
        MTGAbility(observer, _id, _source)
    {
        nbOpponents = 0;
    }

    void Update(float dt)
    {
        if (newPhase != currentPhase)
        {
            if (newPhase == MTG_PHASE_COMBATEND)
            {
                nbOpponents = 0;
                MTGCardInstance * opponent = source->getNextOpponent();
                while (opponent)
                {
                    opponents[nbOpponents] = opponent;
                    nbOpponents++;
                    opponent = source->getNextOpponent(opponent);
                }
                if (source->isInPlay(game))
                {
                    for (int i = 0; i < nbOpponents; i++)
                    {
                        opponents[i]->destroy();
                    }
                }
            }
        }
    }

    int testDestroy()
    {
        if (!game->isInPlay(source) && currentPhase != MTG_PHASE_UNTAP)
        {
            return 0;
        }
        else
        {
            return MTGAbility::testDestroy();
        }
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AKjeldoranFrostbeast ::: opponents : " << opponents << " ; nbOpponents : " << nbOpponents << " (";
        return MTGAbility::toString(out) << ")";
    }
    AKjeldoranFrostbeast * clone() const
    {
        return NEW AKjeldoranFrostbeast(*this);
    }
};

//1143 Animate Dead
class AAnimateDead: public MTGAbility
{
public:
    AAnimateDead(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
        MTGAbility(observer, _id, _source, _target)
    {
        MTGCardInstance * card = _target;

        //Put the card in play again, with all its abilities !
        //AbilityFactory af;
        MTGCardInstance * copy = source->controller()->game->putInZone(card, _target->controller()->game->graveyard,
                source->controller()->game->temp);
        Spell * spell = NEW Spell(game, copy);
        spell->resolve();
        target = spell->source;
        card = spell->source;
        card->power--;
        card->life = card->toughness;
        delete spell;
    }

    int destroy()
    {
        MTGCardInstance * card = (MTGCardInstance *) target;
        card->power++;
        card->controller()->game->putInZone(card, card->controller()->game->inPlay, card->owner->game->graveyard);
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AAnimateDead ::: (";
        return MTGAbility::toString(out) << ")";
    }
    AAnimateDead * clone() const
    {
        return NEW AAnimateDead(*this);
    }
};

//1159 Erg Raiders
class AErgRaiders: public MTGAbility
{
public:
    int attackedThisTurn;
    AErgRaiders(GameObserver* observer, int _id, MTGCardInstance * _source) :
        MTGAbility(observer, _id, _source)
    {
        attackedThisTurn = 1;
    }

        void Update(float dt)
        {
            if (newPhase != currentPhase)
            {
                Player * controller = source->controller();
                if(newPhase == MTG_PHASE_ENDOFTURN)
                {
                    if(!attackedThisTurn && game->currentPlayer == source->controller() && !source->fresh)
                        game->mLayers->stackLayer()->addDamage(source, controller, 2);
                }
                else if (newPhase == MTG_PHASE_UNTAP)
                {

                    if (game->currentPlayer == controller)
                    {
                        attackedThisTurn = 0;
                    }
                }
            }
        }
    
    int receiveEvent(WEvent * event)
    {
        WEventCardAttacked * attacked = dynamic_cast<WEventCardAttacked *> (event);
        if (attacked && !attacked->card->didblocked && attacked->card == source)
        {
            attackedThisTurn = 1;
            return 1;
        }
        return 0;
    }

    AErgRaiders * clone() const
    {
        return NEW AErgRaiders(*this);
    }
};

//Fastbond
class AFastbond: public TriggeredAbility
{
public:

    TargetChooser * counter;
    MaxPerTurnRestriction * landsRestriction;
    int landsPlayedThisTurn;
    AFastbond(GameObserver* observer, int _id, MTGCardInstance * card) :
        TriggeredAbility(observer, _id, card)
    {

        counter = NEW TypeTargetChooser(game, "land");
        landsPlayedThisTurn = source->controller()->game->inPlay->seenThisTurn(counter, Constants::CAST_ALL);
        PlayRestrictions * restrictions = source->controller()->game->playRestrictions;
        landsRestriction = restrictions->getMaxPerTurnRestrictionByTargetChooser(counter);
        restrictions->removeRestriction(landsRestriction);

    }

    void Update(float dt)
    {
        if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP)
        {
            landsPlayedThisTurn = 0;
        }
        TriggeredAbility::Update(dt);
    }

    int trigger()
    {
        int landsPlayedThisTurnUpdated = source->controller()->game->inPlay->seenThisTurn(counter, Constants::CAST_ALL);
        if (landsPlayedThisTurnUpdated > 1 && landsPlayedThisTurnUpdated > landsPlayedThisTurn)
        {
            landsPlayedThisTurn = landsPlayedThisTurnUpdated;
            return 1;
        }
        return 0;
    }

    int resolve()
    {
        game->mLayers->stackLayer()->addDamage(source, source->controller(), 1);
        game->mLayers->stackLayer()->resolve();
        return 1;
    }

    int destroy()
    {
        PlayRestrictions  * restrictions = source->controller()->game->playRestrictions;
        if(restrictions->getMaxPerTurnRestrictionByTargetChooser(counter))
            return 1;

        restrictions->addRestriction(landsRestriction);
            return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        return TriggeredAbility::toString(out) << ")";
    }
    AFastbond * clone() const
    {
        return NEW AFastbond(*this);
    }

    ~AFastbond()
    {
        delete counter;
    }
};

//1117 Jandor's Ring
class AJandorsRing: public ActivatedAbility
{
public:
    AJandorsRing(GameObserver* observer, int _id, MTGCardInstance * _source) :
        ActivatedAbility(observer, _id, _source, NEW ManaCost())
    {
        getCost()->add(Constants::MTG_COLOR_ARTIFACT, 2);
    }

    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL)
    {
        if (!source->controller()->game->hand->hasCard(source->controller()->game->library->lastCardDrawn)) return 0;
        return ActivatedAbility::isReactingToClick(card, mana);
    }

    int resolve()
    {
        source->controller()->game->putInGraveyard(source->controller()->game->library->lastCardDrawn);
        game->mLayers->stackLayer()->addDraw(source->controller());
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AJandorsRing ::: (";
        return ActivatedAbility::toString(out) << ")";
    }
    AJandorsRing * clone() const
    {
        return NEW AJandorsRing(*this);
    }
};

//Power Leak
class APowerLeak: public TriggeredAbility
{
public:
    int damagesToDealThisTurn;
    ManaCost cost;
    APowerLeak(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
        TriggeredAbility(observer, _id, _source, _target)
    {
        cost.add(Constants::MTG_COLOR_ARTIFACT, 1);
        damagesToDealThisTurn = 0;
    }

    void Update(float dt)
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (newPhase != currentPhase && newPhase == MTG_PHASE_UPKEEP && _target->controller() == game->currentPlayer)
        {
            damagesToDealThisTurn = 2;
        }
        TriggeredAbility::Update(dt);
    }

    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL)
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (damagesToDealThisTurn && currentPhase == MTG_PHASE_UPKEEP && card == source && _target->controller()
                == game->currentPlayer)
        {
            if (game->currentPlayer->getManaPool()->canAfford(&cost)) return 1;
        }
        return 0;
    }

    int reactToclick(MTGCardInstance * card)
    {
        game->currentPlayer->getManaPool()->pay(&cost);
        damagesToDealThisTurn--;
        return 1;
    }

    int trigger()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (newPhase != currentPhase && newPhase == MTG_PHASE_DRAW && _target->controller() == game->currentPlayer)
        {
            if (damagesToDealThisTurn) return 1;
        }
        return 0;
    }

    int resolve()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        game->mLayers->stackLayer()->addDamage(source, _target->controller(), damagesToDealThisTurn);
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "APowerLeak ::: damagesToDealThisTurn : " << damagesToDealThisTurn << " ; cost : " << cost << " (";
        return TriggeredAbility::toString(out) << ")";
    }
    APowerLeak * clone() const
    {
        return NEW APowerLeak(*this);
    }
};

//1176 Sacrifice
class ASacrifice: public InstantAbility
{
public:
    ASacrifice(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
        InstantAbility(observer, _id, _source)
    {
        target = _target;
    }

    int resolve()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (_target->isInPlay(game))
        {
            game->currentlyActing()->game->putInGraveyard(_target);
            int x = _target->getManaCost()->getConvertedCost();
            game->currentlyActing()->getManaPool()->add(Constants::MTG_COLOR_BLACK, x);
        }
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "ASacrifice ::: (";
        return InstantAbility::toString(out) << ")";
    }
    ASacrifice * clone() const
    {
        return NEW ASacrifice(*this);
    }
};

//1288 EarthBind
class AEarthbind: public ABasicAbilityModifier
{
public:
    AEarthbind(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
        ABasicAbilityModifier(observer, _id, _source, _target, Constants::FLYING, 0)
    {
        if (value_before_modification)
        {
            Damageable * _target = (Damageable *) target;
            game->mLayers->stackLayer()->addDamage(source, _target, 2);
        }
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AEarthbind ::: (";
        return ABasicAbilityModifier::toString(out) << ")";
    }
    AEarthbind * clone() const
    {
        return NEW AEarthbind(*this);
    }
};

//1291 Fireball
class AFireball: public InstantAbility
{
public:
    AFireball(GameObserver* observer, int _id, MTGCardInstance * card, Spell * spell, int x) :
        InstantAbility(observer, _id, card)
    {
        int nbtargets = spell->getNbTargets();
        int totaldamage = x + 1 - nbtargets;
        int individualdamage = 0;
        if (nbtargets) individualdamage = totaldamage / nbtargets;
        Damageable * _target = spell->getNextDamageableTarget();
        while (_target)
        {
            game->mLayers->stackLayer()->addDamage(source, _target, individualdamage);
            _target = spell->getNextDamageableTarget(_target);
        }
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AFireball ::: (";
        return InstantAbility::toString(out) << ")";
    }
    AFireball * clone() const
    {
        return NEW AFireball(*this);
    }
};

//1351 Island Sanctuary
class AIslandSanctuary: public MTGAbility
{
public:
    int initThisTurn;
    vector<MTGCardInstance*> effectedCards;
    AIslandSanctuary(GameObserver* observer, int _id, MTGCardInstance * _source) :
        MTGAbility(observer, _id, _source)
    {
        initThisTurn = 0;
    }

    void Update(float dt)
    {
        if (currentPhase == MTG_PHASE_UNTAP && game->currentPlayer == source->controller())
        {
            initThisTurn = 0;
            for(unsigned int i = 0; i < effectedCards.size(); i++)
            effectedCards.at(i)->basicAbilities[(int)Constants::CANTATTACK] = 0;
            effectedCards.clear();
        }
        if (initThisTurn && currentPhase == MTG_PHASE_COMBATBEGIN && game->currentPlayer != source->controller())
        {
            MTGGameZone * zone = game->currentPlayer->game->inPlay;
            for (int i = 0; i < zone->nb_cards; i++)
            {
                MTGCardInstance * card = zone->cards[i];
                if (!card->has(Constants::FLYING) && !card->has(Constants::ISLANDWALK) && !card->has(Constants::CANTATTACK))
                {
                    card->basicAbilities[(int)Constants::CANTATTACK] = 1;
                    effectedCards.push_back(card);
                }
            }
        }
    }

    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL)
    {
        if (card == source && game->currentPlayer == card->controller())
        {
        
            Interruptible * action = game->mLayers->stackLayer()->getAt(-1);
            AADrawer * draw = dynamic_cast <AADrawer *> (action);
            if (draw && draw->aType == MTGAbility::STANDARD_DRAW) 
            return 1;
        }
        return 0;
    }

    int reactToClick(MTGCardInstance * card)
    {
        if (!isReactingToClick(card)) return 0;
        game->mLayers->stackLayer()->Remove(game->mLayers->stackLayer()->getAt(-1));
        initThisTurn = 1;
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AIslandSanctuary ::: initThisTurn : " << initThisTurn << " (";
        return MTGAbility::toString(out) << ")";
    }
    AIslandSanctuary * clone() const
    {
        return NEW AIslandSanctuary(*this);
    }
};

//remove or add a phase.
class APhaseAlter: public TriggeredAbility
{
public:
    Targetable * targetPlayerWho;
    bool adding;
    bool applied;
    Player * who;
    string phaseToAlter;
    int phasetoAlter;
    string targetingString;
    string after;
    bool aNext;
    APhaseAlter(GameObserver* observer, int _id, MTGCardInstance * card,Targetable * targetPlayer, bool _adding,string _phaseToAlter,string targeting, bool _aNext,string _after = "") :
        TriggeredAbility(observer, _id, card),targetPlayerWho(targetPlayer),adding(_adding),phaseToAlter(_phaseToAlter),targetingString(targeting),after(_after),aNext(_aNext)
    {
            applied = false;
            who = NULL;
            phasetoAlter = PhaseRing::phaseStrToInt(phaseToAlter);
    }

        int triggerOnEvent(WEvent * event)
        {
            if(!who)
            {
                Player * targetedPlayer = dynamic_cast<Player*>(target);
                if (targetingString.find("targetedplayer") != string::npos && targetedPlayer)
                {
                    who = targetedPlayer;
                }
                if (targetingString.find("controller") != string::npos)
                    who = source->controller();
                if (targetingString.find("opponent") != string::npos)
                    who = source->controller()->opponent();
                if (targetingString.find("targetcontroller") != string::npos)
                    who = source->target?source->target->controller():source->controller();
                if (targetingString.find("owner") != string::npos)
                    who = source->owner;
            }

            if (after == "this")//apply it right now.
            {
                if(!applied)
                    if (who == game->currentPlayer)
                    {
                        after = game->phaseRing->phaseIntToStr(game->oldGamePhase);
                        addTheEffect(game->currentPlayer->getId());
                        return 1;
                    }
            }

            if (WEventPhasePreChange* pe = dynamic_cast<WEventPhasePreChange*>(event))
            {
                if (MTG_PHASE_CLEANUP == pe->to->id)
                {
                    if( aNext && applied && who != game->currentPlayer)
                    {
                        forceDestroy = 1;
                    }
                }
                if(adding)
                {
                    if(!applied)
                        if (PhaseRing::phaseStrToInt(after) == pe->to->id && who == game->currentPlayer)
                        {
                            pe->eventChanged = true;
                            return 1;
                        }
                }
                else if (PhaseRing::phaseStrToInt(phaseToAlter) == pe->to->id && who == game->currentPlayer)
                {
                    pe->eventChanged = true;
                    return 1;
                }
            }
            return 0;
        }

    int resolve()
    {
        for (int i = 0; i < 2; i++)
        {
            if(who == game->players[i] && game->currentPlayer == game->players[i])
            {
                addTheEffect(i);
            }
        }
        return 1;
    }
    void addTheEffect(int i)
    {
        int turnSteps = game->phaseRing->turn.size();
        if(adding && !applied)
        {
            if(phaseToAlter == "combatphases")
            {
                game->phaseRing->addCombatAfter(game->players[i], PhaseRing::phaseStrToInt(after));
            }
            else if(phaseToAlter == "combatphaseswithmain")
            {
                game->phaseRing->addCombatAfter(game->players[i], PhaseRing::phaseStrToInt(after),true);
            }
            else
                game->phaseRing->addPhaseAfter(PhaseRing::phaseStrToInt(phaseToAlter), game->players[i], PhaseRing::phaseStrToInt(after));
        }
        else if(!adding)
            game->phaseRing->removePhase(PhaseRing::phaseStrToInt(phaseToAlter));
        int turnAfterChange = game->phaseRing->turn.size();
        if(turnSteps != turnAfterChange)
            applied = true;
        return;
    }

    void removeTheEffect(int i)//readd this!!!!!!!!!!!!!
    {
        if(applied)
        {
            if(adding)
                game->phaseRing->removePhase(PhaseRing::phaseStrToInt(phaseToAlter));
            else
                game->phaseRing->addPhaseAfter(PhaseRing::phaseStrToInt(phaseToAlter), game->players[i], PhaseRing::phaseStrToInt(after));
            applied = false;
        }
        return;
    }

    int testDestroy()
    {
        if(forceDestroy != -1)
            return 1;
        if(!(source->hasType(Subtypes::TYPE_INSTANT)||source->hasType(Subtypes::TYPE_INSTANT)) && !source->isInPlay(game))
            return 1;
        return 0;
    }

    int destroy()
    {
        for (int i = 0; i < 2; i++)
        {
            if(who == game->players[i] && game->currentPlayer == game->players[i])
            {
                removeTheEffect(i);
            }
        }
        return 1;
    }

    const char * getMenuText()
    {
        return "phase alter";
    }

    APhaseAlter * clone() const
    {
        return NEW APhaseAlter(*this);
    }
};
//Generic Millstone
class AADepleter: public ActivatedAbilityTP
{
public:
    string nbcardsStr;

    AADepleter(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost = NULL,
            int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AADepleter * clone() const;
};

//Generic skip turn/extra turn
class AAModTurn: public ActivatedAbilityTP
{
public:
    string nbTurnStr;

    AAModTurn(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbTurnStr, ManaCost * _cost = NULL,
            int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AAModTurn * clone() const;
};

//Shuffle
class AAShuffle: public ActivatedAbilityTP
{
public:
    AAShuffle(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost = NULL, int who =
            TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AAShuffle * clone() const;
};

//Remove Mana From ManaPool
class AARemoveMana: public ActivatedAbilityTP
{
public:
    ManaCost * mManaDesc;
    bool mRemoveAll;

    AARemoveMana(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, string ManaDesc, int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AARemoveMana * clone() const;
    ~AARemoveMana();

};

//Random Discard
class AARandomDiscarder: public ActivatedAbilityTP
{
public:
    string nbcardsStr;

    AARandomDiscarder(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost = NULL,
             int who = TargetChooser::UNSET);
    int resolve();
    const char * getMenuText();
    AARandomDiscarder * clone() const;
};

//Rampage ability
class ARampageAbility: public MTGAbility
{
public:
    int nbOpponents;
    int PowerModifier;
    int ToughnessModifier;
    int MaxOpponent;

    ARampageAbility(GameObserver* observer, int _id, MTGCardInstance * _source, int _PowerModifier, int _ToughnessModifier, int _MaxOpponent) :
        MTGAbility(observer, _id, _source)
    {
        PowerModifier = _PowerModifier;
        ToughnessModifier = _ToughnessModifier;
        MaxOpponent = _MaxOpponent;
        nbOpponents = 0;
    }
    int receiveEvent(WEvent * event)
    {
        if (dynamic_cast<WEventBlockersChosen*> (event))
        {
            nbOpponents = source->blockers.size();
            if (nbOpponents <= MaxOpponent) return 0;
            source->power += PowerModifier * (nbOpponents - MaxOpponent);
            source->addToToughness(ToughnessModifier * (nbOpponents - MaxOpponent));
        }
        else if (WEventPhaseChange* pe = dynamic_cast<WEventPhaseChange*>(event))
        {
            if (MTG_PHASE_AFTER_EOT == pe->to->id && nbOpponents > MaxOpponent)
            {
                source->power -= PowerModifier * (nbOpponents - MaxOpponent);
                source->addToToughness(-ToughnessModifier * (nbOpponents - MaxOpponent));
                nbOpponents = 0;
            }
        }
        return 1;
    }

    ARampageAbility * clone() const
    {
        return NEW ARampageAbility(*this);
    }
};

//Evole ability
class AEvolveAbility: public MTGAbility
{
public:
    AEvolveAbility(GameObserver* observer, int _id, MTGCardInstance * _source) :
        MTGAbility(observer, _id, _source)
    {
    }
    int receiveEvent(WEvent * event)
    {
       WEventZoneChange * enters = (WEventZoneChange *) event;
       if (enters->to == game->currentlyActing()->game->inPlay && game->currentlyActing() == source->controller() && enters->card->isCreature())
        {
            if(enters->card != source && (enters->card->power > source->power || enters->card->toughness > source->toughness))
            {
                source->counters->addCounter(1,1);
            }
        }
        return 1;
    }

    AEvolveAbility * clone() const
    {
        return NEW AEvolveAbility(*this);
    }
};

//flanking ability
class AFlankerAbility: public MTGAbility
{
public:
    MTGCardInstance * opponents[20];
    int nbOpponents;
    AFlankerAbility(GameObserver* observer, int _id, MTGCardInstance * _source) :
        MTGAbility(observer, _id, _source)
    {
        nbOpponents = 0;
    }
    int receiveEvent(WEvent * event)
    {
        if (dynamic_cast<WEventBlockersChosen*> (event))
        {
            nbOpponents = 0;
            MTGCardInstance * opponent = source->getNextOpponent();
            while (opponent && !opponent->has(Constants::FLANKING) && game->currentlyActing() == source->controller()->opponent())
            {
                opponents[nbOpponents] = opponent;
                nbOpponents++;
                opponent = source->getNextOpponent(opponent);
            }
            for (int i = 0; i < nbOpponents; i++)
            {
                opponents[i]->power -= 1;
                opponents[i]->addToToughness(-1);
                opponents[i]->flanked += 1;
                if (opponents[i]->life == 0)
                {
                    opponents[i]->setPower(0);
                }
            }
        }
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AFlankerAbility ::: opponents : " << opponents << " ; nbOpponents : " << nbOpponents << " (";
        return MTGAbility::toString(out) << ")";
    }

    AFlankerAbility * clone() const
    {
        return NEW AFlankerAbility(*this);
    }
};

//Bushido ability
class ABushidoAbility: public MTGAbility
{
public:
    string PowerToughnessModifier;

    ABushidoAbility(GameObserver* observer, int _id, MTGCardInstance * _source, string _PowerToughnessModifier) :
        MTGAbility(observer, _id, _source)
    {
        PowerToughnessModifier = _PowerToughnessModifier;
    }
        int receiveEvent(WEvent * event)
        {
            if (dynamic_cast<WEventBlockersChosen*> (event))
            {
                MTGCardInstance * opponent = source->getNextOpponent();
                if (!opponent) return 0;
                PTInstant * a = NEW PTInstant(game, this->GetId(), source, source,NEW WParsedPT(PowerToughnessModifier,NULL,source));
                GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source,source, a);
                wrapper->addToGame();
            }
            return 1;
        }

    ABushidoAbility * clone() const
    {
        return NEW ABushidoAbility(*this);
    }
};

//A Spirit Link Ability
class ASpiritLinkAbility: public MTGAbility
{
public:
    MTGCardInstance * source;
    bool combatonly;
    ASpiritLinkAbility(GameObserver* observer, int _id, MTGCardInstance * _source,bool combatonly = false) :
    MTGAbility(observer, _id, _source),source(_source),combatonly(combatonly)
    {
    }
    int receiveEvent(WEvent * event)
    {
        if (event->type == WEvent::DAMAGE)
        {
            WEventDamage * e = (WEventDamage *) event;
            Damage * d = e->damage;
            if (combatonly && e->damage->typeOfDamage != DAMAGE_COMBAT) 
                return 0;
            MTGCardInstance * card = d->source;
            if (d->damage > 0 && card && (card == source || card == source->target))
            {
                source->owner->gainLife(d->damage);
                return 1;
            }
        }
        return 0;
    }
    ASpiritLinkAbility * clone() const
    {
        return NEW ASpiritLinkAbility(*this);
    }
};

//Instant Steal control of a target
class AInstantControlSteal: public InstantAbility
{
public:
    Player * TrueController;
    Player * TheftController;
    AInstantControlSteal(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
        InstantAbility(observer, _id, _source, _target)
    {

    }

        int resolve()
        {
            MTGCardInstance * _theftTarget = (MTGCardInstance*)target;
            bool recast = false;
            if(!_theftTarget->isInPlay(game))
            {
                recast = true;
            }
            while(_theftTarget->next)
            {
                _theftTarget= _theftTarget->next;
            }
            if(_theftTarget)
            {
                TrueController = _theftTarget->controller();
                TheftController = source->controller();
                MTGCardInstance * copy = _theftTarget->changeController(TheftController);
                target = copy;
                source->target = copy;
                copy->summoningSickness = 0;
                if(recast)
                {
                    Spell * spell = NEW Spell(game, copy);
                    spell->resolve();
                    SAFE_DELETE(spell);
                }
            }
            return 1;
        }

    int destroy()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (TheftController && TheftController->game->inPlay->hasCard(_target))
        { //if the target is still in game -> spell was destroyed
            _target->changeController(TrueController);

        }
        return 1;
    }

    virtual ostream& toString(ostream& out) const
    {
        out << "AInstantControlSteal ::: TrueController : " << TrueController << " ; TheftController : " << TheftController << " (";
        return InstantAbility::toString(out) << ")";
    }

    AInstantControlSteal * clone() const
    {
        return NEW AInstantControlSteal(*this);
    }
};

//----------------------------------------------
class AASetColorChosen: public InstantAbility
{
public:
    int color;
    string abilityToAlter;
    MTGAbility * abilityAltered;
    AASetColorChosen(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int _color = 0 ,string toAdd = "");
    int resolve();
    const char* getMenuText();
    AASetColorChosen * clone() const;
    ~AASetColorChosen();
};
class AASetTypeChosen: public InstantAbility
{
public:
    int type;
    string abilityToAlter;
    string menutext;
    MTGAbility * abilityAltered;
    AASetTypeChosen(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int _type = 0,string menu = "error" ,string toAdd = "");
    int resolve();
    const char* getMenuText();
    AASetTypeChosen * clone() const;
    ~AASetTypeChosen();
};
class GenericChooseTypeColor: public ActivatedAbility
{
public:
    string baseAbility;
    bool chooseColor;
    AASetColorChosen * setColor;
    AASetTypeChosen * setType;
    bool ANonWall;
    GenericChooseTypeColor(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target, string toAdd = "",bool chooseColor = false,bool nonwall = false, ManaCost * cost = NULL);
    int resolve();
    const char* getMenuText();
    GenericChooseTypeColor * clone() const;
    ~GenericChooseTypeColor();

};
//------------------------------------------------
//flip a coin and call it, with win or lose abilities
class AASetCoin: public InstantAbility
{
public:
    int side;
    string abilityToAlter;
    string abilityWin;
    string abilityLose;
    MTGAbility * abilityAltered;
    AASetCoin(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int side = -1,string toAdd = "");
    int resolve();
    const char* getMenuText();
    AASetCoin * clone() const;
    ~AASetCoin();
};
class GenericFlipACoin: public ActivatedAbility
{
public:
    string baseAbility;
    bool chooseColor;
    AASetCoin * setCoin;
    GenericFlipACoin(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target, string toAdd = "", ManaCost * cost = NULL);
    int resolve();
    const char* getMenuText();
    GenericFlipACoin * clone() const;
    ~GenericFlipACoin();

};
// utility functions

void PopulateColorIndexVector(list<int>& colors, const string& colorsString, char delimiter = ',');
void PopulateAbilityIndexVector(list<int>& abilities, const string& abilitiesString, char delimiter = ',');
void PopulateSubtypesIndexVector(list<int>& types, const string& subtypesString, char delimiter = ' ');

#endif
