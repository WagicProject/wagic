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

class DeckDataWrapper: public WSrcDeck {
 public:
  MTGDeck * parent;
  DeckDataWrapper(MTGDeck * deck);
  bool next() {currentPos++; return true;};
  bool prev() {currentPos--; return true;};
  void save();
  void save(string filepath, bool useExpandedCardNames);
};

#endif
