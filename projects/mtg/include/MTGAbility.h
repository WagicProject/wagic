#ifndef _MTGABILITY_H_
#define _MTGABILITY_H_



class MTGCardInstance;
class Spell;
class Damageable;
class PlayGuiObject;
class ManaCost;
class MTGGameZone;
class Player;
class AManaProducer;
class WEvent;
class Counter;

#include "GameObserver.h"
#include "ActionElement.h"
#include <string>
#include <map>
#include <hge/hgeparticle.h>
#include "Damage.h"
#include "TargetChooser.h"
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
  char menuText[50];
  
  GameObserver * game;
 public:
 enum {
     NO_RESTRICTION = 0,
     PLAYER_TURN_ONLY = 1,
     AS_SORCERY = 2,
    MY_BEFORE_BEGIN = 3,
    MY_UNTAP = 4,
    MY_UPKEEP = 5,
    MY_DRAW = 6,
    MY_FIRSTMAIN = 7,
    MY_COMBATBEGIN = 8,
    MY_COMBATATTACKERS = 9,
    MY_COMBATBLOCKERS = 10,
    MY_COMBATDAMAGE = 11,
    MY_COMBATEND = 12,
    MY_SECONDMAIN = 13,
    MY_ENDOFTURN = 14,
    MY_EOT = 15,
    MY_CLEANUP = 16,
    MY_AFTER_EOT = 17,

    OPPONENT_BEFORE_BEGIN = 23,
    OPPONENT_UNTAP = 24,
    OPPONENT_UPKEEP = 25,
    OPPONENT_DRAW = 26,
    OPPONENT_FIRSTMAIN = 27,
    OPPONENT_COMBATBEGIN = 28,
    OPPONENT_COMBATATTACKERS = 29,
    OPPONENT_COMBATBLOCKERS = 30,
    OPPONENT_COMBATDAMAGE = 31,
    OPPONENT_COMBATEND = 32,
    OPPONENT_SECONDMAIN = 33,
    OPPONENT_ENDOFTURN = 34,
    OPPONENT_EOT = 35,
    OPPONENT_CLEANUP = 36,
    OPPONENT_AFTER_EOT = 37,

    BEFORE_BEGIN = 43,
    UNTAP = 44,
    UPKEEP = 45,
    DRAW = 46,
    FIRSTMAIN = 47,
    COMBATBEGIN = 48,
    COMBATATTACKERS = 49,
    COMBATBLOCKERS = 50,
    COMBATDAMAGE = 51,
    COMBATEND = 52,
    SECONDMAIN = 53,
    ENDOFTURN = 54,
    EOT = 55,
    CLEANUP = 56,
    AFTER_EOT = 57,
     
    OPPONENT_TURN_ONLY = 60,

 };
 int parseCastRestrictions(MTGCardInstance * card,Player * player,string restrictions,string otherRestrictions);
 int allowedToCast(MTGCardInstance * card,Player * player);
 int allowedToAltCast(MTGCardInstance * card,Player * player);
 int oneShot;
 int forceDestroy;
 ManaCost * cost;
 ManaCost * alternative;
 ManaCost * BuyBack;
 ManaCost * FlashBack;
 ManaCost * Retrace;
 ManaCost * morph;
 ManaCost * suspend;

  Targetable * target;
  int aType;
	int naType;
	int abilitygranted;
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
	ALTERNATIVE_COST = 8,
    BUYBACK_COST = 9,
	FLASHBACK_COST = 10,
    RETRACE_COST = 11,
		MTG_COMBATTRIGGERS_RULE = 12,
		STANDARD_PREVENT = 13,
		STANDARD_EQUIP = 14,
		STANDARD_LEVELUP = 15,
		FOREACH = 16,
		STANDARD_DRAW = 17,
		STANDARD_PUMP = 18,
		STANDARD_BECOMES = 19,
		UPCOST = 20,
		STANDARDABILITYGRANT = 21,
		UNTAPPER = 22,
		TAPPER = 23,
		LIFER = 24,
		CLONING = 25,
		STANDARD_TEACH = 26,
		STANDARD_TOKENCREATOR = 27,
		MORPH_COST = 28,
		SUSPEND_COST = 29,


  };
};

