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

class GuiLayer;
class Targetable;

class ActionLayer: public GuiLayer, public JGuiListener{
 public:
  Targetable * menuObject;
  SimpleMenu * abilitiesMenu;
  virtual void Render();
  virtual void Update(float dt);
  int unstopableRenderInProgress();
  void CheckUserInput(float dt);
 ActionLayer(int id, GameObserver* _game):GuiLayer(id, _game){ menuObject = NULL; abilitiesMenu = NULL;};
  int isWaitingForAnswer();
  int isReactingToTargetClick(Targetable * card);
  int reactToTargetClick(Targetable * card);
  int isReactingToClick(MTGCardInstance  * card);
  int reactToClick(MTGCardInstance * card);
  int isModal();
  void setMenuObject(Targetable * object);
  void ButtonPressed(int controllerid, int controlid);
  TargetChooser * getCurrentTargetChooser();
};



#endif
