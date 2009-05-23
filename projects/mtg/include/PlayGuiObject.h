/*
   A class for all interactive objects in the play area (cards, avatars, etc...)
*/

#ifndef _PLAYGUIOBJECT_H_
#define _PLAYGUIOBJECT_H_

#define GUI_AVATAR 1
#define GUI_CARD 2
#define GUI_GRAVEYARD 3
#define GUI_LIBRARY 4

#include <JGui.h>

class MTGGameZone;
class Player;
class CardDisplay;

class PlayGuiObject: public JGuiObject, public JGuiListener{
 protected:

 public:
  int wave;
  float mHeight;
  float defaultHeight;
  bool mHasFocus;
  int x;
  int y;
  int type;
  virtual void Entering(){mHasFocus = true;};
  virtual bool Leaving(u32 key){mHasFocus = false;return true;};
  virtual bool ButtonPressed(){return true;};
  virtual void Render(){};
  virtual void Update(float dt);
  PlayGuiObject(int id, float desiredHeight,float _x, float _y, bool hasFocus);
  virtual void ButtonPressed(int controllerId, int controlId){};
  virtual ~PlayGuiObject(){};

};

class GuiAvatar: public PlayGuiObject{
 protected:

  int avatarRed;
  int currentLife;
 public:
  Player * player;
  virtual void Render();
  GuiAvatar(int id, float desiredHeight,float _x, float _y, bool hasFocus,Player * _player);
  virtual ostream& toString(ostream& out) const;
};

class GuiGameZone: public PlayGuiObject{
 protected:
  MTGGameZone * zone;

 public:
  CardDisplay * cd;
  int showCards;
  virtual void Render();
  virtual void Update(float dt);
  GuiGameZone(int id, float desiredHeight,float _x, float _y, bool hasFocus,MTGGameZone * _zone);
  ~GuiGameZone();
  virtual void ButtonPressed(int controllerId, int controlId);
  void toggleDisplay();
  virtual ostream& toString(ostream& out) const;
};

class GuiGraveyard: public GuiGameZone{
 public:
  GuiGraveyard(int id, float desiredHeight,float _x, float _y, bool hasFocus,Player * player);
  virtual ostream& toString(ostream& out) const;
};

class GuiLibrary: public GuiGameZone{
 public:
  GuiLibrary(int id, float desiredHeight,float _x, float _y, bool hasFocus,Player * player);
  virtual ostream& toString(ostream& out) const;
};


#endif
