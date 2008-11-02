#ifndef _PLAYGUIOBJECTCONTROLLER_H_
#define _PLAYGUIOBJECTCONTROLLER_H_


#include "GuiLayers.h"

class PlayGuiObjectController : public GuiLayer{
 protected:
  int getClosestItem(int direction);
  int getClosestItem(int direction, float tolerance);
	static bool showBigCards;
 public:
  virtual void Update(float dt);
  virtual void CheckUserInput(float dt);
  PlayGuiObjectController(int id, GameObserver* _game):GuiLayer(id, _game){};
  virtual void Render(){GuiLayer::Render();};
};



#endif