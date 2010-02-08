#include "../include/config.h"
#include "../include/DeckDataWrapper.h"
#include "../include/MTGDeck.h"
#include "../include/PriceList.h"
#include "../include/WDataSrc.h"

DeckDataWrapper::DeckDataWrapper(MTGDeck * deck){
  parent = deck;  
  for(int c=0;c<Constants::MTG_NB_COLORS;c++)
    counts[c] = 0;
  Add(deck);
}

int DeckDataWrapper::Add(MTGDeck * deck){
  if(loadMatches(deck))
    return 1;
  return 0;
}
int DeckDataWrapper::Remove(MTGCard * c, int quantity,bool erase){
  if(WSrcDeck::Remove(c,quantity,erase)){
    for(int i=0;i<Constants::MTG_NB_COLORS;i++){
      if(c->data->hasColor(i))
        counts[i]-=quantity;
    }
    return 1;
  }
  return 0;
}
int DeckDataWrapper::Add(MTGCard * c, int quantity){
  if(WSrcDeck::Add(c,quantity)){
    for(int i=0;i<Constants::MTG_NB_COLORS;i++){
      if(c->data && c->data->hasColor(i))
        counts[i]+=quantity;
    }
    return 1;
  }
  return 0;
}
int DeckDataWrapper::getCount(int color){
  if(color < 0 || color >=Constants::MTG_NB_COLORS)
    return Size(true);
  return counts[color];
}
void DeckDataWrapper::updateCounts(){
  map<int,int>::iterator it;
  for(int c=0;c<Constants::MTG_NB_COLORS;c++)
    counts[c] = 0;

  for(int i=0;i<Size(true);i++){
    for(int c=0;c<Constants::MTG_NB_COLORS;c++){
      MTGCard * card = getCard(c,true);
      if(card->data->hasColor(c)){
        it = copies.find(card->getMTGId());
        if(it != copies.end())
          counts[c]+=it->second;
      }
    }
  }
}
void DeckDataWrapper::save(){
  Rebuild(parent);
  parent->save();
}


DeckDataWrapper::~DeckDataWrapper(){
  SAFE_DELETE(parent);
}