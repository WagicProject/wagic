#include "../include/config.h"
#include "../include/MTGGameZones.h"
#include "../include/Player.h"
#include "../include/GameOptions.h"
#include "../include/WEvent.h"
#include "../include/MTGDeck.h"
#include <assert.h>

#if defined (WIN32) || defined (LINUX)
#include <time.h>
#endif

//------------------------------
//Players Game
//------------------------------

MTGPlayerCards::MTGPlayerCards(int * idList, int idListSize){
  init();
  int i;
  for (i=0;i<idListSize;i++){
    MTGCard * card =  GameApp::collection->getCardById(idList[i]);
    if (card){
      MTGCardInstance * newCard = NEW MTGCardInstance(card, this);
      library->addCard(newCard);
    }
  }
}

MTGPlayerCards::MTGPlayerCards(MTGDeck * deck){
  init();
  initDeck(deck);
}


void MTGPlayerCards::initDeck(MTGDeck * deck){
  resetLibrary();
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
  SAFE_DELETE(temp);
}

void MTGPlayerCards::setOwner(Player * player){
  library->setOwner(player);
  graveyard->setOwner(player);
  hand->setOwner(player);
  inPlay->setOwner(player);
  removedFromGame->setOwner(player);
  stack->setOwner(player);
  garbage->setOwner(player);
  temp->setOwner(player);
}

void MTGPlayerCards::initGame(int shuffle, int draw){
  if (shuffle) library->shuffle();
  if (draw){
    for (int i=0;i<7;i++){
      drawFromLibrary();
    }
  }
}

void MTGPlayerCards::drawFromLibrary(){
  if (!library->nb_cards) {
	    int cantlosers = 0;
        MTGGameZone * z = library->owner->game->inPlay;
        int nbcards = z->nb_cards;
        for (int i = 0; i < nbcards; ++i){
          MTGCardInstance * c = z->cards[i];
		  if (c->has(Constants::CANTLOSE) || c->has(Constants::CANTMILLLOSE)){
            cantlosers++;
          }
         }
	  if(cantlosers < 1){
		  GameObserver::GetInstance()->gameOver = library->owner;}
     return;
  }
  MTGCardInstance * toMove = library->cards[library->nb_cards-1];
  library->lastCardDrawn = toMove;
  putInZone(toMove, library, hand);
}

void MTGPlayerCards::resetLibrary(){
  SAFE_DELETE(library);
  library = NEW MTGLibrary();
}

void MTGPlayerCards::init(){
  library = NEW MTGLibrary();
  graveyard = NEW MTGGraveyard();
  hand = NEW MTGHand();
  inPlay = NEW MTGInPlay();
  battlefield=inPlay;

  stack = NEW MTGStack();
  removedFromGame = NEW MTGRemovedFromGame();
  exile = removedFromGame;
  garbage = NEW MTGGameZone();
  temp = NEW MTGGameZone();
}


void MTGPlayerCards::showHand(){
  hand->debugPrint();
}

MTGCardInstance * MTGPlayerCards::putInGraveyard(MTGCardInstance * card){
  MTGCardInstance * copy = NULL;
  MTGGraveyard * grave = card->owner->game->graveyard;
  if (inPlay->hasCard(card)){
    copy = putInZone(card,inPlay, grave);
  }else if (stack->hasCard(card)){
    copy = putInZone(card,stack, grave);
  }else{
    copy = putInZone(card,hand, grave);
  }
  return copy;

}

MTGCardInstance * MTGPlayerCards::putInExile(MTGCardInstance * card){
  MTGCardInstance * copy = NULL;
  MTGRemovedFromGame * exile = card->owner->game->exile;
  if (inPlay->hasCard(card)){
    copy = putInZone(card,inPlay, exile);
  }else if (stack->hasCard(card)){
    copy = putInZone(card,stack, exile);
  }else if(graveyard->hasCard(card)){
	  copy = putInZone(card,graveyard, exile);
  }else{
    copy = putInZone(card,hand, exile);
  }
   return copy;
}
MTGCardInstance * MTGPlayerCards::putInLibrary(MTGCardInstance * card){
  MTGCardInstance * copy = NULL;
  MTGLibrary * library = card->owner->game->library;
  MTGHand * hand = card->owner->game->hand;
   if (inPlay->hasCard(card)){
       copy = putInZone(card,inPlay, library);
  }else if (stack->hasCard(card)){
    copy = putInZone(card,stack, library);
  }else if(graveyard->hasCard(card)){
	  copy = putInZone(card,graveyard, library);
  }else{
    copy = putInZone(card,hand, library);
  }
   return copy;
}

