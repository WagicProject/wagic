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
  int stuffHappened;
  virtual void Render();
  virtual void Update(float dt);
  int unstoppableRenderInProgress();
  bool CheckUserInput(u32 key);
  ActionLayer(){ menuObject = NULL; abilitiesMenu = NULL; stuffHappened = 0;};
  ~ActionLayer();
  int cancelCurrentAction();
  ActionElement * isWaitingForAnswer();
  int isReactingToTargetClick(Targetable * card);
  int receiveEventPlus(WEvent * event);
  int reactToTargetClick(Targetable * card);
  int isReactingToClick(MTGCardInstance  * card);
  int reactToClick(MTGCardInstance * card);
  int reactToClick(ActionElement * ability,MTGCardInstance * card);
  int reactToTargetClick(ActionElement * ability,Targetable * card);
  int stillInUse(MTGCardInstance * card);
  void setMenuObject(Targetable * object);
  void ButtonPressed(int controllerid, int controlid);
  void doReactTo(int menuIndex);
  TargetChooser * getCurrentTargetChooser();
  MTGAbility * getAbility(int type);
};



#endif
