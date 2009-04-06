/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _IAPLAYER_H
#define _IAPLAYER_H

#include "Player.h"

#include <queue>
using std::queue;

#define INFO_NBCREATURES 0
#define INFO_CREATURESPOWER 1


class AIStats;

class AIAction{
protected:
  int efficiency;
public:
  MTGAbility * ability;
  Player * player;
  MTGCardInstance * click;
  MTGCardInstance * target; // TODO Improve
  AIAction(MTGAbility * a, MTGCardInstance * c, MTGCardInstance * t = NULL):ability(a),click(c),target(t){player = NULL; efficiency = -1;};
  AIAction(MTGCardInstance * c, MTGCardInstance * t = NULL):click(c),target(t){player = NULL; ability = NULL; efficiency = -1;};
  AIAction(Player * p):player(p){ability = NULL; target = NULL; click = NULL; efficiency = -1;};
  int getEfficiency();
  int Act();

};

class CmpAbilities { // compares Abilities efficiency
 public:
  bool operator()(AIAction * a1, AIAction * a2) const {
    int e1 = a1->getEfficiency();
    int e2 = a2->getEfficiency();
    if (e1 == e2) return a1 > a2; //TODO improve
    return (e1 > e2);
  }
};

class AIPlayer: public Player{
 protected:
  MTGCardInstance * nextCardToPlay;
  int agressivity;
  ManaCost * potentialMana;
  queue<AIAction *> clickstream;
  void tapLandsForMana(ManaCost * potentialMana, ManaCost * cost);
  int combatDamages();
  int interruptIfICan();
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
  int selectAbility();
  int createAbilityTargets(MTGAbility * a, MTGCardInstance * c, map<AIAction *, int,CmpAbilities> * ranking);
  int useAbility();
  virtual int getEfficiency(AIAction * action);

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
  virtual int computeActions();
};

class AIPlayerFactory{
 public:
  AIPlayer * createAIPlayer(MTGAllCards * collection, MTGPlayerCards * oponents_deck, int deckid = 0);
};


#endif