MTGCardInstance * MTGPlayerCards::putInHand(MTGCardInstance * card){
  MTGCardInstance * copy = NULL;
  MTGHand * hand = card->owner->game->hand;
  if (inPlay->hasCard(card)){
    copy = putInZone(card,inPlay, hand);
  }else if (stack->hasCard(card)){
    copy = putInZone(card,stack, hand);
  }else if(graveyard->hasCard(card)){
	  copy = putInZone(card,graveyard, hand);
  }else{
    copy = putInZone(card,hand, hand);
  }
   return copy;
}

MTGCardInstance * MTGPlayerCards::putInZone(MTGCardInstance * card, MTGGameZone * from, MTGGameZone * to){
  MTGCardInstance * copy = NULL;
  GameObserver *g = GameObserver::GetInstance();
  if (!from || !to) return card; //Error check

  int doCopy = 1;
  //When a card is moved from inPlay to inPlay (controller change, for example), it is still the same object
  if ((to == g->players[0]->game->inPlay || to == g->players[1]->game->inPlay) &&
    (from == g->players[0]->game->inPlay || from == g->players[1]->game->inPlay)){
      doCopy = 0;
  }

  if ((copy = from->removeCard(card,doCopy))){
    if (options[Options::SFXVOLUME].number > 0){
      if (to == g->players[0]->game->graveyard || to == g->players[1]->game->graveyard){
        if (card->isCreature()){
          JSample * sample = resources.RetrieveSample("graveyard.wav");
          if (sample) JSoundSystem::GetInstance()->PlaySample(sample);
        }
      }
    }

    MTGCardInstance * ret = copy;

    to->addCard(copy);
    GameObserver *g = GameObserver::GetInstance();
    WEvent * e = NEW WEventZoneChange(copy, from, to);
    g->receiveEvent(e);

    return ret;
  }
  return card; //Error
}

void MTGPlayerCards::discardRandom(MTGGameZone * from){
  if (!from->nb_cards)
    return;
  int r = WRand() % (from->nb_cards);
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

MTGGameZone::MTGGameZone() : nb_cards(0), lastCardDrawn(NULL), needShuffle(false) {
}

MTGGameZone::~MTGGameZone(){
  for (int i=0; i<nb_cards; i++) {
    delete cards[i];
  }
}

void MTGGameZone::setOwner(Player * player){
  for (int i=0; i<nb_cards; i++) {
    cards[i]->owner = player;
    cards[i]->lastController = player;
  }
  owner = player;
}

MTGCardInstance * MTGGameZone::removeCard(MTGCardInstance * card, int createCopy){
  assert(nb_cards < 10000);
  int i;
  cardsMap.erase(card);
  for (i=0; i<(nb_cards); i++) {
    if (cards[i] == card){
      card->currentZone = NULL;
      nb_cards--;
      cards.erase(cards.begin()+i);
	    MTGCardInstance * copy = card;
      //if (card->isToken) //TODO better than this ?
      //  return card;
      //card->lastController = card->controller();
      if (createCopy) {
		    copy = NEW MTGCardInstance(card->model,card->owner->game);
		    copy->previous = card;
		    copy->view = card->view;
        copy->isToken = card->isToken;
        copy->X = card->X;

        //stupid bug with tokens...
        if (card->model == card)
          copy->model =  copy;
        if (card->data == card)
          copy->data = copy;

		    card->next = copy;
	    }
      copy->previousZone = this;
      return copy;
    }
  }
  return NULL;

}

MTGCardInstance * MTGGameZone::hasCard(MTGCardInstance * card){
  if (card->currentZone == this) return card;
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

MTGCardInstance * MTGGameZone::findByName(string name){
  for (int i=0; i<(nb_cards); i++) {
    if (cards[i]->name == name){
      return cards[i];
    }
  }
  return NULL;
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
    int r = i + (WRand() % (nb_cards-i)); // Random remaining position.
    MTGCardInstance * temp = cards[i]; cards[i] = cards[r]; cards[r] = temp;
  }
}



void MTGGameZone::addCard(MTGCardInstance * card){
  if (!card) return;
  cards.push_back(card);
  nb_cards++;
  cardsMap[card] = 1;
  card->lastController = this->owner;
  card->currentZone = this;

}


void MTGGameZone::debugPrint(){
  for (int i = 0; i < nb_cards; i++)
    std::cerr << cards[i]->getName() << endl;
}





//------------------------------
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
    MTGCardInstance * card = cards[i];
    card->setUntapping();
	if (!card->basicAbilities[Constants::DOESNOTUNTAP]){
		if(card->frozen < 1) {card->attemptUntap();}
		if(card->frozen >= 1) {card->frozen = 0;}

	}
  }
}


