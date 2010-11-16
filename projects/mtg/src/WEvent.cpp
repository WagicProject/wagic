#include "PrecompiledHeader.h"

#include "WEvent.h"
#include "MTGCardInstance.h"
#include "MTGGameZones.h"
#include "Damage.h"
#include "PhaseRing.h"

WEvent::WEvent(int type) :
    type(type)
{
}

WEventZoneChange::WEventZoneChange(MTGCardInstance * card, MTGGameZone * from, MTGGameZone *to) :
    WEvent(CHANGE_ZONE), card(card), from(from), to(to)
{
}

WEventDamage::WEventDamage(Damage *damage) :
    WEvent(DAMAGE), damage(damage)
{
}

WEventDamageStackResolved::WEventDamageStackResolved() :
    WEvent()
{
}

WEventCardUpdate::WEventCardUpdate(MTGCardInstance * card) :
    WEvent(), card(card)
{
}
;

WEventPhaseChange::WEventPhaseChange(Phase * from, Phase * to) :
    WEvent(CHANGE_PHASE), from(from), to(to)
{
}

WEventCardTap::WEventCardTap(MTGCardInstance * card, bool before, bool after) :
    WEventCardUpdate(card), before(before), after(after)
{
}

WEventCardTappedForMana::WEventCardTappedForMana(MTGCardInstance * card, bool before, bool after) :
    WEventCardUpdate(card), before(before), after(after)
{
}

WEventCardAttacked::WEventCardAttacked(MTGCardInstance * card) :
    WEventCardUpdate(card)
{
}

WEventCardAttackedAlone::WEventCardAttackedAlone(MTGCardInstance * card) :
    WEventCardUpdate(card)
{
}

WEventCardAttackedNotBlocked::WEventCardAttackedNotBlocked(MTGCardInstance * card) :
    WEventCardUpdate(card)
{
}

WEventCardAttackedBlocked::WEventCardAttackedBlocked(MTGCardInstance * card) :
    WEventCardUpdate(card)
{
}

WEventCardBlocked::WEventCardBlocked(MTGCardInstance * card) :
    WEventCardUpdate(card)
{
}

WEventcardDraw::WEventcardDraw(Player * player, int nb_cards) :
    player(player), nb_cards(nb_cards)
{
}

WEventCardSacrifice::WEventCardSacrifice(MTGCardInstance * card) :
    WEventCardUpdate(card)
{
}

WEventCardDiscard::WEventCardDiscard(MTGCardInstance * card) :
    WEventCardUpdate(card)
{
}

WEventCardChangeType::WEventCardChangeType(MTGCardInstance * card, int type, bool before, bool after) :
    WEventCardUpdate(card), type(type), before(before), after(after)
{
}

WEventCreatureAttacker::WEventCreatureAttacker(MTGCardInstance * card, Targetable * before, Targetable * after) :
    WEventCardUpdate(card), before(before), after(after)
{
}

WEventCreatureBlocker::WEventCreatureBlocker(MTGCardInstance * card, MTGCardInstance * from, MTGCardInstance * to) :
    WEventCardUpdate(card), before(from), after(to)
{
}

WEventCreatureBlockerRank::WEventCreatureBlockerRank(MTGCardInstance * card, MTGCardInstance * exchangeWith,
                MTGCardInstance * attacker) :
    WEventCardUpdate(card), exchangeWith(exchangeWith), attacker(attacker)
{
}

WEventEngageMana::WEventEngageMana(int color, MTGCardInstance* card, ManaPool * destination) :
    WEvent(), color(color), card(card), destination(destination)
{
}
WEventConsumeMana::WEventConsumeMana(int color, ManaPool * source) :
    WEvent(), color(color), source(source)
{
}
WEventEmptyManaPool::WEventEmptyManaPool(ManaPool * source) :
    WEvent(), source(source)
{
}

WEventCombatStepChange::WEventCombatStepChange(CombatStep step) :
    WEvent(), step(step)
{
}
;

Targetable * WEventDamage::getTarget(int target)
{
    switch (target)
    {
    case TARGET_TO:
        return damage->target;
    case TARGET_FROM:
        return damage->source;
    }
    return NULL;
}

int WEventDamage::getValue()
{
    return damage->damage;
}

Targetable * WEventZoneChange::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventCardAttacked::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventCardAttackedAlone::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventCardSacrifice::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventCardDiscard::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventCardAttackedNotBlocked::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventCardAttackedBlocked::getTarget(int target)
{
    switch (target)
    {
    case TARGET_TO:
        return card;
    case TARGET_FROM:
        return card->getNextOpponent();
    }
    return NULL;
}

Targetable * WEventCardBlocked::getTarget(int target)
{
    switch (target)
    {
    case TARGET_TO:
        return card;
    case TARGET_FROM:
        return card->getNextOpponent();
    }
    return NULL;
}

Targetable * WEventCardTap::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventCardTappedForMana::getTarget(int target)
{
    if (target) return card;
    return NULL;
}

Targetable * WEventcardDraw::getTarget(Player * player)
{
    if (player) return player;
    return NULL;
}

std::ostream& WEvent::toString(std::ostream& out) const
{
    return out << "EVENT";
}
std::ostream& WEventZoneChange::toString(std::ostream& out) const
{
    return out << "ZONEEVENT " << *card << " : " << *from << " -> " << *to;
}
std::ostream& WEventDamage::toString(std::ostream& out) const
{
    if (MTGCardInstance* m = dynamic_cast<MTGCardInstance*>(damage->target))
        return out << "DAMAGEEVENT " << damage->damage << " >> " << *m;
    else
        return out << "DAMAGEEVENT " << damage->damage << " >> " << damage->target;
}
std::ostream& operator<<(std::ostream& out, const WEvent& m)
{
    return m.toString(out);
}
