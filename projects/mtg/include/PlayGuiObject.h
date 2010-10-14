/*
   A class for all interactive objects in the play area (cards, avatars, etc...)
*/

#ifndef _PLAYGUIOBJECT_H_
#define _PLAYGUIOBJECT_H_

#define GUI_AVATAR 1
#define GUI_CARD 2
#define GUI_GRAVEYARD 3
#define GUI_LIBRARY 4
#define GUI_OPPONENTHAND 5

#include <JGui.h>
#include "Effects.h"
#include "WEvent.h"
#include "Pos.h"

class PlayGuiObject: public JGuiObject, public JGuiListener, public Pos{
 protected:

 public:
  int wave;
  float mHeight;
  float defaultHeight;
  bool mHasFocus;
  int type;
  virtual void Entering(){mHasFocus = true; zoom = 1.4f;};
  virtual bool Leaving(JButton key){mHasFocus = false; zoom = 1.0; return true;};
  virtual bool CheckUserInput(JButton key) {return false;};
  virtual bool ButtonPressed(){return true;};
  virtual void Render();
  virtual void Update(float dt);
  PlayGuiObject(float desiredHeight, float x, float y, bool hasFocus);
  PlayGuiObject(float desiredHeight, const Pos& ref, bool hasFocus);
  virtual void ButtonPressed(int controllerId, int controlId){};
  virtual bool getTopLeft(int& top, int& left) {top = static_cast<int>(actY); left = static_cast<int>(actX); return true;};
  virtual ~PlayGuiObject(){};
  vector<Effect*> effects;
};

#endif
