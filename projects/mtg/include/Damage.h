#ifndef _DAMAGE_H_
#define _DAMAGE_H_

#include <JGui.h>
#include "GuiLayers.h"
#include "ActionStack.h"

class GuiLayer;
class JGuiObject;
class MTGCardInstance;
class GameObserver;

#define DAMAGEABLE_MTGCARDINSTANCE 0
#define DAMAGEABLE_PLAYER 1

#define DAMAGE_ALL_TYPES 0
#define DAMAGE_COMBAT 1
#define DAMAGE_OTHER 2

class Damageable:public Targetable {
 protected:
 public:
  int life;
  int poisonCount;
  int damageCount;
  int preventable;
  int type_as_damageable;
  Damageable(int _life){life=_life;};
  int getLife(){return life;};
  virtual int dealDamage(int damage){life-=damage;return life;};
  virtual int afterDamage(){return 0;}
  virtual int poisoned(){return 0;}
  virtual int prevented(){return 0;}
  virtual JQuad * getIcon(){return NULL;};
};

class Damage: public Interruptible {
 protected:
  void init(MTGCardInstance * source, Damageable * target, int damage, int typeOfDamage);
 public:
  Damageable * target;
  int typeOfDamage;
  int damage;
  void Render();
  Damage(MTGCardInstance* source, Damageable * target);
  Damage(MTGCardInstance* source, Damageable * target, int damage, int typeOfDamage = DAMAGE_OTHER);
  int resolve();
  virtual ostream& toString(ostream& out) const;
};


class DamageStack : public GuiLayer, public Interruptible{
 protected:
  int currentState;
  GameObserver* game;

 public:
  int resolve();
  void Render();
  virtual ostream& toString(ostream& out) const;
  DamageStack();
};

#endif
