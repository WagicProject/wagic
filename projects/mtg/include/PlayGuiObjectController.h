#ifndef _PLAYGUIOBJECTCONTROLLER_H_
#define _PLAYGUIOBJECTCONTROLLER_H_

#define BIG_CARD_RENDER_TIME 0.4

#include "GuiLayers.h"

class PlayGuiObjectController : public GuiLayer{
 protected:
  float last_user_move;
  int getClosestItem(int direction);
  int getClosestItem(int direction, float tolerance);
  static int showBigCards;// 0 hide, 1 show, 2 show text
 public:
  virtual void Update(float dt);
  virtual void CheckUserInput(float dt);
 PlayGuiObjectController(int id, GameObserver* _game):GuiLayer(id, _game){last_user_move=0;};
  virtual void Render(){GuiLayer::Render();};
};



#endif
