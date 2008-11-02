#ifndef _GROUPOFCARDS_H_
#define _GROUPOFCARDS_H_

#include "GameObserver.h"
#include "MTGCardInstance.h"

#define FILTER_SUBTYPE 1

#define GROUP_LIMIT 10

class GroupOfCards{
 protected:
  GameObserver * game;
 public:
  GroupOfCards(GameObserver * _game);
  //virtual ~GroupOfCards();
  virtual int includes(MTGCardInstance * card){return 0;};
};

class GroupOfCreatures:public GroupOfCards{
 protected:
  int filter;
  int filterValue;
 public:
  GroupOfCreatures(GameObserver * _game, int _filter, int _filterValue);
  int includes(MTGCardInstance * card);
};

class GroupOfSpecificCards:public GroupOfCards{
 protected:
  int nb_cards;
  MTGCardInstance * cards[GROUP_LIMIT];
 public:
  int includes(MTGCardInstance * card);
  GroupOfSpecificCards(GameObserver * _game, MTGCardInstance * _creatures[], int _nb_creatures);
  GroupOfSpecificCards(GameObserver * _game, MTGCardInstance * card);
};

#endif
