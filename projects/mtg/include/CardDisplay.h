#ifndef _CARD_DISPLAY_H_
#define _CARD_DISPLAY_H_

#include "../include/PlayGuiObjectController.h"

class TargetChooser;
class MTGGameZone;
class MTGCardInstance;

class CardDisplay:public PlayGuiObjectController{
  int mId;
  GameObserver* game;
 public:
  int x, y , start_item, nb_displayed_items;
  TargetChooser * tc;
  JGuiListener * listener;
  CardDisplay();
  CardDisplay(int id, GameObserver* game, int x, int y, JGuiListener * listener = NULL, TargetChooser * tc = NULL, int nb_displayed_items = 7);
  void AddCard(MTGCardInstance * _card);
  void rotateLeft();
  void rotateRight();
  bool CheckUserInput(u32 key);
  void Render();
  void init(MTGGameZone * zone);
  virtual ostream& toString(ostream& out) const;
};



class DefaultTargetDisplay:CardDisplay{
 public:
  DefaultTargetDisplay(int id, GameObserver* game, int x, int y, JGuiListener * listener, int nb_displayed_items );
  ~DefaultTargetDisplay();
};

std::ostream& operator<<(std::ostream& out, const CardDisplay& m);

#endif
