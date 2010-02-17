#include "../include/config.h"
#include "../include/DeckDataWrapper.h"
#include "../include/MTGDeck.h"
#include "../include/PriceList.h"
#include "../include/WDataSrc.h"

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
