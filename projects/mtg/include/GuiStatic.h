#ifndef _GUISTATIC_H_
#define _GUISTATIC_H_

#include "Player.h"
#include "MTGGameZones.h"
#include "CardDisplay.h"
#include "CardGui.h"
#include "GuiAvatars.h"

class CardView;

struct GuiStatic : public PlayGuiObject{
  GuiAvatars* parent;
  GuiStatic(float desiredHeight, float x, float y, bool hasFocus, GuiAvatars* parent);
  virtual void Entering();
  virtual bool Leaving(JButton key);
};

struct GuiAvatar : public GuiStatic{
  typedef enum { TOP_LEFT, BOTTOM_RIGHT } Corner;
  static const unsigned Width = 35;
  static const unsigned Height = 50;

 protected:
  int avatarRed;
  int currentLife;
  Corner corner;
 public:
  Player * player;
  virtual void Render();
  GuiAvatar(float x, float y, bool hasFocus, Player * player, Corner corner, GuiAvatars* parent);
  virtual ostream& toString(ostream& out) const;
};

struct GuiGameZone : public GuiStatic{
  static const int Width = 20;
  static const int Height = 25;
  vector<CardView*> cards;

 public:
  MTGGameZone * zone;
  CardDisplay * cd;
  int showCards;
  virtual void Render();
  virtual bool CheckUserInput(JButton key);
  virtual void Update(float dt);
  GuiGameZone(float x, float y, bool hasFocus, MTGGameZone * zone, GuiAvatars* parent);
  ~GuiGameZone();
  virtual void ButtonPressed(int controllerId, int controlId);
  void toggleDisplay();
  virtual ostream& toString(ostream& out) const;
};
//opponenthand
class GuiOpponentHand: public GuiGameZone{
public:
    Player * player;
	GuiOpponentHand(float _x, float _y, bool hasFocus, Player * player, GuiAvatars* Parent);
	int receiveEventPlus(WEvent*);
	int receiveEventMinus(WEvent*);
	virtual ostream& toString(ostream& out) const;
};
//end of my addition
class GuiGraveyard: public GuiGameZone{
 public:
  Player * player;
  GuiGraveyard(float _x, float _y, bool hasFocus, Player * player, GuiAvatars* parent);
  int receiveEventPlus(WEvent*);
  int receiveEventMinus(WEvent*);
  virtual ostream& toString(ostream& out) const;
};

class GuiLibrary: public GuiGameZone{
 public:
  Player * player;
  GuiLibrary(float _x, float _y, bool hasFocus, Player * player, GuiAvatars* parent);
  virtual ostream& toString(ostream& out) const;
};

#endif // _GUISTATIC_H_
