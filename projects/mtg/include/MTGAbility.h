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
using std::string;
using std::map;


//Two stupid variables used to give a hint to the AI:
// Should I cast a spell on an enemy or friendly unit ?
#define BAKA_EFFECT_GOOD 1
#define BAKA_EFFECT_BAD -1
#define BAKA_EFFECT_DONTKNOW 0

#define COUNT_POWER 1

#define PARSER_LORD 1
#define PARSER_FOREACH 2
#define PARSER_ASLONGAS 3

class MTGAbility: public ActionElement{
 protected:
  char menuText[25];

  GameObserver * game;
 public:
   int forceDestroy;
  ManaCost * cost;
  Damageable * target;
  int aType;
  MTGCardInstance * source;
  MTGAbility(int id, MTGCardInstance * card);
  MTGAbility(int id, MTGCardInstance * _source, Damageable * _target);
  virtual int testDestroy();
  virtual ~MTGAbility();
  virtual void Render(){};
  virtual int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL){return 0;};
  virtual int reactToClick(MTGCardInstance * card){return 0;};
  virtual int receiveEvent(WEvent * event){return 0;};
  virtual void Update(float dt){};
  virtual int fireAbility();
  virtual int stillInUse(MTGCardInstance * card){if (card==source) return 1; return 0;};
  virtual int resolve(){return 0;};
  virtual ostream& toString(ostream& out) const;

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
  TriggeredAbility(int id, MTGCardInstance * _source, Damageable * _target);
  virtual void Update(float dt);
  virtual void Render(){};
  virtual int trigger()=0;
  virtual int resolve() = 0;
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
  virtual ostream& toString(ostream& out) const;
};

class TargetAbility:public ActivatedAbility{
 public:
  TargetAbility(int id, MTGCardInstance * card, TargetChooser * _tc,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
  TargetAbility(int id, MTGCardInstance * card,ManaCost * _cost = NULL, int _playerturnonly = 0,int tap = 1);
  virtual void Update(float dt);
  virtual int reactToClick(MTGCardInstance * card);
  virtual int reactToTargetClick(Targetable * object);
  virtual void Render();
  virtual ostream& toString(ostream& out) const;
};

class InstantAbility:public MTGAbility{
 public:
  int init;
  virtual void Update(float dt);
  virtual int testDestroy();
  InstantAbility(int _id, MTGCardInstance * source);
  InstantAbility(int _id, MTGCardInstance * source,Damageable * _target);
  virtual int resolve(){return 0;};
  virtual ostream& toString(ostream& out) const;
};

/* State based effects. This class works ONLY for InPlay and needs to be extended for other areas of the game !!! */
class ListMaintainerAbility:public MTGAbility{
 public:
  map<MTGCardInstance *,bool> cards;
 ListMaintainerAbility(int _id):MTGAbility(_id,NULL){};
 ListMaintainerAbility(int _id, MTGCardInstance *_source):MTGAbility(_id, _source){};
 ListMaintainerAbility(int _id, MTGCardInstance *_source,Damageable * _target):MTGAbility(_id, _source, _target){};
  virtual void Update(float dt);
  virtual int canBeInList(MTGCardInstance * card) = 0;
  virtual int added(MTGCardInstance * card) = 0;
  virtual int removed(MTGCardInstance * card) = 0;
  virtual int destroy();
  virtual ostream& toString(ostream& out) const;
};

/* An attempt to globalize triggered abilities as much as possible */
class MTGAbilityBasicFeatures{
 public:
  Damageable * target;
  GameObserver * game;
  MTGCardInstance * source;
  MTGAbilityBasicFeatures();
  MTGAbilityBasicFeatures(MTGCardInstance * _source, Damageable * _target = NULL);
  void init(MTGCardInstance * _source, Damageable * _target = NULL);
};

class Trigger:public MTGAbilityBasicFeatures{
 public:
  virtual int trigger()=0;
  virtual int testDestroy(){return 0;};
};


class TriggerAtPhase:public Trigger{
 public:
  int currentPhase, newPhase;
  int phaseId;

  TriggerAtPhase(int _phaseId);

  virtual int trigger();
};

class TriggerNextPhase:public TriggerAtPhase{
 public:
  int destroyActivated;
  TriggerNextPhase(int _phaseId);

  virtual int testDestroy();

};

class TriggeredEvent:public MTGAbilityBasicFeatures{
 public:
   TriggeredEvent();
   TriggeredEvent(MTGCardInstance * source, Damageable * target = NULL);
  virtual int resolve()=0;
};

class DrawEvent:public TriggeredEvent{
 public:
  Player * player;
  int nbcards;
  DrawEvent(Player * _player, int _nbcards);
  int resolve();
};

class BuryEvent: public TriggeredEvent{
 public:
  int resolve();
};

class DamageEvent:public TriggeredEvent{
  public:
    int damage;
    DamageEvent(MTGCardInstance * _source, Damageable * _target, int _damage);
    int resolve();
};



class DestroyCondition:public MTGAbilityBasicFeatures{
 public:
  virtual int testDestroy();
};


class GenericTriggeredAbility:public TriggeredAbility{
 public:
  Trigger * t;
  TriggeredEvent * te;
  DestroyCondition  * dc;
  GenericTriggeredAbility(int id, MTGCardInstance * _source, Trigger * _t, TriggeredEvent * _te, DestroyCondition * _dc = NULL, Damageable * _target = NULL);
  virtual int trigger();
  virtual int resolve();
  virtual int testDestroy();
  ~GenericTriggeredAbility();
};

/* Ability Factory */
class AbilityFactory{
 private:
  int countCards(TargetChooser * tc, Player * player = NULL, int option = 0);
  int putInPlayFromZone(MTGCardInstance * card, MTGGameZone * zone, Player * p);
  int parsePowerToughness(string s, int *power, int *toughness);
  Trigger * parseTrigger(string magicText);
  Damageable * parseCollateralTarget(MTGCardInstance * card, string s);
 public:
  int magicText(int id, Spell * spell, MTGCardInstance * card = NULL);
  int destroyAllInPlay(TargetChooser * tc, int bury = 0);
  int moveAll(TargetChooser * tc, string destinationZone);
  int damageAll(TargetChooser * tc, int damage);
  void addAbilities(int _id, Spell * spell);
};


class AManaProducer: public MTGAbility{
 protected:

  ManaCost * cost;
  ManaCost * output;
  string menutext;
  float x0,y0,x1,y1,x,y;
  float animation;
  Player * controller;
  int tap;

  hgeParticleSystem * mParticleSys;
 public:
   static int currentlyTapping;
   AManaProducer(int id, MTGCardInstance * card, ManaCost * _output, ManaCost * _cost = NULL, int doTap = 1 );
   void Update(float dt);
   void Render();
   int isReactingToClick(MTGCardInstance *  _card, ManaCost * mana = NULL);
  int resolve();
  int reactToClick(MTGCardInstance *  _card);
  const char * getMenuText();
  int testDestroy();
  ~AManaProducer();
  virtual ostream& toString(ostream& out) const;
};

#include "MTGCardInstance.h"

#endif

