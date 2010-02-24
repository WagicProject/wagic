#ifndef _GUICOMBAT_H_
#define _GUICOMBAT_H_

#include <vector>
#include "WEvent.h"
#include "CardGui.h"
#include "MTGCardInstance.h"
#include "DamagerDamaged.h"

class GuiCombat : public GuiLayer
{
 protected:
  GameObserver* go;
  DamagerDamaged* active;
  AttackerDamaged* activeAtk;
  static JTexture* ok_tex;
  Pos ok, enemy_avatar;
  DamagerDamaged* current;
  enum { BLK, ATK, OK, NONE } cursor_pos;
  CombatStep step;
  void validateDamage();
  void addOne(DefenserDamaged* blocker, CombatStep);
  void removeOne(DefenserDamaged* blocker, CombatStep);
  void remaskBlkViews(AttackerDamaged* before, AttackerDamaged* after);
  int resolve();

 public:

  vector<AttackerDamaged*> attackers;
  void autoaffectDamage(AttackerDamaged* attacker, CombatStep);

  GuiCombat(GameObserver* go);
  ~GuiCombat();
  virtual void Update(float dt);
  virtual void Render();
  bool clickOK();
  virtual bool CheckUserInput(JButton key);
  virtual int receiveEventPlus(WEvent* e);
  virtual int receiveEventMinus(WEvent* e);

  typedef vector<AttackerDamaged*>::iterator inner_iterator;
};

#endif // _GUICOMBAT_H_
