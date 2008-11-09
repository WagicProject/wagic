#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <JGE.h>
#include "ManaCost.h"
#include "MTGGameZones.h"
#include "Damage.h"
#include "Targetable.h"

class MTGPlayerCards;
class MTGInPlay;

class Player: public Damageable, public Targetable{
 protected:
  ManaCost * manaPool;

 public:
	int typeAsTarget(){return TARGET_PLAYER;}
	virtual int displayStack(){return 1;}
  JTexture * mAvatarTex;
  JQuad * mAvatar;
  int canPutLandsIntoPlay;
  MTGPlayerCards * game;	
	int testLife();
	int afterDamage();
  Player(MTGPlayerCards * _deck, char * deckFile);
	virtual ~Player();
  void unTapPhase();
  MTGInPlay * inPlay();
  ManaCost * getManaPool();
	int manaBurn();
	void cleanupPhase();
	virtual int Act(float dt){return 0;};
	virtual int isAI(){return 0;};
	Player * opponent();
	int getId();
	JQuad * getIcon();
	string deckFile;
};

class HumanPlayer: public Player{
 public:
  HumanPlayer(MTGPlayerCards * _deck, char * _deckFile);
  
};




#endif
