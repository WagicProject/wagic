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
  static int currentId;
public:
  MTGAbility * ability;
  Player * player;
  int id;
  MTGCardInstance * click;
  MTGCardInstance * target; // TODO Improve
  AIAction(MTGAbility * a, MTGCardInstance * c, MTGCardInstance * t = NULL):ability(a),click(c),target(t){player = NULL; efficiency = -1; id = currentId++;};
  AIAction(MTGCardInstance * c, MTGCardInstance * t = NULL):click(c),target(t){player = NULL; ability = NULL; efficiency = -1; id = currentId++;};
  AIAction(Player * p):player(p){ability = NULL; target = NULL; click = NULL; efficiency = -1;};
  int getEfficiency();
  int Act();
  

};

class CmpAbilities { // compares Abilities efficiency
 public:
  bool operator()(AIAction * a1, AIAction * a2) const {
    int e1 = a1->getEfficiency();
    int e2 = a2->getEfficiency();
    if (e1 == e2) return a1->id < a2->id;
    return (e1 > e2);
  }
};

class AIPlayer: public Player{
 protected:
  MTGCardInstance * nextCardToPlay;
  int agressivity;
  queue<AIAction *> clickstream;
  void tapLandsForMana(ManaCost * cost, MTGCardInstance * card = NULL);
  int orderBlockers();
  int combatDamages();
  int interruptIfICan();
  int chooseAttackers();
  int chooseBlockers();
  int canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy);
  int effectBadOrGood(MTGCardInstance * card, int mode = MODE_PUTINTOPLAY, TargetChooser * tc = NULL);
  int getCreaturesInfo(Player * player, int neededInfo = INFO_NBCREATURES , int untapMode = 0, int canAttack = 0);
  AIStats * getStats();
  //Variables used by Test suite
 public:
  bool forceBestAbilityUse;
  void End(){};
  virtual int displayStack() {return 0;};
  int receiveEvent(WEvent * event);
  void Render();
  AIStats * stats;
  ManaCost * getPotentialMana(MTGCardInstance * card = NULL);
  AIPlayer(MTGPlayerCards * deck, string deckFile, string deckFileSmall);
  virtual ~AIPlayer();
  virtual MTGCardInstance * chooseCard(TargetChooser * tc, MTGCardInstance * source, int random = 0);
  virtual int chooseTarget(TargetChooser * tc = NULL, Player * forceTarget =NULL);
  virtual int Act(float dt);
  virtual int affectCombatDamages(CombatStep);
  int isAI(){return 1;};
  int canHandleCost(MTGAbility * ability);
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
  int deckId;
  AIPlayerBaka(MTGPlayerCards * deck, string deckFile, string deckfileSmall, string avatarFile);
  virtual int Act(float dt);
  void initTimer();
  virtual int computeActions();
};

class AIPlayerFactory{
 public:
  AIPlayer * createAIPlayer(MTGAllCards * collection, Player * opponent, int deckid = 0);
};


#endif
