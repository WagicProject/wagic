#include "../include/config.h"
#include "../include/MTGGameZones.h"
#include "../include/Player.h"
#include "../include/GameOptions.h"
#include "../include/WEvent.h"
#include "../include/MTGDeck.h"

#if defined (WIN32) || defined (LINUX)
#include <time.h>
#endif

//------------------------------
//Players Game
//------------------------------

MTGPlayerCards::MTGPlayerCards(MTGAllCards * _collection, int * idList, int idListSize){

  init();
  int i;
  collection = _collection;
  for (i=0;i<idListSize;i++){
    MTGCard * card =  collection->getCardById(idList[i]);
    if (card){
      MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
      library->addCard(newCard);
    }
  }
  


}



MTGPlayerCards::MTGPlayerCards(MTGAllCards * _collection,MTGDeck * deck){
  init();
  collection = _collection;
  map<int,int>::iterator it;
  for (it = deck->cards.begin(); it!=deck->cards.end(); it++){
    MTGCard * card = deck->getCardById(it->first);
    if (card){
      for (int i = 0; i < it->second; i++){
        MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
        library->addCard(newCard);
      }
    }
  }
}

MTGPlayerCards::~MTGPlayerCards(){
  SAFE_DELETE(library);
  SAFE_DELETE(graveyard);
  SAFE_DELETE(hand);
  SAFE_DELETE(inPlay);
  SAFE_DELETE(stack);
  SAFE_DELETE(removedFromGame);
  SAFE_DELETE(garbage);
}

void MTGPlayerCards::setOwner(Player * player){
  library->setOwner(player);
}

void MTGPlayerCards::initGame(int shuffle, int draw){
  if (shuffle) library->shuffle();
  if (draw){
    for (int i=0;i<7;i++){ 
      OutputDebugString("draw\n");
      drawFromLibrary();
    }
  }
}

void MTGPlayerCards::drawFromLibrary(){
  MTGCardInstance * drownCard = library->draw();
  hand->addCard(drownCard);
}

void MTGPlayerCards::init(){
  library = NEW MTGLibrary();
  graveyard = NEW MTGGraveyard();
  hand = NEW MTGHand();
  inPlay = NEW MTGInPlay();
  stack = NEW MTGStack();
  removedFromGame = NEW MTGRemovedFromGame();
  garbage = NEW MTGGameZone();
}


void MTGPlayerCards::showHand(){
  hand->debugPrint();
}


MTGCardInstance * MTGPlayerCards::putInPlay(MTGCardInstance * card){
  MTGCardInstance * copy = hand->removeCard(card);
  if(!copy) copy = stack->removeCard(card); //Which one is it ???

  inPlay->addCard(copy);
  copy->summoningSickness = 1;
  copy->changedZoneRecently = 1.f;
  return copy;
}

MTGCardInstance * MTGPlayerCards::putInGraveyard(MTGCardInstance * card){
  MTGCardInstance * copy = NULL;
  if (inPlay->hasCard(card)){
    copy = putInZone(card,inPlay, graveyard);
  }else if (stack->hasCard(card)){
    copy = putInZone(card,stack, graveyard);
  }else{
    copy = putInZone(card,hand, graveyard);
  }
  return copy;

}

MTGCardInstance * MTGPlayerCards::putInZone(MTGCardInstance * card, MTGGameZone * from, MTGGameZone * to){
  MTGCardInstance * copy = NULL;
  GameObserver *g = GameObserver::GetInstance();
  if (!from || !to) return card; //Error check

  if ((copy = from->removeCard(card))){

    if (GameOptions::GetInstance()->values[OPTIONS_SFXVOLUME].getIntValue() > 0){
      if (to == graveyard){
        if (card->isACreature()){
          JSample * sample = SampleCache::GetInstance()->getSample("sound/sfx/graveyard.wav");
          if (sample) JSoundSystem::GetInstance()->PlaySample(sample);
        }
      }
    }

    if (card->isToken){
      if (to != g->players[0]->game->inPlay && to != g->players[1]->game->inPlay){
        garbage->addCard(copy);
        return NULL;
      }
    }
    
    to->addCard(copy);
    copy->changedZoneRecently = 1.f;
    GameObserver *g = GameObserver::GetInstance();
    WEvent * e = NEW WEventZoneChange(copy, from, to);
    g->receiveEvent(e);
    delete e;
    return copy;
  }
  return card; //Error
}

void MTGPlayerCards::discardRandom(MTGGameZone * from){
  if (!from->nb_cards)
    return;
  int r = rand() % (from->nb_cards);
  putInZone(from->cards[r],from, graveyard);
}

