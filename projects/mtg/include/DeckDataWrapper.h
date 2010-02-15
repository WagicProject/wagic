#ifndef _DECKDATAWRAPPER_H_
#define _DECKDATAWRAPPER_H_

#include "../include/MTGDefinitions.h"
#include "../include/MTGCard.h"
#include "../include/CardPrimitive.h"
#include "../include/WDataSrc.h"
#include <map>
#include <string>
using std::map;
using std::string;

class MTGDeck;


class Cmp1 { // compares cards by their name
 public:
  bool operator()(MTGCard * card1, MTGCard * card2) const {
    if (!card2) return true;
    if (!card1) return false;
    int result = card1->data->name.compare(card2->data->name);
    if (!result) return card1->getMTGId() < card2->getMTGId();
    return ( result < 0);
  }
};

class DeckDataWrapper: public WSrcDeck {
 public:
  MTGDeck * parent;
  int counts[Constants::MTG_NB_COLORS];
  unsigned short minCards; //Fewest number of copies of any given card in the deck.

  DeckDataWrapper(MTGDeck * deck);

  int Add(MTGCard * c, int quantity=1);
  int Remove(MTGCard * c, int quantity=1, bool erase=false);
  int Add(MTGDeck * deck);
  int getCount(int color=-1);
  void updateCounts();
  bool next() {currentPos++; return true;};
  bool prev() {currentPos--; return true;};
  void save();
};

#endif
