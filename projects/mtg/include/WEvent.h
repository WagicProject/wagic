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
class MTGAbility;
class DrawAction;

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
  virtual Targetable * getTarget(int) {return 0;};
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
  MTGCardInstance * source;
  WEventCounters(Counters *counter,string name,int power, int toughness,bool added = false, bool removed = false, MTGCardInstance * source = NULL);
  using WEvent::getTarget;
  virtual Targetable * getTarget(int target);
};

struct WEventTotalCounters : public WEvent {
  MTGCardInstance * targetCard;
  Counters * counter;
  string name;
  int power;
  int toughness;
  bool added;
  bool removed;
  int totalamount;
  MTGCardInstance * source;
  WEventTotalCounters(Counters *counter,string name,int power, int toughness,bool added = false, bool removed = false, int totalamount = 0, MTGCardInstance * source = NULL);
  using WEvent::getTarget;
  virtual Targetable * getTarget(int target);
};

struct WEventLife : public WEvent {
    Player * player;
    int amount;
    MTGCardInstance * source;
    WEventLife(Player * player, int amount, MTGCardInstance * source);
    virtual Targetable * getTarget(int target);
};

struct WEventDamageStackResolved : public WEvent {
  WEventDamageStackResolved();
};


struct WEventGameStateBasedChecked : public WEvent {
  WEventGameStateBasedChecked();
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
  bool noTrigger;
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

//event when card is exploited.
struct WEventCardExploited : public WEventCardUpdate {
    MTGCardInstance * cardAfter;
    WEventCardExploited(MTGCardInstance * card,MTGCardInstance * afterCard);
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

//event when card is exerted.
struct WEventCardExerted : public WEventCardUpdate {
  WEventCardExerted(MTGCardInstance * card);
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
    using WEvent::getTarget;
    virtual Targetable * getTarget(Player * player);
};
//event for a card draw ability resolving
struct WEventDraw : public WEvent {
    WEventDraw(Player * player,int nb_cards,MTGAbility * drawer);
    Player * player;
    int nb_cards;
    MTGAbility * drawAbility;
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
struct WEventEngageManaExtra : public WEvent {
  int color;
  MTGCardInstance* card;
  ManaPool * destination;
  WEventEngageManaExtra(int color, MTGCardInstance* card, ManaPool * destination);
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

//event when card-equipment unattached
struct WEventCardUnattached : public WEventCardUpdate {
  WEventCardUnattached(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//event when card-equipment attached/equipped
struct WEventCardEquipped : public WEventCardUpdate {
  WEventCardEquipped(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//event when card moves from player/opponent battlefield to player/opponent battlefield
struct WEventCardControllerChange : public WEventCardUpdate {
  WEventCardControllerChange(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//event when card phases out
struct WEventCardPhasesOut : public WEventCardUpdate {
  WEventCardPhasesOut(MTGCardInstance * card, int turn);
    virtual Targetable * getTarget(int target);
};

//event when card phases in
struct WEventCardPhasesIn : public WEventCardUpdate {
  WEventCardPhasesIn(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//event when card with morph faces up
struct WEventCardFaceUp : public WEventCardUpdate {
  WEventCardFaceUp(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//event when card transforms
struct WEventCardTransforms : public WEventCardUpdate {
  WEventCardTransforms(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//event when card copies a card
struct WEventCardCopiedACard : public WEventCardUpdate {
  WEventCardCopiedACard(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//alterpoison event
struct WEventplayerPoisoned : public WEvent {
    WEventplayerPoisoned(Player * player, int nb_count);
    Player * player;
    int nb_count;
    using WEvent::getTarget;
    virtual Targetable * getTarget(Player * player);
};

//alterenergy event
struct WEventplayerEnergized : public WEvent {
    WEventplayerEnergized(Player * player, int nb_count);
    Player * player;
    int nb_count;
    using WEvent::getTarget;
    virtual Targetable * getTarget(Player * player);
};

//alterexperience event
struct WEventplayerExperienced : public WEvent {
    WEventplayerExperienced(Player * player, int nb_count);
    Player * player;
    int nb_count;
    using WEvent::getTarget;
    virtual Targetable * getTarget(Player * player);
};

//monarch event
struct WEventplayerMonarch : public WEvent {
    WEventplayerMonarch(Player * player);
    Player * player;
    using WEvent::getTarget;
    virtual Targetable * getTarget(Player * player);
};

//initiative event
struct WEventplayerInitiative : public WEvent {
    WEventplayerInitiative(Player * player);
    Player * player;
    using WEvent::getTarget;
    virtual Targetable * getTarget(Player * player);
};

//shuffle event
struct WEventplayerShuffled : public WEvent {
    WEventplayerShuffled(Player * player);
    Player * player;
    using WEvent::getTarget;
    virtual Targetable * getTarget(Player * player);
};

//boast event
struct WEventCardBoasted : public WEventCardUpdate {
    WEventCardBoasted(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//defeated event
struct WEventCardDefeated : public WEventCardUpdate {
    WEventCardDefeated(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//surveil event
struct WEventCardSurveiled : public WEventCardUpdate {
    WEventCardSurveiled(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//foretell event
struct WEventCardForetold : public WEventCardUpdate {
    WEventCardForetold(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//training event
struct WEventCardTrained : public WEventCardUpdate {
    WEventCardTrained(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//scry event
struct WEventCardScryed : public WEventCardUpdate {
    WEventCardScryed(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//explores event
struct WEventCardExplored : public WEventCardUpdate {
    WEventCardExplored(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//dungeon completed event
struct WEventCardDungeonCompleted : public WEventCardUpdate {
    int totaldng;
    string playerName;
    WEventCardDungeonCompleted(MTGCardInstance * card, int totaldng, string playerName);
    virtual Targetable * getTarget(int target);
};

//roll die event
struct WEventCardRollDie : public WEventCardUpdate {
    string playerName;
    WEventCardRollDie(MTGCardInstance * card, string playerName);
    virtual Targetable * getTarget(int target);
};

//flip coin event
struct WEventCardFlipCoin : public WEventCardUpdate {
    string playerName;
    WEventCardFlipCoin(MTGCardInstance * card, string playerName);
    virtual Targetable * getTarget(int target);
};

//mutation event
struct WEventCardMutated : public WEventCardUpdate {
    WEventCardMutated(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

//token creation event
struct WEventTokenCreated : public WEventCardUpdate {
    WEventTokenCreated(MTGCardInstance * card);
    virtual Targetable * getTarget(int target);
};

std::ostream& operator<<(std::ostream&, const WEvent&);

#endif