class NestedAbility{
  public:
  MTGAbility * ability;
  NestedAbility(MTGAbility * _ability);
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
   enum {
     NO_RESTRICTION = 0,
     PLAYER_TURN_ONLY = 1,
     AS_SORCERY = 2,
    MY_BEFORE_BEGIN = 3,
    MY_UNTAP = 4,
    MY_UPKEEP = 5,
    MY_DRAW = 6,
    MY_FIRSTMAIN = 7,
    MY_COMBATBEGIN = 8,
    MY_COMBATATTACKERS = 9,
    MY_COMBATBLOCKERS = 10,
    MY_COMBATDAMAGE = 11,
    MY_COMBATEND = 12,
    MY_SECONDMAIN = 13,
    MY_ENDOFTURN = 14,
    MY_EOT = 15,
    MY_CLEANUP = 16,
    MY_AFTER_EOT = 17,

    OPPONENT_BEFORE_BEGIN = 23,
    OPPONENT_UNTAP = 24,
    OPPONENT_UPKEEP = 25,
    OPPONENT_DRAW = 26,
    OPPONENT_FIRSTMAIN = 27,
    OPPONENT_COMBATBEGIN = 28,
    OPPONENT_COMBATATTACKERS = 29,
    OPPONENT_COMBATBLOCKERS = 30,
    OPPONENT_COMBATDAMAGE = 31,
    OPPONENT_COMBATEND = 32,
    OPPONENT_SECONDMAIN = 33,
    OPPONENT_ENDOFTURN = 34,
    OPPONENT_EOT = 35,
    OPPONENT_CLEANUP = 36,
    OPPONENT_AFTER_EOT = 37,

    BEFORE_BEGIN = 43,
    UNTAP = 44,
    UPKEEP = 45,
    DRAW = 46,
    FIRSTMAIN = 47,
    COMBATBEGIN = 48,
    COMBATATTACKERS = 49,
    COMBATBLOCKERS = 50,
    COMBATDAMAGE = 51,
    COMBATEND = 52,
    SECONDMAIN = 53,
    ENDOFTURN = 54,
    EOT = 55,
    CLEANUP = 56,
    AFTER_EOT = 57,
    
    OPPONENT_TURN_ONLY = 69,
   };
   ManaCost * abilityCost;
  int restrictions;
  int limitPerTurn;
  int counters;
  int needsTapping;
  string limit;
  MTGAbility * sideEffect;
  MTGAbility * sa;
  string usesBeforeSideEffects;
  int uses;
  ActivatedAbility(int id, MTGCardInstance * card,ManaCost * _cost = NULL, int _restrictions = NO_RESTRICTION,int tap = 1,string limit = "",MTGAbility * sideEffect = NULL,string usesBeforeSideEffects = "");
  virtual ~ActivatedAbility();
  virtual void Update(float dt)
  {
      if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_AFTER_EOT)
      {
          counters = 0;
      }
      return MTGAbility::Update(dt);
  }
  virtual int reactToClick(MTGCardInstance * card);
  virtual int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
  virtual int reactToTargetClick(Targetable * object);
  virtual int resolve() = 0;
  void activeSideEffect();
  virtual ActivatedAbility* clone() const = 0; 
  virtual ostream& toString(ostream& out) const;
};

class TargetAbility:public ActivatedAbility, public NestedAbility{
 public:
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
  InstantAbility(int _id, MTGCardInstance * source,Targetable * _target);
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
  bool sourceUntapped;
  bool sourceTap;
  bool lifelost;
  int lifeamount;
  TriggerAtPhase(int id, MTGCardInstance * source, Targetable * target,int _phaseId, int who = 0,bool sourceUntapped = false,bool sourceTap = false,bool lifelost = false, int lifeamount = 0);
  virtual int trigger();
  int resolve(){return 0;};
  virtual TriggerAtPhase* clone() const;
};

class TriggerNextPhase:public TriggerAtPhase{
 public:
  int destroyActivated;
  bool sourceUntapped;
  bool sourceTap;
  TriggerNextPhase(int id, MTGCardInstance * source, Targetable * target,int _phaseId, int who = 0,bool sourceUntapped = false,bool sourceTap = false);
  virtual TriggerNextPhase* clone() const;
  virtual int testDestroy();

};


