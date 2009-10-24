#ifndef _MTGABILITY_H_
#define _MTGABILITY_H_



class MTGCardInstance;
class GameObserver;
class Spell;
class Damageable;
class PlayGuiObject;
class TargetChooser;
class ManaCost;
class MTGGameZone;
class Player;
class AManaProducer;
class WEvent;

#include "ActionElement.h"
#include <string>
#include <map>
#include <hge/hgeparticle.h>
#include "../include/Damage.h"
using std::string;
using std::map;


//stupid variables used to give a hint to the AI:
// Should I cast a spell on an enemy or friendly unit ?
#define BAKA_EFFECT_GOOD 1
#define BAKA_EFFECT_BAD -1
#define BAKA_EFFECT_DONTKNOW 0
#define MODE_PUTINTOPLAY 1
#define MODE_ABILITY 2
#define MODE_TARGET 3

#define COUNT_POWER 1

#define PARSER_LORD 1
#define PARSER_FOREACH 2
#define PARSER_ASLONGAS 3

class MTGAbility: public ActionElement{
 protected:
  char menuText[25];
  
  GameObserver * game;
 public:
   int oneShot;
   int forceDestroy;
  ManaCost * cost;
  Targetable * target;
  int aType;
  MTGCardInstance * source;
  MTGAbility(int id, MTGCardInstance * card);
  MTGAbility(int id, MTGCardInstance * _source, Targetable * _target);
  virtual int testDestroy();
  virtual ~MTGAbility();
  virtual void Render(){};
  virtual int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){return 0;};
  virtual int reactToClick(MTGCardInstance * card){return 0;};
  virtual int receiveEvent(WEvent * event){return 0;};
  virtual void Update(float dt){};
  virtual int fireAbility();
  virtual int stillInUse(MTGCardInstance * card);
  virtual int resolve(){return 0;};
  virtual MTGAbility* clone() const = 0; 
  virtual ostream& toString(ostream& out) const;
  virtual int addToGame();
  virtual int removeFromGame();

  /*Poor man's casting */
  /* Todo replace that crap with dynamic casting */
  enum {
    UNKNOWN = 0,
    MANA_PRODUCER = 1,
    MTG_ATTACK_RULE = 2,
    DAMAGER = 3,
    STANDARD_REGENERATE = 4,
    PUT_INTO_PLAY = 5,
    MOMIR = 6,
    MTG_BLOCK_RULE = 7,
  };
};


class TriggeredAbility:public MTGAbility{
 public:
  TriggeredAbility(int id, MTGCardInstance * card);
  TriggeredAbility(int id, MTGCardInstance * _source, Targetable * _target);
  virtual void Update(float dt);
  virtual void Render(){};
  virtual int trigger(){return 0;};
  virtual int triggerOnEvent(WEvent * e){return 0;};
  int receiveEvent(WEvent * e);
  virtual int resolve() = 0;
  virtual TriggeredAbility* clone() const = 0; 
  virtual ostream& toString(ostream& out) const;
};


