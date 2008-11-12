#ifndef _MTGGUIHAND_H_
#define _MTGGUIHAND_H_



#include "GuiCardsController.h"
#include "Player.h"
#include "GameObserver.h"

#define HAND_SHOW_ANIMATION 1
#define HAND_HIDE_ANIMATION 2
#define HAND_SHOW 3
#define HAND_HIDE 4

class Player;
class GuiCardscontroller;

class MTGGuiHand: public GuiCardsController{
 protected:
  int currentId[2];
  Player * currentPlayer;
  int mShowHand;
  float mAnimState;
  JLBFont * mFont;
 public:
  MTGGuiHand(int id, GameObserver * _game);
  void Update(float dt);
  void CheckUserInput(float dt);
  virtual void Render();
  void updateCards();

};

#endif
