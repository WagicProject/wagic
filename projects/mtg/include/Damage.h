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

class Damageable:public Targetable
{
protected:
public:
    enum DamageableType{
        DAMAGEABLE_MTGCARDINSTANCE = 0,
        DAMAGEABLE_PLAYER
    };

    int life;
    int handsize;
    int poisonCount;
    int damageCount;
    int preventable;
    int thatmuch;
    int lifeLostThisTurn;
    DamageableType type_as_damageable;
    Damageable(GameObserver* observer, int _life)
        : Targetable(observer), life(_life), handsize(0),
          poisonCount(0), damageCount(0), preventable(0), thatmuch(0),
          lifeLostThisTurn(0), type_as_damageable(DAMAGEABLE_MTGCARDINSTANCE)
        {}
    int getLife(){return life;}
    virtual int dealDamage(int damage){life-=damage;return life;}
    virtual int afterDamage(){return 0;}
    virtual int poisoned(){return 0;}
    virtual int prevented(){return 0;}
    virtual JQuadPtr getIcon(){return JQuadPtr();}

    bool parseLine(const string& s);

    friend ostream& operator<<(ostream& out, const Damageable& p);
};

class Damage: public Interruptible
{
 public:
  enum DamageType{
      DAMAGE_ALL_TYPES = 0,
      DAMAGE_COMBAT,
      DAMAGE_OTHER
  };

  Damageable * target;
  DamageType typeOfDamage;
  int damage;
  void Render();
  Damage(GameObserver* observer, MTGCardInstance* source, Damageable * target);
  Damage(GameObserver* observer, MTGCardInstance* source, Damageable * target, int damage, DamageType typeOfDamage = DAMAGE_OTHER);
  int resolve();
  virtual ostream& toString(ostream& out) const;
 protected:
  void init(MTGCardInstance * source, Damageable * target, int damage, DamageType typeOfDamage);
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

#endif
