#ifndef _GUICOMBAT_H_
#define _GUICOMBAT_H_

#include <vector>
#include "WEvent.h"
#include "CardGui.h"
#include "MTGCardInstance.h"

class GuiCombat : public GuiLayer
{
 protected:
  CardSelector* cs;
  vector<MTGCardInstance*> attackers;

 public:
  GuiCombat(CardSelector* cs);
  ~GuiCombat();
  virtual void Update(float dt);
  virtual void Render();
  virtual bool CheckUserInput(u32 key);
  virtual int receiveEventPlus(WEvent* e);
  virtual int receiveEventMinus(WEvent* e);
};


#endif // _GUICOMBAT_H_
