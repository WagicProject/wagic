#ifndef _CARD_DISPLAY_H_
#define _CARD_DISPLAY_H_

#include "../include/PlayGuiObjectController.h"

class TargetChooser;
class MTGGameZone;
class MTGCardInstance;

class CardDisplay:public PlayGuiObjectController{
 public:
  int x, y , start_item, nb_displayed_items;
  TargetChooser * tc;
  JGuiListener * listener;
  CardDisplay();
  CardDisplay(int id, GameObserver* _game, int _x, int _y, JGuiListener * _listener, TargetChooser * _tc = NULL, int _nb_displayed_items = 7 );
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
  DefaultTargetDisplay(int id, GameObserver* _game, int _x, int _y,  JGuiListener * _listener, int _nb_displayed_items );
  ~DefaultTargetDisplay();
};

std::ostream& operator<<(std::ostream& out, const CardDisplay& m);

#endif