int MTGPlayerCards::isInPlay(MTGCardInstance * card){
  if (inPlay->hasCard(card)){
    return 1;
  }
  return 0;
}

//--------------------------------------
// Zones specific code
//--------------------------------------

MTGGameZone::MTGGameZone(){
  nb_cards= 0;
  lastCardDrawn = NULL;
}

MTGGameZone::~MTGGameZone(){
  for (int i=0; i<nb_cards; i++) {
    delete cards[i];
  }
}

void MTGGameZone::setOwner(Player * player){
  for (int i=0; i<nb_cards; i++) {
    cards[i]->owner = player;
  }
  owner = player;
}

MTGCardInstance * MTGGameZone::removeCard(MTGCardInstance * card, int createCopy){
  int i;
  cardsMap.erase(card);
  for (i=0; i<(nb_cards); i++) {
    if (cards[i] == card){
      //cards[i] = cards[nb_cards -1];
      nb_cards--;
      cards.erase(cards.begin()+i);
	    MTGCardInstance * copy = card;
      if (card->isToken){ //TODO better than this ?
        return card;
      }
      card->lastController = card->controller();
      if (createCopy) {
		    copy = NEW MTGCardInstance(card->model,card->owner->game);
		    copy->previous = card;
		    card->next = copy;
	    }
      copy->previousZone = this;
      return copy;
    }
  }
  return NULL;

}

MTGCardInstance * MTGGameZone::hasCard(MTGCardInstance * card){
  if (cardsMap.find(card) != cardsMap.end()) return card;
  return NULL;

}

int MTGGameZone::countByType(const char * value){
  int result = 0 ;
  for (int i=0; i<(nb_cards); i++) {
    if (cards[i]->hasType(value)){
      result++;
    }
  }
  return result;

}

int MTGGameZone::hasType(const char * value){
  for (int i=0; i<(nb_cards); i++) {
    if (cards[i]->hasType(value)){
      return 1;
    }
  }
  return 0;
}


void MTGGameZone::cleanupPhase(){
  for (int i=0; i<(nb_cards); i++)
    (cards[i])->cleanup();
}

void MTGGameZone::shuffle(){
  int i;
  for (i=0; i<(nb_cards); i++) {
    int r = i + (rand() % (nb_cards-i)); // Random remaining position.
    MTGCardInstance * temp = cards[i]; cards[i] = cards[r]; cards[r] = temp;
  }
  //srand(time(0));  // initialize seed "randomly" TODO :improve
}



void MTGGameZone::addCard(MTGCardInstance * card){
  if (!card) return;
  cards.push_back(card);
  nb_cards++;
  cardsMap[card] = 1;

}

MTGCardInstance * MTGGameZone::draw(){
  if (!nb_cards) return NULL;
  nb_cards--;
  lastCardDrawn = cards[nb_cards];
  cards.pop_back();
  cardsMap.erase( lastCardDrawn);
  return lastCardDrawn;
}

MTGCardInstance * MTGLibrary::draw(){
  if (!nb_cards) {
    GameObserver::GetInstance()->gameOver = this->owner;
  }
  return MTGGameZone::draw();
}

void MTGGameZone::debugPrint(){
  int i;
  for (i=0;i<nb_cards;i++){
    MTGCard * card = cards[i];
    fprintf(stderr, "%s", card->getName());
  }
}





//------------------------------
int MTGInPlay::nbDefensers( MTGCardInstance * attacker){
  int result = 0;
  MTGCardInstance * defenser = getNextDefenser(NULL, attacker);
  while (defenser){
    result++;
    defenser = getNextDefenser(defenser, attacker);
  }
  return result;
}

//Return the number of creatures this card is banded with
//Number of creatures in the band is n+1 !!!
int MTGInPlay::nbPartners(MTGCardInstance * attacker){
  int result = 0;
  if (!attacker->banding) return 0;
  for (int i = 0; i < nb_cards; i ++){
    if (cards[i]->banding == attacker->banding) result++;
  }
  return result;
}

MTGCardInstance *  MTGInPlay::getNextDefenser(MTGCardInstance * previous, MTGCardInstance * attacker){
  int foundprevious = 0;
  if (previous == NULL){
    foundprevious = 1;
  }
  for (int i = 0; i < nb_cards; i ++){
    MTGCardInstance * current = cards[i];
    if (current == previous){
      foundprevious = 1;
    }else if (foundprevious && current->isDefenser() == attacker){
      return current;
    }
  }
  return NULL;
}