class GenericTriggeredAbility:public TriggeredAbility, public NestedAbility{
 public:
  TriggeredAbility * t;
  queue<Targetable *> targets;
  MTGAbility * destroyCondition;
  GenericTriggeredAbility(int id, MTGCardInstance * _source,  TriggeredAbility * _t, MTGAbility * a,MTGAbility * dc = NULL, Targetable * _target = NULL);
  virtual int trigger();
  virtual int triggerOnEvent(WEvent * e);
  virtual int resolve();
  virtual int testDestroy();

  Targetable * getTriggerTarget(WEvent * e, MTGAbility * a);
  void setTriggerTargets(Targetable * ta, MTGAbility * a);

  void Update(float dt);
  virtual GenericTriggeredAbility* clone() const;
  const char * getMenuText();
  ~GenericTriggeredAbility();
};

/* Ability Factory */
class AbilityFactory{
 private:
     string storedString;
     int countCards(TargetChooser * tc, Player * player = NULL, int option = 0);
  TriggeredAbility * parseTrigger(string s, string magicText, int id, Spell * spell, MTGCardInstance *card, Targetable * target);
  MTGAbility * getAlternateCost( string s, int id, Spell *spell, MTGCardInstance *card );
  MTGAbility * getManaReduxAbility(string s, int id, Spell *spell, MTGCardInstance *card, MTGCardInstance *target);
  
public:
  int parseRestriction(string s);
  Counter * parseCounter(string s, MTGCardInstance * target, Spell * spell = NULL);
  int parsePowerToughness(string s, int *power, int *toughness);	
  int getAbilities(vector<MTGAbility *> * v, Spell * spell, MTGCardInstance * card = NULL, int id = 0,MTGGameZone * dest = NULL);
  MTGAbility * parseMagicLine(string s, int id, Spell * spell, MTGCardInstance *card, int activated = 0, int forceUEOT = 0,int oneShot = 0,int forceForever = 0, MTGGameZone * dest = NULL);

  int abilityEfficiency(MTGAbility * a, Player * p, int mode = MODE_ABILITY, TargetChooser * tc = NULL);
  int magicText(int id, Spell * spell, MTGCardInstance * card = NULL, int mode = MODE_PUTINTOPLAY, TargetChooser * tc = NULL, MTGGameZone * dest = NULL);
  static int computeX(Spell * spell, MTGCardInstance * card);
  static int computeXX(Spell * spell, MTGCardInstance * card);
  static MTGAbility * getCoreAbility(MTGAbility * a);
  int destroyAllInPlay(TargetChooser * tc, int bury = 0);
  int moveAll(TargetChooser * tc, string destinationZone);
  int damageAll(TargetChooser * tc, int damage);
  int TapAll(TargetChooser * tc);
  int UntapAll(TargetChooser * tc);
  void addAbilities(int _id, Spell * spell);
};


class ActivatedAbilityTP:public ActivatedAbility{
public:
  int who;
  ActivatedAbilityTP(int id, MTGCardInstance * card, Targetable * _target = NULL, ManaCost * cost=NULL, int doTap = 0, int who = TargetChooser::UNSET);
  Targetable * getTarget();
};

class InstantAbilityTP:public InstantAbility{
public:
  int who;
  InstantAbilityTP(int id, MTGCardInstance * card, Targetable * _target = NULL, int who = TargetChooser::UNSET);
  Targetable * getTarget();
};

class AbilityTP:public MTGAbility{
public:
  int who;
  AbilityTP(int id, MTGCardInstance * card, Targetable * _target = NULL, int who = TargetChooser::UNSET);
  Targetable * getTarget();
  virtual ~AbilityTP(){};
};

class AManaProducer: public ActivatedAbilityTP{
 protected:

  
  Player * controller;

 public:
   string menutext;
   ManaCost * output;
   int tap;
   AManaProducer(int id, MTGCardInstance * card, Targetable * t, ManaCost * _output, ManaCost * _cost = NULL, int doTap = 0, int who = TargetChooser::UNSET );
   int isReactingToClick(MTGCardInstance *  _card, ManaCost * mana = NULL);
  int resolve();
  int reactToClick(MTGCardInstance *  _card);
  const char * getMenuText();
  ~AManaProducer();
  virtual AManaProducer * clone() const;
};

#endif

