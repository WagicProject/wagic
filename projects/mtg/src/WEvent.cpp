#include "../include/WEvent.h"
#include "../include/MTGCardInstance.h"
#include "../include/MTGGameZones.h"
#include "../include/Damage.h"
#include "../include/PhaseRing.h"

WEvent::WEvent(int type) : type(type){}

WEventZoneChange::WEventZoneChange(MTGCardInstance * card, MTGGameZone * from, MTGGameZone *to) : WEvent(CHANGE_ZONE), card(card), from(from), to(to){}

WEventDamage::WEventDamage(Damage *damage) : WEvent(DAMAGE), damage(damage){}

WEventCardUpdate::WEventCardUpdate(MTGCardInstance * card) : WEvent(), card(card) {};

WEventPhaseChange::WEventPhaseChange(Phase * from, Phase * to) : WEvent(CHANGE_PHASE), from(from), to(to){}

WEventCardTap::WEventCardTap(MTGCardInstance * card, bool before, bool after) : WEventCardUpdate(card), before(before), after(after){}

WEventCreatureAttacker::WEventCreatureAttacker(MTGCardInstance * card, Targetable * before, Targetable * after) : WEventCardUpdate(card), before(before), after(after){}

WEventCreatureBlocker::WEventCreatureBlocker(MTGCardInstance * card, MTGCardInstance * from,MTGCardInstance * to) : WEventCardUpdate(card), before(before), after(after){}

WEventCreatureBlockerRank::WEventCreatureBlockerRank(MTGCardInstance * card, MTGCardInstance * exchangeWith, MTGCardInstance * attacker) : WEventCardUpdate(card), exchangeWith(exchangeWith), attacker(attacker){}

WEventEngageMana::WEventEngageMana(int color, MTGCardInstance* card) : WEvent(), color(color), card(card) {}
WEventConsumeMana::WEventConsumeMana(int color) : WEvent(), color(color) {}
