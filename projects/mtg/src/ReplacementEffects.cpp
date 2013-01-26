#include "PrecompiledHeader.h"

#include "ReplacementEffects.h"
#include "MTGCardInstance.h"
#include "TargetChooser.h"
#include "AllAbilities.h"

REDamagePrevention::REDamagePrevention(MTGAbility * source, TargetChooser *tcSource, TargetChooser *tcTarget, int damage,
                bool oneShot, int typeOfDamage) :
    source(source), tcSource(tcSource), tcTarget(tcTarget), damage(damage), oneShot(oneShot), typeOfDamage(typeOfDamage)
{
}

WEvent * REDamagePrevention::replace(WEvent *event)
{
    if (!event) return event;
    if (!damage) return event;
    WEventDamage * e = dynamic_cast<WEventDamage*> (event);
    if (!e) return event;
    Damage *d = e->damage;
    if (d->typeOfDamage != typeOfDamage && typeOfDamage != DAMAGE_ALL_TYPES) return event;
    if ((!tcSource || tcSource->canTarget(d->source)) && (!tcTarget || tcTarget->canTarget(d->target)))
    {
        if (damage == -1)
        {
            d->damage = 0;
            delete event;
            if (oneShot) damage = 0;
            return NULL;
        }
        if (damage >= d->damage)
        {
            damage -= d->damage;
            d->damage = 0;
            delete event;
            return NULL;
        }
        d->damage -= damage;
        damage = 0;
        delete event;
        WEventDamage* newEvent = NEW WEventDamage(d);
        return newEvent;
    }
    return event;
}
REDamagePrevention::~REDamagePrevention()
{
    SAFE_DELETE(tcSource);
    SAFE_DELETE(tcTarget);
}
//counters replacement effect///////////////////
RECountersPrevention::RECountersPrevention(MTGAbility * source,MTGCardInstance * cardSource,MTGCardInstance * cardTarget,TargetChooser * tc,Counter * counter) :
    source(source),cardSource(cardSource),cardTarget(cardTarget),TargetingCards(tc),counter(counter)
{
}

    WEvent * RECountersPrevention::replace(WEvent *event)
    {
        if (!event) return event;
        WEventCounters * e = dynamic_cast<WEventCounters*> (event);
        if (!e) return event;
        if((MTGCardInstance*)e->targetCard)
        {
            if((MTGCardInstance*)e->targetCard == cardSource && counter)
            {
                if(e->power == counter->power && e->toughness == counter->toughness && e->name == counter->name)
                    return event = NULL;
            }
            else if((MTGCardInstance*)e->targetCard == cardSource)
                return event = NULL;
            else if(TargetingCards && TargetingCards->canTarget((MTGCardInstance*)e->targetCard))
                return event = NULL;
        }
        return event;
    }
RECountersPrevention::~RECountersPrevention()
{
    SAFE_DELETE(TargetingCards);
}
////--draw replacement---------------------
REDrawReplacement::REDrawReplacement(MTGAbility * source, Player * Drawer, MTGAbility * replaceWith) :
    source(source), DrawerOfCard(Drawer),replacementAbility(replaceWith)
{
}

WEvent * REDrawReplacement::replace(WEvent *event)
{
    if (!event) return event;
    WEventDraw * e = dynamic_cast<WEventDraw*> (event);
    if (!e) return event;
    if (DrawerOfCard != e->player) return event;
    if(!replacementAbility) return event;
    //check for dredge
    TargetChooserFactory tf(e->player->getObserver());
    TargetChooser * tcb = NULL;
    tcb = tf.createTargetChooser("dredgeable",source->source);
    tcb->targetter = NULL;
    if(tcb->validTargetsExist())
    {
        if(e->player == DrawerOfCard && e->player == source->source->controller())
        {
            SAFE_DELETE(tcb);
            return event;
        }
    }
    SAFE_DELETE(tcb);

    vector<MTGAbility*>selection;
    //look for other draw replacement effects
    list<ReplacementEffect *>::iterator it;
    GameObserver * game = source->source->getObserver();
    if(replacementAbility->source->controller() == DrawerOfCard)
    for (it = game->replacementEffects->modifiers.begin(); it != game->replacementEffects->modifiers.end(); it++)
    {
        if(REDrawReplacement * DR = dynamic_cast<REDrawReplacement *>(*it))
        {
            MTGAbility * otherA = NULL;
            if(DR->DrawerOfCard == e->player)
            {
                if(DR->replacementAbility->oneShot)
                    selection.push_back(DR->replacementAbility->clone());
                else
                {
                    otherA = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), source->source,NULL,DR->replacementAbility->clone());
                    selection.push_back(otherA);
                }
            }
        }
    }

    
    for(int j = 0; j < e->nb_cards; j++)
    {
        if(e->player != DrawerOfCard || selection.size() < 2)
        {
            MTGAbility * toResolve = replacementAbility->clone();
            if(replacementAbility->oneShot)
                toResolve->resolve();
            else
                toResolve->addToGame();
        }
        else
        {
            MTGAbility * menuChoice = NEW MenuAbility(game, 1, source->source, source->source,true,selection,NULL,"Choose Draw Replacement");
            menuChoice->addToGame();
        }
    }

    delete event;
    event = NULL;
    return event;
}
REDrawReplacement::~REDrawReplacement()
{
}
//////////////////////////////////////////////
ReplacementEffects::ReplacementEffects()
{
}

WEvent * ReplacementEffects::replace(WEvent *e)
{
    list<ReplacementEffect *>::iterator it;

    for (it = modifiers.begin(); it != modifiers.end(); it++)
    {
        ReplacementEffect *re = *it;
        WEvent * newEvent = re->replace(e);
        if (!newEvent) return NULL;
        if (newEvent != e) return replace(newEvent);
    }
    return e;
}

int ReplacementEffects::add(ReplacementEffect * re)
{
    modifiers.push_back(re);
    return 1;
}

int ReplacementEffects::remove(ReplacementEffect *re)
{
    modifiers.remove(re);
    return 1;
}

ReplacementEffects::~ReplacementEffects()
{
    list<ReplacementEffect *>::iterator it;
    for (it = modifiers.begin(); it != modifiers.end(); it++)
    {
        ReplacementEffect *re = *it;
        delete (re);
    }
    modifiers.clear();
}
