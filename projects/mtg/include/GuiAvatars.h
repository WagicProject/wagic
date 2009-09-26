#ifndef _GUIAVATARS_H_
#define _GUIAVATARS_H_

#include "GuiLayers.h"
#include "CardSelector.h"

struct GuiAvatar;
class GuiGraveyard;
class GuiLibrary;

class GuiAvatars : public GuiLayer
{
 protected:
  GuiAvatar* self, *opponent;
  GuiGraveyard* selfGraveyard, *opponentGraveyard;
  GuiLibrary* selfLibrary, *opponentLibrary;
  CardSelector* cs;
  GuiAvatar* active;

 public:
  GuiAvatars(CardSelector*);
  ~GuiAvatars();
  //  virtual void Render();
  void Update(float dt);
  void Activate(PlayGuiObject* c);
  void Deactivate(PlayGuiObject* c);
  int receiveEventPlus(WEvent*);
  int receiveEventMinus(WEvent*);
  bool CheckUserInput(u32 key);
};

#endif // _GUIAVATARS_H_