MTGCardInstance *  MTGInPlay::getNextAttacker(MTGCardInstance * previous){
  int foundprevious = 0;
  if (previous == NULL){
    foundprevious = 1;
  }
  for (int i = 0; i < nb_cards; i ++){
    MTGCardInstance * current = cards[i];
    if (current == previous){
      foundprevious = 1;
    }else if (foundprevious && current->isAttacker()){
      return current;
    }
  }
  return NULL;
}

void MTGInPlay::untapAll(){
  int i;
  for (i = 0; i < nb_cards; i ++){
    cards[i]->setUntapping();
    if (cards[i]->getBlockers()->isEmpty()){
#if defined (WIN32) || defined (LINUX)
      char buf[4096];
      sprintf(buf, "Can untap %s\n", cards[i]->getName());
      OutputDebugString(buf);
#endif
      cards[i]->untap();
    }
  }
}


//--------------------------
void MTGLibrary::shuffleTopToBottom(int nbcards){
  if (nbcards>nb_cards) nbcards = nb_cards;
  MTGCardInstance * _cards[MTG_MAX_PLAYER_CARDS];
  for (int i= nb_cards-nbcards; i<(nb_cards); i++) {
    int r = i + (rand() % (nbcards-i)); // Random remaining position.
    MTGCardInstance * temp = cards[i]; cards[i] = cards[r]; cards[r] = temp;
  }
  for (int i= 0; i < nbcards; i++){
    _cards[i] = cards[nb_cards - 1 - i];
  }
  for (int i = nbcards; i < nb_cards; i++){
    _cards[i] = cards[i - nb_cards];
  }
  for (int i=0 ; i < nb_cards; i++){
    cards[i] = _cards[i];
  }
}


MTGGameZone * MTGGameZone::stringToZone(string zoneName, MTGCardInstance * source,MTGCardInstance * target){
  Player *p, *p2;
  GameObserver * g = GameObserver::GetInstance();
  if (!source) p = g->currentlyActing();
  else p = source->controller();
  if (!target){
    p2 = p;
    target = source;//hack ?
  }
  else p2 = target->controller();
  if(zoneName.compare("mygraveyard") == 0)return p->game->graveyard;
  if(zoneName.compare("opponentgraveyard") == 0) return p->opponent()->game->graveyard;
  if(zoneName.compare("targetownergraveyard") == 0) return target->owner->game->graveyard;
  if(zoneName.compare("targetcontrollergraveyard") == 0) return p2->game->graveyard;
  if(zoneName.compare("ownergraveyard") == 0) return target->owner->game->graveyard;
  if(zoneName.compare("graveyard") == 0) return target->owner->game->graveyard;

  if(zoneName.compare("myinplay") == 0)return p->game->inPlay;
  if(zoneName.compare("opponentinplay") == 0) return p->opponent()->game->inPlay;
  if(zoneName.compare("targetownerinplay") == 0) return target->owner->game->inPlay;
  if(zoneName.compare("targetcontrollerinplay") == 0) return p2->game->inPlay;
  if(zoneName.compare("ownerinplay") == 0) return target->owner->game->inPlay;
  if(zoneName.compare("inplay") == 0) return p->game->inPlay;

  if(zoneName.compare("myhand") == 0)return p->game->hand;
  if(zoneName.compare("opponenthand") == 0) return p->opponent()->game->hand;
  if(zoneName.compare("targetcontrollerhand") == 0) return p2->game->hand;
  if(zoneName.compare("targetownerhand") == 0) return target->owner->game->hand;
  if(zoneName.compare("ownerhand") == 0) return target->owner->game->hand;
  if(zoneName.compare("hand") == 0) return target->owner->game->hand;

  if(zoneName.compare("myremovedfromgame") == 0)return p->game->removedFromGame;
  if(zoneName.compare("opponentremovedfromgame") == 0) return p->opponent()->game->removedFromGame;
  if(zoneName.compare("targetcontrollerremovedfromgame") == 0) return p2->game->removedFromGame;
  if(zoneName.compare("targetownerremovedfromgame") == 0) return target->owner->game->removedFromGame;
  if(zoneName.compare("ownerremovedfromgame") == 0) return target->owner->game->removedFromGame;
  if(zoneName.compare("removedfromgame") == 0) return target->owner->game->removedFromGame;

  if(zoneName.compare("mylibrary") == 0)return p->game->library;
  if(zoneName.compare("opponentlibrary") == 0) return p->opponent()->game->library;
  if(zoneName.compare("targetownerlibrary") == 0) return target->owner->game->library;
  if(zoneName.compare("targetcontrollerlibrary") == 0) return p2->game->library;
  if(zoneName.compare("ownerlibrary") == 0) return target->owner->game->library;
  if(zoneName.compare("library") == 0) return p->game->library;
  return NULL;
}
