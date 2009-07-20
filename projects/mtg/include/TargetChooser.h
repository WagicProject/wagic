#ifndef _TARGETCHOOSER_H_
#define _TARGETCHOOSER_H_

#define TARGET_NOK 0
#define TARGET_OK 1
#define TARGET_OK_FULL 2
#define TARGET_OK_NOT_READY 3

#include <JGE.h>
#include "../include/TargetsList.h"
#include "../include/ActionStack.h"

#include <string>
using std::string;

class MTGCardInstance;
class MTGGameZone;
class Player;
class Damageable;
class Targetable;
class CardDescriptor;



class TargetChooser: public TargetsList {
 protected:
  int forceTargetListReady;

 public:
  TargetChooser(MTGCardInstance * card = NULL, int _maxtargets = -1);

  MTGCardInstance * source; 
  MTGCardInstance * targetter; //Optional, usually equals source, used for protection from...
  int maxtargets; //Set to -1 for "unlimited"
  virtual int targetsZone(MTGGameZone * z){return 0;};
  int ForceTargetListReady();
  int targetsReadyCheck();
  virtual int addTarget(Targetable * target);
  virtual int canTarget(Targetable * _target);
  virtual int full(){if (maxtargets != -1 && cursor>=maxtargets) {return 1;} else{return 0;}};
  virtual int ready(){return cursor;};
  virtual ~TargetChooser(){};
  int targetListSet();


};


class TargetChooserFactory{
 public:
  TargetChooser * createTargetChooser(string s, MTGCardInstance * card);
  TargetChooser * createTargetChooser(MTGCardInstance * card);
};



class TargetZoneChooser:public TargetChooser{
 public:
  int zones[10];
  int nbzones;
  int init(int * _zones, int _nbzones);
  int targetsZone(MTGGameZone * z);
  TargetZoneChooser(MTGCardInstance * card = NULL, int _maxtargets = 1);
  TargetZoneChooser(int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1);
  virtual int canTarget(Targetable * _card);
};

class CardTargetChooser:public TargetZoneChooser {

protected:
  MTGCardInstance * validTarget;
public:
  CardTargetChooser(MTGCardInstance * _card, MTGCardInstance * source,int * _zones = NULL, int _nbzones = 0);
  virtual int canTarget(Targetable * target );
};


class CreatureTargetChooser:public TargetZoneChooser{
 public:
  int maxpower;
  int maxtoughness;
  CreatureTargetChooser(int * _zones, int _nbzones,MTGCardInstance * card = NULL, int _maxtargets = 1);
  CreatureTargetChooser(MTGCardInstance * card = NULL, int _maxtargets = 1);
  virtual int canTarget(Targetable * _card);

};


class DamageableTargetChooser:public CreatureTargetChooser{
 public:
 DamageableTargetChooser(int * _zones, int _nbzones,MTGCardInstance * card = NULL, int _maxtargets = 1):CreatureTargetChooser( _zones,_nbzones, card, _maxtargets){};
 DamageableTargetChooser(MTGCardInstance * card = NULL, int _maxtargets = 1):CreatureTargetChooser(card, _maxtargets){};
  virtual int canTarget(Targetable * target);
};


class PlayerTargetChooser:public TargetChooser{
protected:
  Player * p; //In Case we can only target a specific player
 public:
 PlayerTargetChooser(MTGCardInstance * card = NULL, int _maxtargets = 1, Player *_p = NULL);
  virtual int canTarget(Targetable * target);
};

class TypeTargetChooser:public TargetZoneChooser{
 public:
  int nbtypes;
  int types[10];
  TypeTargetChooser(const char * _type, MTGCardInstance * card = NULL, int _maxtargets = 1);
  TypeTargetChooser(const char * _type, int * _zones, int nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1);
  void addType(int type);
  void addType(const char * type);
  virtual int canTarget(Targetable * targe);
};

class DescriptorTargetChooser:public TargetZoneChooser{
 public:
  CardDescriptor  * cd;
  DescriptorTargetChooser(CardDescriptor * _cd, MTGCardInstance * card = NULL, int _maxtargets = 1);
  DescriptorTargetChooser(CardDescriptor * _cd, int * _zones, int nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1);
  virtual int canTarget(Targetable * target);
  ~DescriptorTargetChooser();
};


class SpellTargetChooser:public TargetChooser{
 public:
  int color;
  SpellTargetChooser( MTGCardInstance * card = NULL,int _color = -1, int _maxtargets = 1 );
  virtual int canTarget(Targetable * target);
};

class SpellOrPermanentTargetChooser:public TargetZoneChooser{
 public:
  int color;
  SpellOrPermanentTargetChooser(MTGCardInstance * card = NULL,int _color = -1 , int _maxtargets = 1);
  virtual int canTarget(Targetable * target);
};



class DamageTargetChooser:public TargetChooser{
 public:
  int color;
  int state;
  DamageTargetChooser( MTGCardInstance * card = NULL,int _color = -1 , int _maxtargets = 1, int state = NOT_RESOLVED);
  virtual int canTarget(Targetable * target);
};

class DamageOrPermanentTargetChooser:public TargetZoneChooser{
 public:
  int color;
  DamageOrPermanentTargetChooser(MTGCardInstance * card = NULL,int _color = -1 , int _maxtargets = 1);
  virtual int canTarget(Targetable * target);
};
#endif
