#ifndef _PLAYER_DATA_H_
#define _PLAYER_DATA_H_

#define PLAYER_SAVEFILE "Res/player/data.dat"

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
