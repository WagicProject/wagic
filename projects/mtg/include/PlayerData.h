#ifndef _PLAYER_DATA_H_
#define _PLAYER_DATA_H_

#include "../include/MTGDeck.h"
#include "../include/Tasks.h"

class PlayerData{
 protected:
 public:
  int credits;
  MTGDeck * collection;
  TaskList * taskList;
  PlayerData(MTGAllCards * allcards);
  ~PlayerData();
  int save();
};

#endif
