#ifndef _WEVENT_H_
#define _WEVENT_H_

#include <iostream>
#include "PhaseRing.h"

class MTGCardInstance;
class MTGGameZone;
class Damage;
class Phase;
class Targetable;
class ManaPool;
class AACounter;
class Counters;

class WEvent {
public:
  enum {
    NOT_SPECIFIED = 0,
    CHANGE_ZONE = 1,
    DAMAGE = 2,
    CHANGE_PHASE = 3,
  };
  //for getTargets, in  case  event has more than one possible "target", like with damage events.
  enum {
    TARGET_NONE = 0,
    TARGET_TO,
    TARGET_FROM,
  };
  int type; //Deprecated, use dynamic casting instead
  WEvent(int type = NOT_SPECIFIED);
  virtual ~WEvent() {};
  virtual std::ostream& toString(std::ostream& out) const;
  virtual int getValue() {return 0;};
  virtual Targetable * getTarget(int target) {return 0;};
};

struct WEventZoneChange : public WEvent {
  MTGCardInstance * card;
  MTGGameZone * from;
  MTGGameZone * to;
  WEventZoneChange(MTGCardInstance * card, MTGGameZone * from, MTGGameZone *to);
  virtual ~WEventZoneChange() {};
  virtual std::ostream& toString(std::ostream& out) const;
  virtual Targetable * getTarget(int target);
};

struct WEventDamage : public WEvent {
  Damage * damage;
  WEventDamage(Damage * damage);
  virtual std::ostream& toString(std::ostream& out) const;
  virtual int getValue();
  virtual Targetable * getTarget(int target);
};

struct WEventCounters : public WEvent {
  MTGCardInstance * targetCard;
  Counters * counter;
  string name;
  int power;
  int toughness;
  bool added;
  bool removed;
  WEventCounters(Counters *counter,string name,int power, int toughness,bool added = false, bool removed = false);
  virtual Targetable * getTarget();
};

struct WEventLife : public WEvent {
    Player * player;
    int amount;
    WEventLife(Player * player,int amount);
    virtual Targetable * getTarget(int target);
};

struct WEventDamageStackResolved : public WEvent {
  WEventDamageStackResolved();
};

struct WEventPhasePreChange : public WEvent {
  Phase * from;
  Phase * to;
  bool eventChanged;
  WEventPhasePreChange(Phase * from,Phase * to);
};
struct WEventPhaseChange : public WEvent {
  Phase * from;
  Phase * to;
  WEventPhaseChange(Phase * from, Phase * to);
};


//Abstract class of event when a card's status changes
struct WEventCardUpdate : public WEvent {
  MTGCardInstance * card;
  WEventCardUpdate(MTGCardInstance * card);
};

//creature damaged was killed, triggers effects targetter
struct WEventVampire : public WEventCardUpdate {
  MTGCardInstance * card;
  MTGCardInstance * source;
  MTGCardInstance * victem;
  WEventVampire(MTGCardInstance * card,MTGCardInstance * source,MTGCardInstance * victem);
  virtual Targetable * getTarget(int target);
};

//creature became the target of a spell or ability
struct WEventTarget : public WEventCardUpdate {
  MTGCardInstance * card;
  MTGCardInstance * source;
  WEventTarget(MTGCardInstance * card,MTGCardInstance * source);
  virtual Targetable * getTarget(int target);
};

//Event when a card gains/looses types
struct WEventCardChangeType : public WEventCardUpdate {
  int type;
  bool before;
  bool after;
  WEventCardChangeType(MTGCardInstance * card, int type, bool before, bool after);
};

//Event when a card is tapped/untapped
struct WEventCardTap : public WEventCardUpdate {
  bool before;
  bool after;
  WEventCardTap(MTGCardInstance * card, bool before, bool after);
  virtual Targetable * getTarget(int target);
};

