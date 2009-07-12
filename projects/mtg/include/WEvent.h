#ifndef _WEVENT_H_
#define _WEVENT_H_

class MTGCardInstance;
class MTGGameZone;
class Damage;
class Phase;
class Targetable;

class WEvent{
public:
  enum{
    NOT_SPECIFIED = 0,
    CHANGE_ZONE = 1,
    DAMAGE = 2,
    CHANGE_PHASE = 3,
  };
  int type; //Deprecated, use dynamic casting instead
  WEvent(int _type = NOT_SPECIFIED);
  virtual ~WEvent() {};
};

class WEventZoneChange: public WEvent{
public:
  MTGCardInstance * card;
  MTGGameZone * from;
  MTGGameZone * to;
  WEventZoneChange(MTGCardInstance * _card, MTGGameZone * _from, MTGGameZone *_to);
  virtual ~WEventZoneChange() {};
};


class WEventDamage: public WEvent{
public:
  Damage * damage;
  WEventDamage(Damage * _damage);
};

class WEventPhaseChange: public WEvent{
public:
  Phase * from;
  Phase * to;
  WEventPhaseChange(Phase * _from, Phase * _to);
};


//Abstract class of event when a card's status changes
class WEventCardUpdate: public WEvent{
public:
  MTGCardInstance * card;
  WEventCardUpdate(MTGCardInstance * card):WEvent(),card(card){};
};

//Event when a card is tapped/untapped
class WEventCardTap: public WEventCardUpdate{
public:
  bool before;
  bool after;
  WEventCardTap(MTGCardInstance * card, bool before, bool after);
};

//Event when a card's "attacker" status changes
//before:Player/Planeswalker that card was attacking previously
//after: Player/Planeswalker that card is attacking now
class WEventCreatureAttacker: public WEventCardUpdate{
public:
  Targetable * before;
  Targetable * after;
  WEventCreatureAttacker(MTGCardInstance * card,Targetable * from, Targetable * to);

};

//Event when a card's "defenser" status changes
//before : attacker that card was blocking previously
//after: attacker that card is blocking now
class WEventCreatureBlocker: public WEventCardUpdate{
public:
  MTGCardInstance * before;
  MTGCardInstance * after;
  WEventCreatureBlocker(MTGCardInstance * card,MTGCardInstance * from,MTGCardInstance * to);

};

//Event when a blocker is reordered
//exchangeWith: exchange card's position with exchangeWith's position
//attacker:both card and exchangeWith *should* be in attacker's "blockers" list.
class WEventCreatureBlockerRank: public WEventCardUpdate{
public:
  MTGCardInstance * exchangeWith;
  MTGCardInstance * attacker;
  WEventCreatureBlockerRank(MTGCardInstance * card,MTGCardInstance * exchangeWith, MTGCardInstance * attacker);

};


#endif
