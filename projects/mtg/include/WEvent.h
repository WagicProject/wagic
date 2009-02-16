#ifndef _WEVENT_H_
#define _WEVENT_H_

class MTGCardInstance;
class MTGGameZone;

class WEvent{
public:
  enum{
    CHANGE_ZONE = 1,
  };
  int type;
  WEvent(int _type);
};

class WEventZoneChange: public WEvent{
public:
  MTGCardInstance * card;
  MTGGameZone * from;
  MTGGameZone * to;
  WEventZoneChange(MTGCardInstance * _card, MTGGameZone * _from, MTGGameZone *_to);
};

#endif