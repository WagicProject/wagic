#include "PrecompiledHeader.h"

#include "DeckDataWrapper.h"
#include "MTGDeck.h"
#include "PriceList.h"
#include "WDataSrc.h"

DeckDataWrapper::DeckDataWrapper(MTGDeck * deck){
  parent = deck;  
  loadMatches(deck);
}

void DeckDataWrapper::save(){
  if(parent){
    Rebuild(parent);
    parent->save();
  }
}

void DeckDataWrapper::save(string filepath, bool useExpandedCardNames, string &deckTitle, string &deckDesc){
  if(parent){
    Rebuild(parent);
    parent->save(filepath, useExpandedCardNames, deckTitle, deckDesc);
  }
}
