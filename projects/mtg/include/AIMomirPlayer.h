#ifndef _AIMOMIRPLAYER_H_
#define _AIMOMIRPLAYER_H_

#include "AIPlayer.h"


class AIMomirPlayer:public AIPlayerBaka{
public:
  AIMomirPlayer(MTGDeck * deck, string file, string fileSmall, string avatarFile);
  int getEfficiency(AIAction * action);
  int momir();
  int computeActions();
  static MTGAbility * momirAbility;
  static MTGAbility * getMomirAbility();
};

#endif
