/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _IAPLAYER_H
#define _IAPLAYER_H

#include "Player.h"


#define INFO_NBCREATURES 0
#define INFO_CREATURESPOWER 1


class AIStats;

class AIPlayer: public Player{
 protected:
  MTGCardInstance * nextCardToPlay;
  ManaCost * potentialMana;
  void tapLandsForMana(ManaCost * potentialMana, ManaCost * cost);
  int checkInterrupt();
  int combatDamages();
  int chooseAttackers();
  int chooseBlockers();
  int effectBadOrGood(MTGCardInstance * card);
  int getCreaturesInfo(Player * player, int neededInfo = INFO_NBCREATURES , int untapMode = 0, int canAttack = 0);
  AIStats * getStats();
 public:
   void End(){};
  virtual int displayStack(){return 0;};
  AIStats * stats;
  ManaCost * getPotentialMana();
  AIPlayer(MTGPlayerCards * _deck, string deckFile);
  virtual ~AIPlayer();
  virtual MTGCardInstance * chooseCard(TargetChooser * tc, MTGCardInstance * source, int random = 0);
  virtual int chooseTarget(TargetChooser * tc = NULL);
  virtual int Act(float dt);
  int isAI(){return 1;};

};


class AIPlayerBaka: public AIPlayer{
 protected:
  int oldGamePhase;
  float timer;
  MTGCardInstance * FindCardToPlay(ManaCost * potentialMana, const char * type);
 public:
  AIPlayerBaka(MTGPlayerCards * _deck, char * deckFile, char * avatarFile);
  virtual int Act(float dt);
  void initTimer();
};

class AIPlayerFactory{
 public:
  AIPlayer * createAIPlayer(MTGAllCards * collection, MTGPlayerCards * oponents_deck, int deckid = 0);
};


#endif
