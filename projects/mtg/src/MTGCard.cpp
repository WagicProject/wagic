//------------------------------------------------------
//MTGCard Class
//-------------------------------------------------
//TODO Fill BasicAbilities

#include <string>
#include <stdlib.h>

#include "../include/MTGDeck.h"
#include "../include/config.h"
#include "../include/MTGCard.h"
#include "../include/CardPrimitive.h"
#include "../include/Subtypes.h"
#include "../include/Translate.h"

using std::string;

MTGCard::MTGCard(){
  init();
}

MTGCard::MTGCard(int set_id){
  init();
  setId = set_id;
}
MTGCard::MTGCard(MTGCard * source){

  strcpy(image_name, source->image_name);
  rarity = source->rarity;
  mtgid = source->mtgid;
  setId = source->setId;
  data = source->data;

}

int MTGCard::init(){
  setId = 0;
  mtgid = 0;
  data = NULL;
  rarity = Constants::RARITY_C;
  return 1;
}


void MTGCard::setMTGId(int id){
  mtgid = id;
  if (id < 0){
    sprintf(image_name, "%dt.jpg", -mtgid);
  }else{
    sprintf(image_name, "%d.jpg", mtgid);
  }
}

int MTGCard::getMTGId(){
  return mtgid;
}
int MTGCard::getId(){
  return mtgid;
}

char MTGCard::getRarity(){
  return rarity;
}

void MTGCard::setRarity(char _rarity){
  rarity = _rarity;
}

char * MTGCard::getImageName(){
  return image_name;
}

void MTGCard::setPrimitive(CardPrimitive * cp){
  data = cp;
}