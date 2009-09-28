#ifndef _MTGGAMEZONES_H_
#define _MTGGAMEZONES_H_

#include <map>
using std::map;

#include "MTGDeck.h"
#include "MTGCardInstance.h"

#define MTG_MAX_PLAYER_CARDS 100

class MTGAllCards;
class MTGDeck;
class MTGCardInstance;
class Player;

class MTGGameZone {
 protected:

 public:

   enum{
    MY_GRAVEYARD = 11,
    OPPONENT_GRAVEYARD = 12,
    TARGET_OWNER_GRAVEYARD = 13,
    TARGET_CONTROLLER_GRAVEYARD = 14,
    GRAVEYARD = 15,
    OWNER_GRAVEYARD = 16,

    MY_BATTLEFIELD = 21,
    OPPONENT_BATTLEFIELD = 22,
    TARGET_OWNER_BATTLEFIELD = 23,
    TARGET_CONTROLLER_BATTLEFIELD = 24,
    BATTLEFIELD = 25,
    OWNER_BATTLEFIELD = 26,

    MY_HAND = 31,
    OPPONENT_HAND = 32,
    TARGET_OWNER_HAND = 33,
    TARGET_CONTROLLER_HAND = 34,
    HAND = 35,
    OWNER_HAND = 36,

    MY_EXILE = 41,
    OPPONENT_EXILE = 42,
    TARGET_OWNER_EXILE = 43,
    TARGET_CONTROLLER_EXILE = 44,
    EXILE = 45,
    OWNER_EXILE = 46,

    MY_LIBRARY = 51,
    OPPONENT_LIBRARY = 52,
    TARGET_OWNER_LIBRARY = 53,
    TARGET_CONTROLLER_LIBRARY = 54,
    LIBRARY = 55,
    OWNER_LIBRARY = 56,

    MY_STACK = 61,
    OPPONENT_STACK = 62,
    TARGET_OWNER_STACK = 63,
    TARGET_CONTROLLER_STACK = 64,
    STACK = 65,
    OWNER_STACK = 66,

   };

   Player * owner;
   //Both cards and cardsMap contain the cards of a zone. The long term objective is to get rid of the array
   vector<MTGCardInstance *> cards; //[MTG_MAX_PLAYER_CARDS];
   map<MTGCardInstance *,int> cardsMap;
   int nb_cards;
   MTGGameZone();
   ~MTGGameZone();
   void shuffle();
   virtual MTGCardInstance * draw();
   void addCard(MTGCardInstance * card);
   void debugPrint();
   MTGCardInstance * removeCard(MTGCardInstance * card, int createCopy = 1);
   MTGCardInstance * hasCard(MTGCardInstance * card);
   void cleanupPhase();
   int countByType(const char * value);
   MTGCardInstance * findByName(string name);
   int hasType(const char * value);
   void setOwner(Player * player);
   MTGCardInstance * lastCardDrawn;
   static MTGGameZone * stringToZone(string zoneName, MTGCardInstance * source, MTGCardInstance * target);
   static int zoneStringToId(string zoneName);
   static MTGGameZone *intToZone(int zoneId, MTGCardInstance * source = NULL,MTGCardInstance * target = NULL);
   bool needShuffle;
   virtual const char * getName(){return "zone";}; 
   virtual ostream& toString(ostream&) const;
};

class MTGLibrary: public MTGGameZone {
 public:
  //  MTGLibrary();
  void shuffleTopToBottom(int nbcards);
  MTGCardInstance * draw();
  virtual ostream& toString(ostream&) const;
  const char * getName(){return "library";}
};

class MTGGraveyard: public MTGGameZone {
 public:
  // MTGGraveyard();
  virtual ostream& toString(ostream&) const;
  const char * getName(){return "graveyard";}
};

class MTGHand: public MTGGameZone {
 public:
  virtual ostream& toString(ostream&) const;
  const char * getName(){return "hand";}
};

class MTGRemovedFromGame: public MTGGameZone {
 public:
  virtual ostream& toString(ostream&) const;
  const char * getName(){return "exile";}
};

class MTGStack: public MTGGameZone {
 public:
  virtual ostream& toString(ostream&) const;
  const char * getName(){return "stack";}
};

class MTGInPlay: public MTGGameZone {
 public:
  //MTGInPlay();
  void untapAll();
  MTGCardInstance * getNextAttacker(MTGCardInstance * previous);
  MTGCardInstance * getNextDefenser(MTGCardInstance * previous, MTGCardInstance * attacker);
  int nbDefensers( MTGCardInstance * attacker);
  int nbPartners(MTGCardInstance * attacker);
  virtual ostream& toString(ostream&) const;
  const char * getName(){return "battlefield";}
};


class MTGPlayerCards {
 protected:
  void init();

 public:
  MTGLibrary * library;
  MTGGraveyard * graveyard;
  MTGHand * hand;
  MTGInPlay * inPlay;
  MTGInPlay * battlefield; //alias to inPlay

  MTGStack * stack;
  MTGRemovedFromGame * removedFromGame;
  MTGRemovedFromGame * exile; //alias to removedFromZone
  MTGGameZone * garbage;
  MTGGameZone * temp;

  MTGAllCards * collection;

  MTGPlayerCards(MTGAllCards * _collection, int * idList, int idListSize);
  MTGPlayerCards(MTGAllCards * _collection, MTGDeck * deck);
  ~MTGPlayerCards();
  void initGame(int shuffle = 1, int draw = 1);
  void setOwner(Player * player);
  void discardRandom(MTGGameZone * from);
  void drawFromLibrary();
  void showHand();
  MTGCardInstance * putInGraveyard(MTGCardInstance * card);
  MTGCardInstance * putInZone(MTGCardInstance * card, MTGGameZone * from, MTGGameZone * to);
  int isInPlay(MTGCardInstance * card);
};

ostream& operator<<(ostream&, const MTGGameZone&);

#endif
