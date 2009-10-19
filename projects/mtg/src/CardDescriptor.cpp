#include "../include/config.h"
#include "../include/CardDescriptor.h"
#include "../include/Subtypes.h"

CardDescriptor::CardDescriptor(): MTGCardInstance(){
  init();
  mode = CD_AND;
}

int CardDescriptor::init(){
  int result = MTGCardInstance::init();
  attacker = 0;
  defenser = NULL;
  banding = NULL;
  return result;
}

void CardDescriptor::unsecureSetTapped(int i){
  tapped = i;
}

void CardDescriptor::setNegativeSubtype( string value){
  int id = Subtypes::subtypesList->find(value);
  addType(-id);
} 

MTGCardInstance * CardDescriptor::match_or(MTGCardInstance * card){
  int found = 1;
  for (int i = 0; i< nb_types; i++){
    found = 0;
    if (types[i] >= 0){

      if (card->hasSubtype(types[i]) || (Subtypes::subtypesList->find(card->getLCName(),false) == types[i])){
        found = 1;
        break;
      }
    }else{
      if (!card->hasSubtype(-types[i]) && (Subtypes::subtypesList->find(card->getLCName(), false) != -types[i])){
        found = 1;
        break;
      }
    }
  }
  if (!found) return NULL;

  for (int i = 0; i< Constants::MTG_NB_COLORS; i++){
    if (colors[i] == 1){
      found = 0;
      if(card->hasColor(i)){
        found = 1;
        break;
      }
    }
    else if (colors[i] == -1){
      found = 0;
      if(!card->hasColor(i)){
        found = 1;
        break;
      }
    }
  }
  if (!found) return NULL;
  return card;
}

MTGCardInstance * CardDescriptor::match_and(MTGCardInstance * card){
  MTGCardInstance * match = card;
  for (int i = 0; i< nb_types; i++){
    if (types[i] >= 0){
      if (!card->hasSubtype(types[i]) && !(Subtypes::subtypesList->find(card->getLCName(),false) == types[i])){
        match = NULL;
      }
    }else{
      if(card->hasSubtype(-types[i]) || (Subtypes::subtypesList->find(card->getLCName(),false) == -types[i])){
        match = NULL;
      }
    }
  }
  for (int i = 0; i< Constants::MTG_NB_COLORS; i++){
    if ((colors[i] == 1 && !card->hasColor(i))||(colors[i] == -1 && card->hasColor(i))){
      match = NULL;
    }
  }
  return match;
}

MTGCardInstance * CardDescriptor::match(MTGCardInstance * card){

  MTGCardInstance * match = card;
  if (mode == CD_AND){
    match = match_and(card);
  }else{
    match=match_or(card);
  }




  //Abilities
  for(map<int,int>::const_iterator it = basicAbilities.begin(); it != basicAbilities.end(); ++it){
    int j = it->first;
    if ((basicAbilities[j] == 1 && !card->basicAbilities[j]) || (basicAbilities[j] == -1 && card->basicAbilities[j])){
      match = NULL;
    }
  }


  if ((tapped == -1 && card->isTapped()) || (tapped == 1 && !card->isTapped())){
    match = NULL;
  }



  if (attacker == 1){
    if (defenser == &AnyCard){
      if (!card->attacker && !card->defenser) match = NULL;
    }else{
      if (!card->attacker) match = NULL;
    }
  }else if (attacker == -1){
    if (defenser == &NoCard){
      if (card->attacker || card->defenser) match = NULL;
    }else{
      if (card->attacker) match = NULL;
    }
  }else{
    if (defenser == &NoCard){
      if (card->defenser) match = NULL;
    }else if (defenser == &AnyCard){
      if (!card->defenser) match = NULL;
    }else{
      // we don't care about the attack/blocker state
    }
  }


  return match;
}

MTGCardInstance * CardDescriptor::match(MTGGameZone * zone){
  return (nextmatch(zone, NULL));
}

MTGCardInstance * CardDescriptor::nextmatch(MTGGameZone * zone, MTGCardInstance * previous){
  int found = 0;
  if (NULL == previous) found = 1;
  for(int i=0; i < zone->nb_cards; i++){
    if(found && match(zone->cards[i])){
      return zone->cards[i];
    }
    if (zone->cards[i] == previous){
      found = 1;
    }
  }
  return NULL;
}
