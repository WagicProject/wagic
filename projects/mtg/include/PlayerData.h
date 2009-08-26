#ifndef _PLAYER_DATA_H_
#define _PLAYER_DATA_H_

#include "../include/MTGDeck.h"

class PlayerData{
 protected:
 public:
  int credits;
  MTGDeck * collection;
  PlayerData(MTGAllCards * allcards);
  ~PlayerData();
  int save();
};

#endif
