#ifndef _MTGGAMEZONES_H_
#define _MTGGAMEZONES_H_

#include <map>
using std::map;

#include "MTGDeck.h"
#include "MTGCardInstance.h"

#define MTG_MAX_PLAYER_CARDS 100

class MTGAllCards;
class MTGCardInstance;
class Player;

class MTGGameZone {
 protected:
  Player * owner;
 public:
  //Both cards and cardsMap contain the cards of a zone. The long term objective is to get rid of the array
  MTGCardInstance * cards[MTG_MAX_PLAYER_CARDS];
  map<MTGCardInstance *,int> cardsMap;
  int nb_cards;
  MTGGameZone();
  ~MTGGameZone();
  void shuffle();
  virtual MTGCardInstance * draw();
  void addCard(MTGCardInstance * card);
  void debugPrint();
  MTGCardInstance * removeCard(MTGCardInstance * card);
  MTGCardInstance * hasCard(MTGCardInstance * card);
  void cleanupPhase();
  int countByType(const char * value);
  int hasType(const char * value);
  void setOwner(Player * player);
  MTGCardInstance * lastCardDrawn;
};

class MTGLibrary: public MTGGameZone {
 public:
  //  MTGLibrary();
  void shuffleTopToBottom(int nbcards);
  MTGCardInstance * draw();
};

class MTGGraveyard: public MTGGameZone {
 public:
  // MTGGraveyard();
};

class MTGHand: public MTGGameZone {
 public:
};


class MTGStack: public MTGGameZone {
 public:
};

class MTGInPlay: public MTGGameZone {
 public:
  //MTGInPlay();
  void untapAll();
  MTGCardInstance * getNextAttacker(MTGCardInstance * previous);
  MTGCardInstance * getNextDefenser(MTGCardInstance * previous, MTGCardInstance * attacker);
  int nbDefensers( MTGCardInstance * attacker);
  int nbPartners(MTGCardInstance * attacker);
};


class MTGPlayerCards {
 protected:
  void init();

 public:
  MTGLibrary * library;
  MTGGraveyard * graveyard;
  MTGHand * hand;
  MTGInPlay * inPlay;
  MTGStack * stack;
  MTGAllCards * collection;

  MTGPlayerCards(MTGAllCards * _collection, int * idList, int idListSize);
  ~MTGPlayerCards();
  void initGame(int shuffle = 1, int draw = 1);
  void setOwner(Player * player);
  void discardRandom(MTGGameZone * from);
  void drawFromLibrary();
  void showHand();
  void putInGraveyard(MTGCardInstance * card);
  void putInZone(MTGCardInstance * card, MTGGameZone * from, MTGGameZone * to);
  void putInPlay(MTGCardInstance * card);
  int isInPlay(MTGCardInstance * card);

};


#endif
