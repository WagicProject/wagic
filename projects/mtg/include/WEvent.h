#ifndef _WEVENT_H_
#define _WEVENT_H_

class MTGCardInstance;
class MTGGameZone;
class Damage;
class Phase;
class Targetable;
class ManaPool;

class WEvent{
public:
  enum{
    NOT_SPECIFIED = 0,
    CHANGE_ZONE = 1,
    DAMAGE = 2,
    CHANGE_PHASE = 3,
  };
  int type; //Deprecated, use dynamic casting instead
  WEvent(int type = NOT_SPECIFIED);
  virtual ~WEvent() {};
};

struct WEventZoneChange: public WEvent{
  MTGCardInstance * card;
  MTGGameZone * from;
  MTGGameZone * to;
  WEventZoneChange(MTGCardInstance * card, MTGGameZone * from, MTGGameZone *to);
  virtual ~WEventZoneChange() {};
};


struct WEventDamage: public WEvent{
  Damage * damage;
  WEventDamage(Damage * damage);
};

struct WEventPhaseChange: public WEvent{
  Phase * from;
  Phase * to;
  WEventPhaseChange(Phase * from, Phase * to);
};


//Abstract class of event when a card's status changes
struct WEventCardUpdate: public WEvent{
  MTGCardInstance * card;
  WEventCardUpdate(MTGCardInstance * card);
};

//Event when a card is tapped/untapped
struct WEventCardTap: public WEventCardUpdate{
  bool before;
  bool after;
  WEventCardTap(MTGCardInstance * card, bool before, bool after);
};

//Event when a card's "attacker" status changes
//before:Player/Planeswalker that card was attacking previously
//after: Player/Planeswalker that card is attacking now
struct WEventCreatureAttacker: public WEventCardUpdate{
  Targetable * before;
  Targetable * after;
  WEventCreatureAttacker(MTGCardInstance * card, Targetable * from, Targetable * to);
};

//Event when a card's "defenser" status changes
//before : attacker that card was blocking previously
//after: attacker that card is blocking now
struct WEventCreatureBlocker: public WEventCardUpdate{
  MTGCardInstance * before;
  MTGCardInstance * after;
  WEventCreatureBlocker(MTGCardInstance * card,MTGCardInstance * from,MTGCardInstance * to);
};

//Event when a blocker is reordered
//exchangeWith: exchange card's position with exchangeWith's position
//attacker:both card and exchangeWith *should* be in attacker's "blockers" list.
struct WEventCreatureBlockerRank: public WEventCardUpdate{
  MTGCardInstance * exchangeWith;
  MTGCardInstance * attacker;
  WEventCreatureBlockerRank(MTGCardInstance * card,MTGCardInstance * exchangeWith, MTGCardInstance * attacker);

};

//Event when a mana is engaged
//color : color
struct WEventEngageMana : public WEvent {
  int color;
  MTGCardInstance* card;
  ManaPool * destination;
  WEventEngageMana(int color, MTGCardInstance* card, ManaPool * destination);
};

//Event when a mana is consumed
//color : color
struct WEventConsumeMana : public WEvent {
  int color;
  ManaPool * source;
  WEventConsumeMana(int color, ManaPool * source);
};

//Event when a manapool is emptied
//color : color
struct WEventEmptyManaPool : public WEvent {
  ManaPool * source;
  WEventEmptyManaPool(ManaPool * source);
};

#endif
