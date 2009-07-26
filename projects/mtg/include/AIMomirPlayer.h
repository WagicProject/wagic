#ifndef _AIMOMIRPLAYER_H_
#define _AIMOMIRPLAYER_H_

#include "AIPlayer.h"


class AIMomirPlayer:public AIPlayerBaka{
public:
  AIMomirPlayer(MTGPlayerCards * _deck, char * file, const char * fileSmall, char * avatarFile);
  int getEfficiency(AIAction * action);
  int momir();
  int computeActions();
  static MTGAbility * momirAbility;
  static MTGAbility * getMomirAbility();
};

#endif