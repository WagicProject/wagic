/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _SPELLSTACK_H_
#define _SPELLSTACK_H_

#define MAX_SPELL_TARGETS 10


#define ACTION_SPELL 10
#define ACTION_DAMAGE 11
#define ACTION_DAMAGES 12
#define ACTION_NEXTGAMEPHASE 13
#define ACTION_DRAW 14
#define ACTION_PUTINGRAVEYARD 15
#define ACTION_ABILITY 16

#define NOT_RESOLVED -2
#define RESOLVED_OK 1
#define RESOLVED_NOK -1

#include "../include/PlayGuiObject.h"
#include "GuiLayers.h"
#include "../include/TargetsList.h"
#include "../include/Targetable.h"

class GuiLayer;
class PlayGuiObject;
class MTGCardInstance;
class GameObserver;
class Player;
class Damageable;
class MTGAbility;
class Targetable;
class DamageStack;
class ManaCost;
class TargetChooser;


#define ACTIONSTACK_STANDARD 0
#define ACTIONSTACK_TARGET 1

class Interruptible: public PlayGuiObject, public Targetable{
 public:
  //TODO : remove these when they are back in PlayGuiObject
  float x, y;

  int state, display;
  MTGCardInstance * source;
  virtual void Entering(){mHasFocus = true;};
  virtual bool Leaving(u32 key){mHasFocus = false;return true;};
  virtual bool ButtonPressed(){return true;};
  virtual int resolve(){return 0;};
  virtual void Render(){};
  int typeAsTarget(){return TARGET_STACKACTION;};
 Interruptible(bool hasFocus = false):PlayGuiObject(40,x,y,hasFocus){state=NOT_RESOLVED;display=0;source=NULL;};
  virtual const string getDisplayName() const {return "stack object";};
  void Render(MTGCardInstance * source, JQuad * targetQuad, string alt1, string alt2, string action, bool bigQuad = false);
#if defined (WIN32) || defined (LINUX)
  virtual void Dump();
#endif
};

class NextGamePhase: public Interruptible {
 public:
  int resolve();
  bool extraDamagePhase();
  void Render();
  virtual ostream& toString(ostream& out) const;
  NextGamePhase(int id);
};

class Spell: public Interruptible {
 protected:

 public:
  MTGGameZone * from;
  TargetChooser * tc;
  ManaCost * cost;
  int payResult;
  int computeX(MTGCardInstance * card);
  Spell(MTGCardInstance* _source);
  Spell(int id, MTGCardInstance* _source, TargetChooser *_tc, ManaCost * _cost, int payResult);
  ~Spell();
  int resolve();
  void Render();
  bool kickerWasPaid();
  const string getDisplayName() const;
  virtual ostream& toString(ostream& out) const;
  MTGCardInstance * getNextCardTarget(MTGCardInstance * previous = 0);
  Player * getNextPlayerTarget(Player * previous = 0);
  Damageable * getNextDamageableTarget(Damageable * previous = 0);
  Interruptible * getNextInterruptible(Interruptible * previous, int type);
  Spell * getNextSpellTarget(Spell * previous = 0);
  Damage * getNextDamageTarget(Damage * previous = 0);
  Targetable * getNextTarget(Targetable * previous = 0, int type = -1);
  int getNbTargets();
};

class StackAbility: public Interruptible {
 public:
  MTGAbility * ability;
  int resolve();
  void Render();
  virtual ostream& toString(ostream& out) const;
  StackAbility(int id, MTGAbility * _ability);
};

class PutInGraveyard: public Interruptible {
 public:
  MTGCardInstance * card;
  int removeFromGame;
  int resolve();
  void Render();
  virtual ostream& toString(ostream& out) const;
  PutInGraveyard(int id, MTGCardInstance * _card);
};


class DrawAction: public Interruptible {
 public:
  int nbcards;
  Player * player;
  int resolve();
  void Render();
  virtual ostream& toString(ostream& out) const;
  DrawAction(int id, Player *  _player, int _nbcards);
};

class ActionStack :public GuiLayer{
 protected:
  GameObserver* game;
  int interruptDecision[2];
  float timer;
  int currentState;
  int mode;
  int checked;

 public:

   enum{
     NOT_DECIDED = 0,
     INTERRUPT = -1,
     DONT_INTERRUPT = 1,
     DONT_INTERRUPT_ALL = 2,
   };

  int setIsInterrupting(Player * player);
  int count( int type = 0 , int state = 0 , int display = -1);
  Interruptible * getPrevious(Interruptible * next, int type = 0, int state = 0 , int display = -1);
  int getPreviousIndex(Interruptible * next, int type = 0, int state = 0 , int display = -1);
  Interruptible * getNext(Interruptible * previous, int type = 0, int state = 0 , int display = -1);
  int getNextIndex(Interruptible * previous, int type = 0, int state = 0 , int display = -1);
  void Fizzle(Interruptible * action);
  Interruptible * getAt(int id);
  void cancelInterruptOffer(int cancelMode = 1);
  void endOfInterruption();
  Interruptible * getLatest(int state);
  Player * askIfWishesToInterrupt;
  int garbageCollect();
  int addAction(Interruptible * interruptible);
  Spell * addSpell(MTGCardInstance* card, TargetChooser * tc, ManaCost * mana, int payResult);
  int AddNextGamePhase();
  int addPutInGraveyard(MTGCardInstance * card);
  int addDraw(Player * player, int nbcards = 1);
  int addDamage(MTGCardInstance * _source, Damageable * target, int _damage);
  int addAbility(MTGAbility * ability);
  void Update(float dt);
  bool CheckUserInput(u32 key);
  virtual void Render();
  ActionStack(GameObserver* game);
  int resolve();
  int has(Interruptible * action);
  int has(MTGAbility * ability);
#if defined (WIN32) || defined (LINUX)
   void Dump();
#endif

};





#endif
