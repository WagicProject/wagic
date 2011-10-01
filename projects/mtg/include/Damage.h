#ifndef _DAMAGE_H_
#define _DAMAGE_H_

#include <JGui.h>
#include "GuiLayers.h"
#include "ActionStack.h"
#include "WResource_Fwd.h"

class GuiLayer;
class JGuiObject;
class MTGCardInstance;
class GameObserver;

#define DAMAGEABLE_MTGCARDINSTANCE 0
#define DAMAGEABLE_PLAYER 1

#define DAMAGE_ALL_TYPES 0
#define DAMAGE_COMBAT 1
#define DAMAGE_OTHER 2

class Damageable:public Targetable
{
protected:
public:
    int life;
    int handsize;
    int poisonCount;
    int damageCount;
    int preventable;
    int thatmuch;
    int lifeLostThisTurn;
    int type_as_damageable;
    Damageable(GameObserver* observer, int _life)
        : Targetable(observer)
        {life=_life;lifeLostThisTurn = 0;};
    int getLife(){return life;};
    virtual int dealDamage(int damage){life-=damage;return life;};
    virtual int afterDamage(){return 0;}
    virtual int poisoned(){return 0;}
    virtual int prevented(){return 0;}
    virtual JQuadPtr getIcon(){return JQuadPtr();}
};

class Damage: public Interruptible
{
 protected:
  void init(MTGCardInstance * source, Damageable * target, int damage, int typeOfDamage);
 public:
  Damageable * target;
  int typeOfDamage;
  int damage;
  void Render();
  Damage(GameObserver* observer, MTGCardInstance* source, Damageable * target);
  Damage(GameObserver* observer, MTGCardInstance* source, Damageable * target, int damage, int typeOfDamage = DAMAGE_OTHER);
  int resolve();
  virtual ostream& toString(ostream& out) const;
};

class DamageStack : public GuiLayer, public Interruptible
{
 protected:
  int currentState;

 public:
  int receiveEvent(WEvent * event);
  int resolve();
  void Render();
  virtual ostream& toString(ostream& out) const;
  DamageStack(GameObserver *observer);
};

ostream& operator<<(ostream& out, const Damageable& p);

istream& operator>>(istream& in, Damageable& p);

#endif
