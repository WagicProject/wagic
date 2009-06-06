//------------------------------------------------------
//MTGCard Class
//-------------------------------------------------
//TODO Fill BasicAbilities

#include "../include/config.h"
#include "../include/MTGCard.h"

#include "../include/TexturesCache.h"
#include "../include/Subtypes.h"

#include <string>
#include <stdlib.h>
using std::string;


const char * const MTGCard::Colors_To_Text[] = {"Artifact", "Green", "Blue", "Red", "Black", "White", "Land"};

MTGCard::MTGCard(){
  init();
  mCache = NULL;
}

MTGCard::MTGCard(TexturesCache * cache, int set_id){
  init();
  mCache = cache;
  setId = set_id;
}

const char * MTGCard::getSetName(){
  return MtgSets::SetsList->values[setId].c_str();
}

MTGCard::MTGCard(MTGCard * source){
  mCache = source->mCache;
  for (int i = 0; i< Constants::NB_BASIC_ABILITIES; i++){
    basicAbilities[i] = source->basicAbilities[i];
  }
  for (int i = 0; i< MAX_TYPES_PER_CARD; i++){
    types[i] = source->types[i];
  }
  nb_types = source->nb_types;
  for (int i = 0; i< Constants::MTG_NB_COLORS; i++){
    colors[i] = source->colors[i];
  }
  manaCost.copy(source->getManaCost());

  text = source->text;
  name = source->name;
  strcpy(image_name, source->image_name);

  rarity = source->rarity;
  power = source->power;
  toughness = source->toughness;
  mtgid = source->mtgid;
  setId = source->setId;
  formattedTextInit = 0;
  magicText = source->magicText;
  spellTargetType = source->spellTargetType;
  alias = source->alias;
}

int MTGCard::init(){
  nb_types = 0;
  for (int i = 0; i< Constants::NB_BASIC_ABILITIES; i++){
    basicAbilities[i] = 0;
  }
  for (int i = 0; i< MAX_TYPES_PER_CARD; i++){
    types[i] = 0;
  }
  for (int i = 0; i< Constants::MTG_NB_COLORS; i++){
    colors[i] = 0;
  }
  setId = 0;
  formattedTextInit = 0;
  magicText = "";
  spellTargetType = "";
  alias = 0;
  return 1;
}

JQuad * MTGCard::getQuad(int type){
  if (mCache == NULL){
    return NULL;
  }
  return mCache->getQuad(this, type);
}


JQuad * MTGCard::getThumb(){
  return getQuad(CACHE_THUMB);
}

JQuad * MTGCard::getQuad(TexturesCache * cache){

  return cache->getQuad(this);
}



int MTGCard::isACreature(){
  return (hasSubtype("creature"));
}

void MTGCard::setColor(int _color, int removeAllOthers){
  if (removeAllOthers){
    for (int i=0; i<Constants::MTG_NB_COLORS; i++){
      colors[i] = 0;
    }
  }
  colors[_color] = 1;
}

int MTGCard::getColor(){
  for (int i=0; i<Constants::MTG_NB_COLORS; i++){
    if (colors[i]){
      return i;
    }
  }
  return 0;
}


int MTGCard::hasColor(int color){
  return (colors[color]);
}

void MTGCard::setManaCost(string s){
  ManaCost::parseManaCost(s, &manaCost);
  for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; i++){
    if (manaCost.hasColor(i)){
      setColor(i);
    }
  }

}


const char * MTGCard::colorToString(){
  int color = getColor();
  if (color>=0 && color <=5){
    return Colors_To_Text[color];
  }
  return "Unknown";
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

void MTGCard::setType(const char * _type_text){
  setSubtype(_type_text);
}

void MTGCard::addType(char * _type_text){
  setSubtype(_type_text);
}

void MTGCard::setSubtype( string value){

      int id = Subtypes::subtypesList->Add(value);
      addType(id);
}

void MTGCard::addType(int id){
  types[nb_types] = id;
  nb_types++;
}


//Removes a type from the types of a given card
//If removeAll is true, removes all occurences of this type, otherwise only removes the first occurence
int MTGCard::removeType(string value, int removeAll){

  int id = Subtypes::subtypesList->Add(value);
  return removeType(id, removeAll);
}

int MTGCard::removeType(int id, int removeAll){
  int result = 0;
  for (int i = nb_types -1 ; i >=0; i--){
    if (types[i] == id){
      types[i] = types[nb_types -1];
      nb_types--;
      result++;
      if (!removeAll) return result;
    }
  }
  return result;
}


char * MTGCard::getImageName(){
  return image_name;
}


void MTGCard::setText( string value){
  text = value;
}

const char * MTGCard::getText(){
  return text.c_str();
}

void MTGCard::addMagicText(string value){
  std::transform( value.begin(), value.end(), value.begin(),::tolower );
  if (magicText.size()) magicText.append("\n");
  magicText.append(value);
}

void MTGCard::setName( string value){
  name = value;
  //This is a bug fix for plague rats and the "foreach ability"
  //Right now we add names as types, so that they get recognized
  if (value.at(value.length()-1) == 's') Subtypes::subtypesList->Add(value); 
}

const char * MTGCard::getName(){
  return name.c_str();
}


ManaCost *  MTGCard::getManaCost(){
  return &manaCost;
}



int MTGCard::hasType(int _type){
  int i;


  for (i = 0; i<nb_types; i++){
    if(types[i] == _type){
      return 1;
    }

  }
  return 0;
}

int MTGCard::hasSubtype(int _subtype){
  return(hasType(_subtype));
}

int MTGCard::hasType(const char * _type){
  int id = Subtypes::subtypesList->Add(_type);
  return(hasType(id));
}


int MTGCard::hasSubtype(const char * _subtype){
  int id = Subtypes::subtypesList->Add(_subtype);
  return(hasType(id));
}

int MTGCard::hasSubtype(string _subtype){
  int id = Subtypes::subtypesList->Add(_subtype);
  return(hasType(id));
}


int MTGCard::has(int basicAbility){
  return basicAbilities[basicAbility];
}

//---------------------------------------------
// Creature specific
//---------------------------------------------
void MTGCard::setPower(int _power){
  power = _power;
}

int MTGCard::getPower(){
  return power;
}

void MTGCard::setToughness(int _toughness){
  toughness = _toughness;
}

int MTGCard::getToughness(){
  return toughness;
}

