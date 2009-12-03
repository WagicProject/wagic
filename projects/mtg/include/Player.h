#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "JGE.h"
#include "MTGGameZones.h"
#include "Damage.h"
#include "Targetable.h"

class MTGPlayerCards;
class MTGInPlay;
class ManaPool;

class Player: public Damageable{
 protected:
  ManaPool * manaPool;

 public:
  virtual void End();
  int typeAsTarget(){return TARGET_PLAYER;}
  const string getDisplayName() const;
  virtual int displayStack(){return 1;}
  JTexture * mAvatarTex;
  JQuad * mAvatar;
  int canPutLandsIntoPlay;
  MTGPlayerCards * game;
  int afterDamage();
  Player(MTGPlayerCards * deck, string deckFile, string deckFileSmall);
  virtual ~Player();
  void unTapPhase();
  MTGInPlay * inPlay();
  ManaPool * getManaPool();
  void cleanupPhase();
  virtual int Act(float dt){return 0;};
  virtual int isAI(){return 0;};
  Player * opponent();
  int getId();
  JQuad * getIcon();
  string deckFile;
  string deckFileSmall;

  virtual int receiveEvent(WEvent * event){return 0;};
  virtual void Render(){};
};

class HumanPlayer: public Player{
 public:
  HumanPlayer(MTGPlayerCards * deck, string deckFile, string deckFileSmall);

};

ostream& operator<<(ostream&, const Player&);

#endif
