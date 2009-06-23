#ifndef _MTG_CARD_INSTANCE_H_
#define _MTG_CARD_INSTANCE_H_

#include "MTGCard.h"
#include "MTGGameZones.h"
#include "MTGAbility.h"
#include "TexturesCache.h"
#include "ManaCost.h"
#include "Blocker.h"
#include "Damage.h"
#include "Targetable.h"


class MTGCardInstance;
class MTGPlayerCards;
class MTGAbility;
class MTGCard;
class TexturesCache;
class ManaCost;
class Blockers;
class CardDescriptor;
class Counters;



class MTGCardInstance: public MTGCard, public Damageable {
 protected:
  int untapping;
  int nb_damages;
  string sample;

  int lifeOrig;
  Blockers * blockers;
  MTGPlayerCards * belongs_to;
  MTGAbility * UntapBlockers[10];
  void unband();
  MTGCardInstance * getNextPartner();
  void initMTGCI();
 public:
  int regenerateTokens;
  bool isToken;
  int stillInUse();
  Player * lastController;
  MTGGameZone * getCurrentZone();
  MTGGameZone * previousZone;
  MTGCardInstance * previous;
  MTGCardInstance * next;
  int doDamageTest;
  int summoningSickness;
  // The recommended method to test for summoning Sickness !
  int hasSummoningSickness();
  MTGCardInstance * changeController(Player * newcontroller);
  MTGCardInstance * defenser;
  float changedZoneRecently;
  Player * owner;
  Counters * counters;
  int typeAsTarget(){return TARGET_CARD;}
  int attacker;
  MTGCardInstance * banding; // If belongs to a band when attacking
  MTGCardInstance * target;
  int tapped;
  void addType(int type);
  int canBlock();
  int canBlock(MTGCardInstance * opponent);
  int canAttack();
  int afterDamage();
  void setUntapping();
  int isUntapping();
  int has(int ability);
  int cleanup();
  int reset();
  int isAttacker();
  MTGCardInstance * isDefenser();
  int toggleDefenser(MTGCardInstance * opponent);
  int toggleAttacker();
  MTGCard * model;
  MTGCardInstance();
  MTGCardInstance(MTGCard * card, MTGPlayerCards * _belongs_to);
  Blockers * getBlockers();
  int regenerate();
  int triggerRegenerate();
  Player * controller();
  JQuad * getIcon();
  int initAttackersDefensers();
  MTGCardInstance * getNextOpponent(MTGCardInstance * previous=NULL);
  int nbOpponents();
  ~MTGCardInstance();
  int bury();
  int destroy();


  int addToToughness(int value);
  int setToughness(int value);

  CardDescriptor * protections[10];
  int nbprotections;
  int addProtection(CardDescriptor * cd);
  int removeProtection(CardDescriptor *cd, int erase = 0);
  int protectedAgainst(MTGCardInstance * card);
  void copy(MTGCardInstance * card);
  // in game
  int isTapped();
  void untap();
  void tap();
  int isInPlay();
  void resetAllDamage();
  JSample * getSample();
};




#endif
