#include "../include/config.h"
#include "../include/Player.h"
#include "../include/GameObserver.h"
#include "../include/DeckStats.h"
#include "../include/ManaCost.h"


Player::Player(MTGPlayerCards * deck, string file, string fileSmall) : Damageable(20){
  deckFile = file;
  deckFileSmall = fileSmall;
  game = deck;
  game->setOwner(this);
  manaPool = NEW ManaPool(this);
  canPutLandsIntoPlay = 1;
  castedspellsthisturn = 0;
  castrestrictedspell = 0;
  castrestrictedcreature = 0;
  cantcastcreature = 0;
  cantcastspell = 0;
  cantcastinso = 0;
  onlyonecast = 0;
  poisonCount = 0;
  damageCount = 0;
  preventable = 0;
  mAvatar = NULL;
  mAvatarTex = NULL;
  type_as_damageable = DAMAGEABLE_PLAYER;
}

/*Method to call at the end of a game, before all objects involved in the game are destroyed */
void Player::End(){
  DeckStats::GetInstance()->saveStats(this, opponent(),GameObserver::GetInstance());
}

Player::~Player(){
  SAFE_DELETE(manaPool);
  resources.Release(mAvatarTex);
  mAvatar = NULL;
  mAvatarTex = NULL;
}

void Player::loadAvatar(string file){
  if (mAvatarTex) {
    resources.Release(mAvatarTex);
    mAvatar = NULL;
    mAvatarTex = NULL;
  }
  mAvatarTex = resources.RetrieveTexture(file,RETRIEVE_LOCK,TEXTURE_SUB_AVATAR);
  if (mAvatarTex)
    mAvatar = resources.RetrieveQuad(file,0,0,35,50,"playerAvatar",RETRIEVE_NORMAL,TEXTURE_SUB_AVATAR);
  else 
    mAvatar = NULL;
}

const string Player::getDisplayName() const {
  GameObserver  * g = GameObserver::GetInstance();
  if (this == g->players[0]) return "Player 1";
  return "Player 2";
}

MTGInPlay * Player::inPlay(){
  return game->inPlay;
}

int Player::getId(){
  GameObserver * game = GameObserver::GetInstance();
  for (int i= 0; i < 2; i++){
    if (game->players[i] == this) return i;
  }
  return -1;
}

JQuad * Player::getIcon(){
  return mAvatar;
}

Player * Player::opponent(){
  GameObserver * game = GameObserver::GetInstance();
  if (!game) return NULL;
  return this == game->players[0] ? game->players[1] : game->players[0];
}

HumanPlayer::HumanPlayer(MTGPlayerCards * deck, string file, string fileSmall) : Player(deck, file, fileSmall) {
  loadAvatar("avatar.jpg");
}



ManaPool * Player::getManaPool(){
  return manaPool;
}

int Player::afterDamage(){
  return life;
}
int Player::poisoned(){
  return poisonCount;
}
int Player::damaged(){
  return damageCount;
}
int Player::prevented(){
  return preventable;
}
//Cleanup phase at the end of a turn
void Player::cleanupPhase(){
  game->inPlay->cleanupPhase();
  game->graveyard->cleanupPhase();
}

ostream& operator<<(ostream& out, const Player& p)
{
  return out << p.getDisplayName();
}
