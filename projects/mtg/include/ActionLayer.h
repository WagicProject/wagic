/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _ACTIONLAYER_H_
#define _ACTIONLAYER_H_

#include "GuiLayers.h"
#include "ActionElement.h"
#include "SimpleMenu.h"
#include "MTGAbility.h"

class GuiLayer;
class Targetable;
class WEvent;

class ActionLayer: public GuiLayer, public JGuiListener{
 public:
  Targetable * menuObject;
  SimpleMenu * abilitiesMenu;
  virtual void Render();
  virtual void Update(float dt);
  int unstopableRenderInProgress();
  bool CheckUserInput(u32 key);
  ActionLayer(int id, GameObserver* _game):GuiLayer(id, _game){ menuObject = NULL; abilitiesMenu = NULL;};
  ~ActionLayer();
  int isWaitingForAnswer();
  int isReactingToTargetClick(Targetable * card);
  int receiveEvent(WEvent * event);
  int reactToTargetClick(Targetable * card);
  int isReactingToClick(MTGCardInstance  * card);
  int reactToClick(MTGCardInstance * card);
  int stillInUse(MTGCardInstance * card);
  void setMenuObject(Targetable * object);
  void ButtonPressed(int controllerid, int controlid);
  void doReactTo(int menuIndex);
  TargetChooser * getCurrentTargetChooser();
  MTGAbility * getAbility(int type);
};



#endif