class ActivatedAbility:public MTGAbility{
 public:
  int playerturnonly;
  int needsTapping;
  ActivatedAbility(int id, MTGCardInstance * card,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
  virtual int reactToClick(MTGCardInstance * card);
  virtual int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  virtual int reactToTargetClick(Targetable * object);
  virtual int resolve() = 0;
  virtual ActivatedAbility* clone() const = 0; 
  virtual ostream& toString(ostream& out) const;
};

class TargetAbility:public ActivatedAbility{
 public:
  MTGAbility * ability;
  TargetAbility(int id, MTGCardInstance * card, TargetChooser * _tc,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
  TargetAbility(int id, MTGCardInstance * card,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
  virtual int reactToClick(MTGCardInstance * card);
  virtual int reactToTargetClick(Targetable * object);
  virtual TargetAbility* clone() const = 0;
  virtual void Render();
  virtual int resolve();
  virtual const char * getMenuText();
  virtual ostream& toString(ostream& out) const;
  ~TargetAbility();
};

class InstantAbility:public MTGAbility{
 public:
  int init;
  virtual void Update(float dt);
  virtual int testDestroy();
  InstantAbility(int _id, MTGCardInstance * source);
  InstantAbility(int _id, MTGCardInstance * source,Damageable * _target);
  virtual int resolve(){return 0;};
  virtual InstantAbility* clone() const = 0;
  virtual ostream& toString(ostream& out) const;
};

/* State based effects. This class works ONLY for InPlay and needs to be extended for other areas of the game !!! */
class ListMaintainerAbility:public MTGAbility{
 public:
  map<MTGCardInstance *,bool> cards;
  map<Player *,bool> players;
 ListMaintainerAbility(int _id):MTGAbility(_id,NULL){};
 ListMaintainerAbility(int _id, MTGCardInstance *_source):MTGAbility(_id, _source){};
 ListMaintainerAbility(int _id, MTGCardInstance *_source,Damageable * _target):MTGAbility(_id, _source, _target){};
  virtual void Update(float dt);
  void updateTargets();
  virtual bool canTarget(MTGGameZone * zone);
  virtual int canBeInList(MTGCardInstance * card) = 0;
  virtual int added(MTGCardInstance * card) = 0;
  virtual int removed(MTGCardInstance * card) = 0;
  virtual int canBeInList(Player * p){return 0;};
  virtual int added(Player * p){return 0;};
  virtual int removed(Player * p){return 0;};
  virtual int destroy();
  virtual ListMaintainerAbility* clone() const = 0;
  virtual ostream& toString(ostream& out) const;
};

class TriggerAtPhase:public TriggeredAbility{
 public:
  int phaseId;
  int who;
  TriggerAtPhase(int id, MTGCardInstance * source, Targetable * target,int _phaseId, int who = 0);
  virtual int trigger();
  int resolve(){return 0;};
  virtual TriggerAtPhase* clone() const;
};

class TriggerNextPhase:public TriggerAtPhase{
 public:
  int destroyActivated;
  TriggerNextPhase(int id, MTGCardInstance * source, Targetable * target,int _phaseId, int who = 0);
  virtual TriggerNextPhase* clone() const;
  virtual int testDestroy();

};


class GenericTriggeredAbility:public TriggeredAbility{
 public:
  TriggeredAbility * t;
  MTGAbility * ability;
  MTGAbility * destroyCondition;
  GenericTriggeredAbility(int id, MTGCardInstance * _source,  TriggeredAbility * _t, MTGAbility * a,MTGAbility * dc = NULL, Targetable * _target = NULL);
  virtual int trigger();
  virtual int triggerOnEvent(WEvent * e);
  virtual int resolve();
  virtual int testDestroy();
  void Update(float dt);
  virtual GenericTriggeredAbility* clone() const;
  const char * getMenuText();
  ~GenericTriggeredAbility();
};

/* Ability Factory */
class AbilityFactory{
 private:
   int countCards(TargetChooser * tc, Player * player = NULL, int option = 0);
  int parsePowerToughness(string s, int *power, int *toughness);
  TriggeredAbility * parseTrigger(string s, int id, Spell * spell, MTGCardInstance *card, Targetable * target);
  MTGAbility * parseMagicLine(string s, int id, Spell * spell, MTGCardInstance *card, int activated = 0, int forceUEOT = 0);
 public:
  int abilityEfficiency(MTGAbility * a, Player * p, int mode = MODE_ABILITY);
  int magicText(int id, Spell * spell, MTGCardInstance * card = NULL, int mode = MODE_PUTINTOPLAY);
  static int computeX(Spell * spell, MTGCardInstance * card);
  int destroyAllInPlay(TargetChooser * tc, int bury = 0);
  int moveAll(TargetChooser * tc, string destinationZone);
  int damageAll(TargetChooser * tc, int damage);
  int TapAll(TargetChooser * tc);
  int CantBlock(TargetChooser * tc);
  int UntapAll(TargetChooser * tc);
  void addAbilities(int _id, Spell * spell);
};


class AManaProducer: public MTGAbility{
 protected:

  
  string menutext;
  Player * controller;

 public:
   ManaCost * output;
   int tap;
   AManaProducer(int id, MTGCardInstance * card, ManaCost * _output, ManaCost * _cost = NULL, int doTap = 1 );
   int isReactingToClick(MTGCardInstance *  _card, ManaCost * mana = NULL);
  int resolve();
  int reactToClick(MTGCardInstance *  _card);
  const char * getMenuText();
  ~AManaProducer();
  virtual AManaProducer * clone() const;
};

#include "MTGCardInstance.h"

#endif

