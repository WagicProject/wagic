#ifndef _GUICOMBAT_H_
#define _GUICOMBAT_H_

#include <vector>
#include "WEvent.h"
#include "CardGui.h"
#include "MTGCardInstance.h"

class GuiCombat : public GuiLayer
{
 protected:
  GameObserver* go;
  TransientCardView* active, *activeAtk;
  JQuad* ok_quad;
  Pos ok;
  vector<MTGCardInstance*> attackers;
  TransientCardView* current;
  enum { BLK, ATK, OK, NONE } cursor_pos;

  vector<TransientCardView*> atkViews;
  vector<TransientCardView*> blkViews;

  void generateBlkViews(MTGCardInstance* card);

 public:
  GuiCombat(GameObserver* go);
  ~GuiCombat();
  virtual void Update(float dt);
  virtual void Render();
  virtual bool CheckUserInput(u32 key);
  virtual int receiveEventPlus(WEvent* e);
  virtual int receiveEventMinus(WEvent* e);
};


#endif // _GUICOMBAT_H_
