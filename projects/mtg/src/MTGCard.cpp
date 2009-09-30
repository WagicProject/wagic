//------------------------------------------------------
//MTGCard Class
//-------------------------------------------------
//TODO Fill BasicAbilities

#include <string>
#include <stdlib.h>

#include "../include/MTGDeck.h"
#include "../include/config.h"
#include "../include/MTGCard.h"
#include "../include/Subtypes.h"
#include "../include/Translate.h"

using std::string;


const char * const MTGCard::Colors_To_Text[] = {"Artifact", "Green", "Blue", "Red", "Black", "White", "Land"};

MTGCard::MTGCard(){
  init();
}

MTGCard::MTGCard(int set_id){
  init();
  setId = set_id;
}

const char * MTGCard::getSetName(){
  return MtgSets::SetsList->values[setId].c_str();
}

MTGCard::MTGCard(MTGCard * source){
  for(map<int,int>::const_iterator it = source->basicAbilities.begin(); it != source->basicAbilities.end(); ++it){
    basicAbilities[it->first] = source->basicAbilities[it->first];
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
  magicText = source->magicText;
  spellTargetType = source->spellTargetType;
  alias = source->alias;
}

int MTGCard::init(){
  nb_types = 0;
  basicAbilities.clear();

  for (int i = 0; i< MAX_TYPES_PER_CARD; i++){
    types[i] = 0;
  }
  for (int i = 0; i< Constants::MTG_NB_COLORS; i++){
    colors[i] = 0;
  }
  setId = 0;
  mtgid = 0;
  magicText = "";
  spellTargetType = "";
  alias = 0;
  return 1;
}

const vector<string>& MTGCard::formattedText()
{
  if (ftdText.empty())
    {
      std::string s = _(text);
      std::string::size_type found = s.find_first_of("{}");
      while (found!=string::npos)
	{
	  s[found] = '/';
	  found = s.find_first_of("{}", found + 1);
	}
      std::string::size_type len = 30;
      while (s.length() > 0)
	{
	  std::string::size_type cut = s.find_first_of("., \t)", 0);
	  if (cut >= len || cut == string::npos)
	    {
	      ftdText.push_back(s.substr(0,len));
	      if (s.length() > len)
		s = s.substr(len, s.length() - len);
	      else
		s = "";
	    }
	  else
	    {
	      std::string::size_type newcut = cut;
	      while (newcut < len && newcut != string::npos)
		{
		  cut = newcut;
		  newcut = s.find_first_of("., \t)", newcut + 1);
		}
	      ftdText.push_back(s.substr(0,cut+1));
	      if (s.length() > cut+1)
		s = s.substr(cut+1,s.length() - cut - 1);
	      else
		s = "";
	    }
	}
    }
  return ftdText;
}


bool MTGCard::isCreature(){
  return hasSubtype("creature");
}
bool MTGCard::isLand(){
  return hasSubtype("land");
}
bool MTGCard::isSpell(){
  return (!isCreature() && !isLand());
}

void MTGCard::setColor(string _color, int removeAllOthers){
  if(_color.compare("blue")==0) return setColor(Constants::MTG_COLOR_BLUE,removeAllOthers);
  if(_color.compare("red")==0) return setColor(Constants::MTG_COLOR_RED,removeAllOthers);
  if(_color.compare("green")==0) return setColor(Constants::MTG_COLOR_GREEN,removeAllOthers);
  if(_color.compare("black")==0) return setColor(Constants::MTG_COLOR_BLACK,removeAllOthers);
  if(_color.compare("white")==0) return setColor(Constants::MTG_COLOR_WHITE,removeAllOthers);
}

void MTGCard::setColor(int _color, int removeAllOthers){
  if (removeAllOthers){
    for (int i=0; i<Constants::MTG_NB_COLORS; i++){
      colors[i] = 0;
    }
  }
  colors[_color] = 1;
}

void MTGCard::removeColor(int _color){
  colors[_color] = 0;
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

const string MTGCard::getName() const{
  return name;
}


ManaCost* MTGCard::getManaCost(){
  return &manaCost;
}



bool MTGCard::hasType(int _type){
  for (int i = 0; i<nb_types; i++)
    if (types[i] == _type)
      return true;
  return false;
}

bool MTGCard::hasSubtype(int _subtype){
  return hasType(_subtype);
}

bool MTGCard::hasType(const char * _type){
  int id = Subtypes::subtypesList->Add(_type);
  return hasType(id);
}


bool MTGCard::hasSubtype(const char * _subtype){
  int id = Subtypes::subtypesList->Add(_subtype);
  return hasType(id);
}

bool MTGCard::hasSubtype(string _subtype){
  int id = Subtypes::subtypesList->Add(_subtype);
  return hasType(id);
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
