#include "../include/debug.h"
#include "../include/GroupOfCards.h"


GroupOfCards::GroupOfCards(GameObserver * _game){
  game = _game;
}

GroupOfCreatures::GroupOfCreatures(GameObserver * _game, int _filter, int _filterValue):GroupOfCards(_game){
  filter = _filter;
  filterValue = _filterValue;
}

int GroupOfCreatures::includes(MTGCardInstance * card){
  if (!game->isACreature(card)){
    return 0;
  }
  switch (filter){
  case FILTER_SUBTYPE:
    return card->hasSubtype( filterValue);
    break;
  default:
    return 0;
  }
}


GroupOfSpecificCards::GroupOfSpecificCards(GameObserver * _game, MTGCardInstance * _cards[], int _nb_cards):GroupOfCards(_game){
  int i;
  nb_cards = _nb_cards;
  for (i=0; i<nb_cards; i++){
    cards[i] = _cards[i];
  }
}

GroupOfSpecificCards::GroupOfSpecificCards(GameObserver * _game, MTGCardInstance * card):GroupOfCards(_game){
  nb_cards = 1;
  cards[0] = card;
}

int GroupOfSpecificCards::includes(MTGCardInstance * card){
  int i;
  for (i=0; i<nb_cards; i++){
    if (cards[i] == card){
      return 1;
    }
  }
  return 0;
}