struct WEventCardTappedForMana : public WEventCardUpdate {
  bool before;
  bool after;
  WEventCardTappedForMana(MTGCardInstance * card, bool before, bool after);
  virtual Targetable * getTarget(int target);
};


//Event when a card's "attacker" status changes
//before:Player/Planeswalker that card was attacking previously
//after: Player/Planeswalker that card is attacking now
struct WEventCreatureAttacker : public WEventCardUpdate {
  Targetable * before;
  Targetable * after;
  WEventCreatureAttacker(MTGCardInstance * card, Targetable * from, Targetable * to);
};

//event when card attacks.
struct WEventCardAttacked : public WEventCardUpdate {
  WEventCardAttacked(MTGCardInstance * card);
	virtual Targetable * getTarget(int target);
};

//event when card attacks alone.
struct WEventCardAttackedAlone : public WEventCardUpdate {
  WEventCardAttackedAlone(MTGCardInstance * card);
	virtual Targetable * getTarget(int target);
};

//event when card attacks but is not blocked.
struct WEventCardAttackedNotBlocked : public WEventCardUpdate {
  WEventCardAttackedNotBlocked(MTGCardInstance * card);
	virtual Targetable * getTarget(int target);
};

//event when card attacks but is blocked.
struct WEventCardAttackedBlocked : public WEventCardUpdate {
  WEventCardAttackedBlocked(MTGCardInstance * card,MTGCardInstance * opponent);
  MTGCardInstance * opponent;
	virtual Targetable * getTarget(int target);
};

//event when card blocked.
struct WEventCardBlocked : public WEventCardUpdate {
  WEventCardBlocked(MTGCardInstance * card,MTGCardInstance * opponent);
  MTGCardInstance * opponent;
	virtual Targetable * getTarget(int target);
};

//event when card is sacrificed.
struct WEventCardSacrifice : public WEventCardUpdate {
    MTGCardInstance * cardAfter;
    WEventCardSacrifice(MTGCardInstance * card,MTGCardInstance * afterCard);
    virtual Targetable * getTarget(int target);
};

//event when card is discarded.
struct WEventCardDiscard : public WEventCardUpdate {
  WEventCardDiscard(MTGCardInstance * card);
	virtual Targetable * getTarget(int target);
};

//event when card is cycled.
struct WEventCardCycle : public WEventCardUpdate {
  WEventCardCycle(MTGCardInstance * card);
	virtual Targetable * getTarget(int target);
};

//Event when a card's "defenser" status changes
//before : attacker that card was blocking previously
//after: attacker that card is blocking now
struct WEventCreatureBlocker : public WEventCardUpdate {
  MTGCardInstance * before;
  MTGCardInstance * after;
  WEventCreatureBlocker(MTGCardInstance * card,MTGCardInstance * from,MTGCardInstance * to);
};

//Event sent when attackers have been chosen and they
//cannot be changed any more.
struct WEventAttackersChosen : public WEvent {
};

//Event sent when blockers have been chosen and they
//cannot be changed any more.
struct WEventBlockersChosen : public WEvent {
};

struct WEventcardDraw : public WEvent {
	WEventcardDraw(Player * player,int nb_cards);
	Player * player;
	int nb_cards;
	virtual Targetable * getTarget(Player * player);
};

//Event when a blocker is reordered
//exchangeWith: exchange card's position with exchangeWith's position
//attacker:both card and exchangeWith *should* be in attacker's "blockers" list.
struct WEventCreatureBlockerRank : public WEventCardUpdate {
  MTGCardInstance * exchangeWith;
  MTGCardInstance * attacker;
  WEventCreatureBlockerRank(MTGCardInstance * card,MTGCardInstance * exchangeWith, MTGCardInstance * attacker);
};

//Event when a combat phase step ends
struct WEventCombatStepChange : public WEvent
{
  CombatStep step;
  WEventCombatStepChange(CombatStep);
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

std::ostream& operator<<(std::ostream&, const WEvent&);

#endif
