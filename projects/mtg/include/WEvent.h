#ifndef _WEVENT_H_
#define _WEVENT_H_

class MTGCardInstance;
class MTGGameZone;
class Damage;

class WEvent{
public:
  enum{
    CHANGE_ZONE = 1,
    DAMAGE = 2,
  };
  int type;
  WEvent(int _type);
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

#endif