//--------------------------
void MTGLibrary::shuffleTopToBottom(int nbcards){
  if (nbcards>nb_cards) nbcards = nb_cards;
  if (nbcards < 0) return;
  MTGCardInstance * _cards[MTG_MAX_PLAYER_CARDS];
  for (int i= nb_cards-nbcards; i<(nb_cards); i++) {
    int r = i + (WRand() % (nbcards-i)); // Random remaining position.
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


MTGGameZone * MTGGameZone::intToZone(int zoneId, MTGCardInstance * source,MTGCardInstance * target){
  Player *p, *p2;
  GameObserver * g = GameObserver::GetInstance();
  if (!source) p = g->currentlyActing();
  else p = source->controller();
  if (!target){
    p2 = p;
    target = source;//hack ?
  }
  else p2 = target->controller();

  switch(zoneId){
    case MY_GRAVEYARD: return p->game->graveyard;
    case OPPONENT_GRAVEYARD: return p->opponent()->game->graveyard;
    case TARGET_OWNER_GRAVEYARD : return target->owner->game->graveyard;
    case TARGET_CONTROLLER_GRAVEYARD:  return p2->game->graveyard;
    case GRAVEYARD : return target->owner->game->graveyard;
    case OWNER_GRAVEYARD : return target->owner->game->graveyard;

    case MY_BATTLEFIELD : return p->game->inPlay;
    case OPPONENT_BATTLEFIELD : return p->opponent()->game->inPlay;
    case TARGET_OWNER_BATTLEFIELD : return target->owner->game->inPlay;
    case TARGET_CONTROLLER_BATTLEFIELD : return p2->game->inPlay;
    case BATTLEFIELD : return p->game->inPlay;
    case OWNER_BATTLEFIELD : return target->owner->game->inPlay;

    case MY_HAND : return p->game->hand;
    case OPPONENT_HAND : return p->opponent()->game->hand;
    case TARGET_OWNER_HAND : return target->owner->game->hand;
    case TARGET_CONTROLLER_HAND : return p2->game->hand; 
    case HAND : return target->owner->game->hand;
    case OWNER_HAND : return target->owner->game->hand;

    case MY_EXILE : return p->game->removedFromGame;
    case OPPONENT_EXILE : return p->opponent()->game->removedFromGame;
    case TARGET_OWNER_EXILE :  return target->owner->game->removedFromGame;
    case TARGET_CONTROLLER_EXILE : return p2->game->removedFromGame;
    case EXILE : return target->owner->game->removedFromGame;
    case OWNER_EXILE : return target->owner->game->removedFromGame;

    case MY_LIBRARY : return p->game->library;
    case OPPONENT_LIBRARY : return p->opponent()->game->library;
    case TARGET_OWNER_LIBRARY : return target->owner->game->library;
    case TARGET_CONTROLLER_LIBRARY : return p2->game->library;
    case LIBRARY : return p->game->library;
    case OWNER_LIBRARY: return target->owner->game->library;

    case MY_STACK : return p->game->stack;
    case OPPONENT_STACK : return p->opponent()->game->stack;
    case TARGET_OWNER_STACK : return target->owner->game->stack;
    case TARGET_CONTROLLER_STACK : return p2->game->stack;
    case STACK : return p->game->stack;
    case OWNER_STACK: return target->owner->game->stack;
    default:
      return NULL;
  }
}

int MTGGameZone::zoneStringToId(string zoneName){
  const char * strings[] = {
    "mygraveyard",
    "opponentgraveyard",
    "targetownergraveyard",
    "targetcontrollergraveyard",
    "ownergraveyard",
    "graveyard",

    "myinplay",
    "opponentinplay",
    "targetownerinplay",
    "targetcontrollerinplay",
    "ownerinplay",
    "inplay",

    "mybattlefield",
    "opponentbattlefield",
    "targetownerbattlefield",
    "targetcontrollerbattlefield",
    "ownerbattlefield",
    "battlefield",

    "myhand",
    "opponenthand",
    "targetownerhand",
    "targetcontrollerhand",
    "ownerhand",
    "hand",

    "mylibrary",
    "opponentlibrary",
    "targetownerlibrary",
    "targetcontrollerlibrary",
    "ownerlibrary",
    "library",

    "myremovedfromgame",
    "opponentremovedfromgame",
    "targetownerremovedfromgame",
    "targetcontrollerremovedfromgame",
    "ownerremovedfromgame",
    "removedfromgame",

    "myexile",
    "opponentexile",
    "targetownerexile",
    "targetcontrollerexile",
    "ownerexile",
    "exile",

    "mystack",
    "opponentstack",
    "targetownerstack",
    "targetcontrollerstack",
    "ownerstack",
    "stack",

  };

  int values[] = {
    MY_GRAVEYARD,
    OPPONENT_GRAVEYARD,
    TARGET_OWNER_GRAVEYARD ,
    TARGET_CONTROLLER_GRAVEYARD,
    OWNER_GRAVEYARD ,
    GRAVEYARD,

    MY_BATTLEFIELD,
    OPPONENT_BATTLEFIELD,
    TARGET_OWNER_BATTLEFIELD ,
    TARGET_CONTROLLER_BATTLEFIELD,
    OWNER_BATTLEFIELD ,
    BATTLEFIELD,

    MY_BATTLEFIELD,
    OPPONENT_BATTLEFIELD,
    TARGET_OWNER_BATTLEFIELD ,
    TARGET_CONTROLLER_BATTLEFIELD,
    OWNER_BATTLEFIELD ,
    BATTLEFIELD,

    MY_HAND,
    OPPONENT_HAND,
    TARGET_OWNER_HAND ,
    TARGET_CONTROLLER_HAND,
    OWNER_HAND ,
    HAND,

    MY_LIBRARY,
    OPPONENT_LIBRARY,
    TARGET_OWNER_LIBRARY ,
    TARGET_CONTROLLER_LIBRARY,
    OWNER_LIBRARY ,
    LIBRARY,

    MY_EXILE,
    OPPONENT_EXILE,
    TARGET_OWNER_EXILE ,
    TARGET_CONTROLLER_EXILE,
    OWNER_EXILE ,
    EXILE,

    MY_EXILE,
    OPPONENT_EXILE,
    TARGET_OWNER_EXILE ,
    TARGET_CONTROLLER_EXILE,
    OWNER_EXILE ,
    EXILE,

    MY_STACK,
    OPPONENT_STACK,
    TARGET_OWNER_STACK ,
    TARGET_CONTROLLER_STACK,
    OWNER_STACK ,
    STACK,
  };

  int max = sizeof(values) / sizeof*(values);

  for (int i = 0; i < max; ++i){
    if(zoneName.compare(strings[i]) == 0){
      return values[i];
    }
  }
  return 0;
}

MTGGameZone * MTGGameZone::stringToZone(string zoneName, MTGCardInstance * source,MTGCardInstance * target){
  return intToZone(zoneStringToId(zoneName), source,target);
}

ostream& MTGGameZone::toString(ostream& out) const { return out << "Unknown zone"; }
ostream& MTGLibrary::toString(ostream& out) const { return out << "Library " << *owner; }
ostream& MTGGraveyard::toString(ostream& out) const { return out << "Graveyard " << *owner; }
ostream& MTGHand::toString(ostream& out) const { return out << "Hand " << *owner; }
ostream& MTGRemovedFromGame::toString(ostream& out) const { return out << "RemovedFromGame " << *owner; }
ostream& MTGStack::toString(ostream& out) const { return out << "Stack " << *owner; }
ostream& MTGInPlay::toString(ostream& out) const { return out << "InPlay " << *owner; }
ostream& operator<<(ostream& out, const MTGGameZone& z)
{
  return z.toString(out);
}
