#include "../include/config.h"
#include "../include/MTGDeck.h"
#include "../include/utils.h"
#include "../include/Subtypes.h"
#include "../include/Translate.h"
#include <algorithm>
#include <string>
#include <sstream>
using std::string;

#include <JGE.h>

#if defined (WIN32) || defined (LINUX)
#include <time.h>
#endif

//MTGAllCards
int MTGAllCards::processConfLine(string s, MTGCard *card, CardPrimitive * primitive){
  if (s.size() && s[0] == '#') return 0;
  size_t i = s.find_first_of("=");
  if (i == string::npos){
#if defined (_DEBUG)
    char buffer[4096];
    sprintf(buffer, "MTGDECK: Bad Line:\n    %s\n", s.c_str());
    OutputDebugString(buffer);
#endif
    return 0;
  }
  string key = s.substr(0,i);
  string value = s.substr(i+1);
  if (!key.size()) return 0;

  switch(key[0]) {
    case 'a':
      if(key.compare( "auto")==0){
        if(!primitive) primitive = NEW CardPrimitive();
        primitive->addMagicText(value);
      } else if(key.find("auto") == 0){
        if(!primitive) primitive = NEW CardPrimitive();
        primitive->addMagicText(value,key.substr(4));
      } else if(key.compare( "alias")==0){
        if(!primitive) primitive = NEW CardPrimitive();
        primitive->alias=atoi(value.c_str());
      } else if (key.compare("abilities")==0){
        if(!primitive) primitive = NEW CardPrimitive();
        //Specific Abilities
        std::transform( value.begin(), value.end(), value.begin(),::tolower );
        while (value.size()){
          string attribute;
          size_t found2 = value.find(",");
          if (found2 != string::npos){
            attribute = value.substr(0,found2);
            value = value.substr(found2+1);
          }else{
            attribute = value;
            value = "";
          }
          for (int j = Constants::NB_BASIC_ABILITIES-1; j >=0 ; j--){
            size_t found = attribute.find(Constants::MTGBasicAbilities[j]);
            if (found != string::npos){
	            primitive->basicAbilities[j] = 1;
              break;
            }
          }
        }
      }
      break;

    case 'c': //color
      if(!primitive) primitive = NEW CardPrimitive();
      std::transform( value.begin(), value.end(), value.begin(),::tolower );
      primitive->setColor(value,1);
      break;

    case 'g': //grade
      {
        switch(value[0]) {
        case 'S':
        case 's':
          currentGrade = Constants::GRADE_SUPPORTED;
          break;
        case 'B':
        case 'b':
          currentGrade = Constants::GRADE_BORDERLINE;
          break;
        case 'C':
        case 'c':
          currentGrade = Constants::GRADE_CRAPPY;
          break;
        case 'U':
        case 'u':
          currentGrade = Constants::GRADE_UNSUPPORTED;
          break;
        case 'D':
        case 'd':
          currentGrade = Constants::GRADE_DANGEROUS;
          break;
        }
      }
      break;
    case 'k': //kicker
      if(!primitive) primitive = NEW CardPrimitive();
      std::transform( value.begin(), value.end(), value.begin(),::tolower );
      if (ManaCost * cost = primitive->getManaCost()){
        cost->kicker = ManaCost::parseManaCost(value);
      }
      break;

    case 'i': //id
      if (!card) card = NEW MTGCard();
      card->setMTGId(atoi(value.c_str()));
      break;

    case 'm': //mana
      if(!primitive) primitive = NEW CardPrimitive();
      std::transform( value.begin(), value.end(), value.begin(),::tolower );
      primitive->setManaCost(value);
      break;

    case 'n': //name      
      if(!primitive) primitive = NEW CardPrimitive();
      primitive->setName(value);
      break;

    case 'p':
      if(key.compare("primitive")==0){
        if(!card) card = NEW MTGCard();
        map<string,CardPrimitive *>::iterator it = primitives.find(value);
        if (it != primitives.end()) {
          card->setPrimitive(it->second);
        }
      } else { //power
        if(!primitive) primitive = NEW CardPrimitive();
        primitive->setPower (atoi(value.c_str()));
      }
      break;

    case 'r': //rarity
      if (!card) card = NEW MTGCard();
      card->setRarity (value.c_str()[0]);
      break;

    case 's': //subtype
      if(!primitive) primitive = NEW CardPrimitive();
      while (value.size()){
        size_t found = value.find(" ");
        if (found != string::npos){
          primitive->setSubtype(value.substr(0,found));
          value = value.substr(found+1);
        }else{
          primitive->setSubtype(value);
          value = "";
        }
      }
      break;

    case 't':
      if(key.compare( "target")==0){
        if(!primitive) primitive = NEW CardPrimitive();
        std::transform( value.begin(), value.end(), value.begin(),::tolower );
        primitive->spellTargetType=value;
      }else if(key.compare( "text")==0){
        if(!primitive) primitive = NEW CardPrimitive();
        primitive->setText(value);
      }else if (key.compare("type")==0) {
        if(!primitive) primitive = NEW CardPrimitive();
        while (value.size()){
          size_t found = value.find(" ");
          if (found != string::npos){
            primitive->setType(value.substr(0,found).c_str());
            value = value.substr(found+1);
          }else{
            primitive->setType(value.c_str());
            value = "";
          }
        }
      }else if(key.compare("toughness")==0){
        if(!primitive) primitive = NEW CardPrimitive();
        primitive->setToughness(atoi(value.c_str()));
      }
      break;
      
    default:
      string error = "MTGDECK Parsing Error:" + s + "\n";
      OutputDebugString(error.c_str());
      break;
  }

  tempPrimitive = primitive;
  tempCard = card;

  return i;

}

void MTGAllCards::initCounters(){
  for (int i=0; i < Constants::MTG_NB_COLORS; i++){
    colorsCount[i] = NULL;
  }
}

void MTGAllCards::init(){
  tempCard = NULL;
  tempPrimitive = NULL;
  total_cards = 0;
  initCounters();
}



int MTGAllCards::load(const char * config_file, const char * set_name,int autoload){
  conf_read_mode = 0;
  int set_id = 0;

  if (set_name) set_id = setlist.Add(set_name);

  std::ifstream setFile(config_file);

  if (setFile){
    while(readConfLine(setFile, set_id)){};
  }

  return total_cards;
}

MTGAllCards::MTGAllCards(){
  init();
}

MTGAllCards::~MTGAllCards(){
  //Why don't we call destroyAllCards from here ???
}

void MTGAllCards::destroyAllCards(){
  

  for (map<int,MTGCard *>::iterator it = collection.begin(); it!=collection.end(); it++) delete(it->second);
  collection.clear();
  ids.clear();

  for (map<string,CardPrimitive *>::iterator it = primitives.begin(); it!=primitives.end(); it++) delete(it->second);
  primitives.clear();

}

MTGAllCards::MTGAllCards(const char * config_file, const char * set_name){
  init();
  load(config_file,set_name, 0);
}



int MTGAllCards::randomCardId(){
  int id = (rand() % ids.size());
  return ids[id];
}



int MTGAllCards::countBySet(int setId){
  int result = 0;
  map<int,MTGCard *>::iterator it;

  for (it = collection.begin(); it!=collection.end(); it++){
    MTGCard * c = it->second;
    if( c->setId == setId){
      result++;
    }
  }
  return result;
}

//TODO more efficient way ?
int MTGAllCards::countByType(const char * _type){
  int result = 0;
  map<int,MTGCard *>::iterator it;
  for (it = collection.begin(); it!=collection.end(); it++){
    MTGCard * c = it->second;
    if(c->data->hasType(_type)){
      result++;
    }
  }
  return result;
}


int MTGAllCards::countByColor(int color){
  if (colorsCount[color] == 0){
    for (int i=0; i< Constants::MTG_NB_COLORS; i++){
      colorsCount[i] = 0;
    }
    map<int,MTGCard *>::iterator it;
    for (it = collection.begin(); it!=collection.end(); it++){
      MTGCard * c = it->second;
      int j = c->data->getColor();

      colorsCount[j]++;
    }
  }
  return colorsCount[color];
}

int MTGAllCards::totalCards(){
  return (total_cards);
}

bool MTGAllCards::addCardToCollection(MTGCard * card, int setId){
  card->setId = setId;
  int newId = card->getId();
  if (collection.find(newId) != collection.end()){
    char outBuf[4096];
    sprintf(outBuf,"warning, card id collision! : %i\n", newId);
    OutputDebugString (outBuf);
    SAFE_DELETE(card);
    return false;
  }

  //Don't add cards that don't have a primitive
  if (!card->data){
    SAFE_DELETE(card);
    return false;
  }
  ids.push_back(newId);

  collection[newId] = card; //Push card into collection.
  MTGSetInfo * si = setlist.getInfo(setId);
  if(si)
    si->count(card);  //Count card in set info

  total_cards++;

  return true;

}

CardPrimitive * MTGAllCards::addPrimitive(CardPrimitive * primitive, MTGCard * card){
  int maxGrade = options[Options::MAX_GRADE].number;
  if (!maxGrade) maxGrade = Constants::GRADE_BORDERLINE;
  if (currentGrade >maxGrade) {
    SAFE_DELETE(primitive);
    return NULL;
  }
  string key;
  if (card) {
    std::stringstream ss;
    ss << card->getId();
    ss >> key;
  }
  else key = primitive->name;
  if (primitives.find(key) != primitives.end()){
    //ERROR
    //Todo move the deletion somewhere else ?
#ifdef _DEBUG
    OutputDebugString("MTGDECK: primitives conflict:");
    OutputDebugString(key.c_str());
    OutputDebugString("\n");
#endif
    SAFE_DELETE(primitive);
    return NULL;
  }
  //translate cards text
  Translator * t = Translator::GetInstance();
  map<string,string>::iterator it = t->tempValues.find(primitive->name);
  if (it != t->tempValues.end()) {
    primitive->setText(it->second);
  }

  //Legacy:
  //For the Deck editor, we need Lands and Artifact to be colors...
  if (primitive->hasType(Subtypes::TYPE_LAND)) primitive->setColor(Constants::MTG_COLOR_LAND);
  if (primitive->hasType(Subtypes::TYPE_ARTIFACT)) primitive->setColor(Constants::MTG_COLOR_ARTIFACT);

  primitives[key] = primitive;
  return primitive;
}

int MTGAllCards::readConfLine(std::ifstream &file, int set_id){
  MTGSetInfo * si = setlist.getInfo(set_id);
  string s;
  int result = 1;
  if(!std::getline(file,s)) return 0;
  if (!s.size()) return -1;
  if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
    switch(conf_read_mode) {
    case MTGAllCards::READ_ANYTHING:
      if (s[0] == '['){
        currentGrade = Constants::GRADE_SUPPORTED; //default value
        if(s[1] == 'm'){ //M for metadata.
          conf_read_mode = MTGAllCards::READ_METADATA;
        }
        else{
          conf_read_mode = MTGAllCards::READ_CARD;
        }
      }
      break;    
    case MTGAllCards::READ_METADATA:
      if (s[0] == '[' && s[1] == '/'){
        conf_read_mode = MTGAllCards::READ_ANYTHING;
        break;
      }
      if(si)
        si->processConfLine(s);
      break;
    case MTGAllCards::READ_CARD:
      if (s[0] == '[' && s[1] == '/'){
	      conf_read_mode = MTGAllCards::READ_ANYTHING;
        if (tempPrimitive) tempPrimitive = addPrimitive (tempPrimitive,tempCard);
        if (tempCard){
          if (tempPrimitive) tempCard->setPrimitive(tempPrimitive);
          addCardToCollection(tempCard, set_id);
        }
        tempCard = NULL;
        tempPrimitive = NULL;

      }else{
	      processConfLine(s, tempCard, tempPrimitive);
      }
      break;
    default:
      break;
    }

 

  return result;

}


MTGCard * MTGAllCards::getCardById(int id){
	map<int, MTGCard *>::iterator it = collection.find(id);
	if ( it != collection.end()){
    return (it->second);
  }
  return 0;
}

MTGCard * MTGAllCards::_(int index){
  if (index >= total_cards) return NULL;
  return getCardById(ids[index]);
}

MTGCard * MTGAllCards::getCardByName(string name){
  if (!name.size()) return NULL;
  if (name[0] == '#') return NULL;
  std::transform(name.begin(), name.end(), name.begin(),::tolower );
  int setId = -1;
  size_t found = name.find(" (");
  if (found != string::npos){
    size_t end = name.find(")");
    string setName = name.substr(found+2,end-found-2);
    name = name.substr(0,found);
    setId = setlist[setName];
  }
  map<int,MTGCard *>::iterator it;
  for (it = collection.begin(); it!=collection.end(); it++){
    MTGCard * c = it->second;
    if (setId!=-1 && setId != c->setId) continue;
    string cardName = c->data->name;
    std::transform(cardName.begin(), cardName.end(), cardName.begin(),::tolower );
    if (cardName.compare(name) == 0) return c;
    
  }
  return NULL;
}

//MTGDeck
MTGDeck::MTGDeck(MTGAllCards * _allcards){
  total_cards = 0;
  database = _allcards;
  filename ="";
  meta_name = "";
}

MTGDeck::MTGDeck(const char * config_file, MTGAllCards * _allcards, int meta_only){
  total_cards = 0;
  database = _allcards;
  filename = config_file;
  size_t slash = filename.find_last_of("/");
  size_t dot = filename.find(".");
  meta_name = filename.substr(slash+1,dot-slash-1);
  std::ifstream file(config_file);
  std::string s;

  if(file){
    while(std::getline(file,s)){
      if (!s.size()) continue;
      if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
      if (s[0] == '#'){
        size_t found = s.find("NAME:");
        if ( found != string::npos){
          meta_name = s.substr(found+5);
          continue;
        }
        found = s.find("DESC:");
        if ( found != string::npos){
          if (meta_desc.size()) meta_desc.append("\n");
          meta_desc.append(s.substr(found+5));
          continue;
        }
        continue;
      }
      if (meta_only) break;
      int cardnb = atoi(s.c_str());
      if (cardnb){
        add(cardnb);
      }else{
        int nb = 1;
        size_t found = s.find(" *");
        if (found != string::npos){
          nb = atoi(s.substr(found+2).c_str());
          s=s.substr(0,found);
          OutputDebugString(s.c_str());
        }
        MTGCard * card = database->getCardByName(s);
        if (card){
          for (int i = 0; i < nb; i++){
            add(card);
          }
        }
      }
    }
    file.close();
  }else{
    //TODO Error management
  }


}

int MTGDeck::totalCards(){
  return total_cards;
}

MTGCard * MTGDeck::getCardById(int mtgId){
  return database->getCardById(mtgId);
}


int MTGDeck::addRandomCards(int howmany, int * setIds, int nbSets, int rarity, const char * _subtype, int * colors, int nbcolors){
  if(howmany <= 0)
    return 1;

  int unallowedColors[Constants::MTG_NB_COLORS+1];
  for (int i=0; i < Constants::MTG_NB_COLORS; ++i){
    if (nbcolors) unallowedColors[i] = 1;
    else unallowedColors[i] = 0;
  }
  for (int i=0; i < nbcolors; ++i){
    unallowedColors[colors[i]] = 0;
  }

  int collectionTotal = database->totalCards();
  if (!collectionTotal) return 0;

  char subtype[4096];
  if (_subtype)
    sprintf(subtype, "%s", _subtype);


  vector<int> subcollection;
  int subtotal = 0;
  for (int i = 0; i < collectionTotal; i++){
    MTGCard * card = database->_(i);
    int r = card->getRarity();
    if (r != Constants::RARITY_T && (rarity == -1 || r==rarity) &&
	(!_subtype || card->data->hasSubtype(subtype))
	){
      int ok = 0;

      if (!nbSets) ok = 1;
      for (int j=0; j < nbSets; ++j){
        if (card->setId == setIds[j]){
          ok = 1;
          break;
        }
      }

      if (ok){
        for (int j=0; j < Constants::MTG_NB_COLORS; ++j){
          if (unallowedColors[j] && card->data->hasColor(j)){
            ok = 0;
            break;
          }
        }
      }

      if (ok){
        subcollection.push_back(card->getId());
        subtotal++;
      }
    }
  }
  if (subtotal == 0){
    if (rarity == Constants::RARITY_M) return addRandomCards(howmany, setIds, nbSets, Constants::RARITY_R,  _subtype, colors, nbcolors);
    return 0;
  }
  for (int i = 0; i < howmany; i++){
    int id = (rand() % subtotal);
    add(subcollection[id]);
  }
  return 1;
}

int MTGDeck::add(MTGDeck * deck){
  map<int,int>::iterator it;
  for (it = deck->cards.begin(); it!=deck->cards.end(); it++){
    for (int i = 0; i < it->second; i++){
      add(it->first);
    }
  }
  return deck->totalCards();
}

int MTGDeck::add(int cardid){
  if (!database->getCardById(cardid)) return 0;
  if(cards.find(cardid) == cards.end()){
    cards[cardid] = 1;
  }else{
    cards[cardid]++;
  }
  ++total_cards;
  //initCounters();
  return total_cards;
}

int MTGDeck::add(MTGCard * card){
  if (!card) return 0;
  return (add(card->getId()));
}

int MTGDeck::complete() {
  /* (PSY) adds cards to the deck/collection. Makes sure that the deck
     or collection has at least 4 of every implemented card. Does not
     change the number of cards of which already 4 or more are present. */
  int id, n;
  size_t databaseSize = database->ids.size();
  for (size_t it = 0 ; it < databaseSize ; it++) {
    id = database->ids[it];
    if(cards.find(id) == cards.end()){
      cards[id] = 4;
      total_cards += 4;
    } else {
      n = cards[id];
      if (n < 4) {
        total_cards += 4 - n;
        cards[id] = 4;
      }
    }    
  }
  return 1;
}

int MTGDeck::removeAll(){
  total_cards = 0;
  cards.clear();
  //initCounters();
  return 1;
}

int MTGDeck::remove(int cardid){
  if(cards.find(cardid) == cards.end() || cards[cardid] == 0) return 0;
  cards[cardid]--;
  total_cards--;
  //initCounters();
  return 1;
}


int MTGDeck::remove(MTGCard * card){
  if (!card) return 0;
  return (remove(card->getId()));
}

int MTGDeck::save(){
  std::ofstream file(filename.c_str());
  char writer[10];
  if (file){
#if defined (WIN32) || defined (LINUX)
    OutputDebugString("saving");
#endif
    if (meta_name.size()){
      file << "#NAME:" << meta_name << '\n';
    }
    
    if (meta_desc.size()){
      size_t found = 0;
      string desc= meta_desc;
      found = desc.find_first_of("\n");
      while(found != string::npos){
        file << "#DESC:" << desc.substr(0,found+1);
        desc=desc.substr(found+1);
        found = desc.find_first_of("\n");
      }
      file << "#DESC:" << desc << "\n";
    }
    map<int,int>::iterator it;
    for (it = cards.begin(); it!=cards.end(); it++){
      sprintf(writer,"%i\n", it->first);
      for (int j = 0; j<it->second; j++){
        file<<writer;
      }
    }
    file.close();
  }

  return 1;
}

//MTGSets
MTGSets setlist; //Our global.

MTGSets::MTGSets(){
}

MTGSets::~MTGSets(){
  for (size_t i = 0; i < setinfo.size(); ++i){
    delete (setinfo[i]);
  }
}

MTGSetInfo* MTGSets::getInfo(int setID){
  if(setID < 0 || setID >= (int) setinfo.size())
    return NULL;

  return setinfo[setID];
}

int MTGSets::Add(const char * name){
  int setid = findSet(name);
  if(setid != -1)
    return setid;

  MTGSetInfo* s = NEW MTGSetInfo(name);
  setinfo.push_back(s);
  setid = (int) setinfo.size();
  
  return setid - 1;
}

int MTGSets::findSet(string name){
  std::transform(name.begin(), name.end(), name.begin(),::tolower );

  for (int i = 0; i < (int) setinfo.size(); i++){
    MTGSetInfo* s = setinfo[i];
    if(!s) continue;
    string set = s->id;
    std::transform(set.begin(), set.end(), set.begin(),::tolower);
    if (set.compare(name) == 0) return i;
  }
  return -1;
}

int MTGSets::findBlock(string s){
  if(!s.size())
    return -1;

  string comp = s;
  std::transform(comp.begin(), comp.end(), comp.begin(),::tolower);
  for(int i=0;i<(int)blocks.size();i++){
    string b = blocks[i];
    std::transform(b.begin(), b.end(), b.begin(),::tolower);
    if(b.compare(comp) == 0) return i;
  }

  blocks.push_back(s); 
  return ((int) blocks.size()) -1;
}

int MTGSets::operator[](string id){
  return findSet(id);
}
string MTGSets::operator[](int id){
  if(id < 0 || id >= (int) setinfo.size())
    return "";

  MTGSetInfo * si = setinfo[id];
  if(!si)
    return "";

  return si->id;
}

int MTGSets::size(){
  return (int) setinfo.size();
}


//MTGSetInfo
MTGSetInfo::MTGSetInfo(string _id) {
  string whitespaces (" \t\f\v\n\r");
  id = _id;
  block = -1;
  year = -1;

  for(int i=0;i<MTGSetInfo::MAX_COUNT;i++)
    counts[i] = 0;

  booster[MTGSetInfo::LAND] = 1;
  booster[MTGSetInfo::COMMON] = 10;
  booster[MTGSetInfo::UNCOMMON] = 3;
  booster[MTGSetInfo::RARE] = 1;

  //Load metadata. (FIXME - Removed for release 0.1.0, will be fully implemented next release)
}

void MTGSetInfo::count(MTGCard*c){
  if(!c)
    return;

  switch(c->getRarity()){
    case Constants::RARITY_M:
      counts[MTGSetInfo::MYTHIC]++;
      break;
    case Constants::RARITY_R:
      counts[MTGSetInfo::RARE]++;
      break;
    case Constants::RARITY_U:
      counts[MTGSetInfo::UNCOMMON]++;
      break;
    case Constants::RARITY_C:
      counts[MTGSetInfo::COMMON]++;
      break;
    default:
    case Constants::RARITY_L:
      counts[MTGSetInfo::LAND]++;
      break;
  }
  
  counts[MTGSetInfo::TOTAL_CARDS]++;
}

int MTGSetInfo::totalCards(){
  return counts[MTGSetInfo::TOTAL_CARDS];
}

int MTGSetInfo::boosterSize(){
  int size = 0;

  for(int i = 0; i<MTGSetInfo::MAX_RARITY;i++)
    size += booster[i];

  return size;
}

int MTGSetInfo::boosterCost(){
  int price = 0;
  for(int i = 0; i<MTGSetInfo::MAX_RARITY;i++){
      if(i == MTGSetInfo::LAND)
        price += booster[i] * Constants::PRICE_XL;
      else if(i == MTGSetInfo::COMMON)
        price += booster[i] * Constants::PRICE_XC;
      else if(i == MTGSetInfo::UNCOMMON)
        price += booster[i] * Constants::PRICE_XU;
      else
        price += booster[i] * Constants::PRICE_XR;
  }

  return price;
}

string MTGSetInfo::getName(){
  if(name.size()) 
    return _(name); //Pretty name is translated.
  return id;  //Ugly name is not.
}
string MTGSetInfo::getBlock(){
  if(block < 0 || block >= (int) setlist.blocks.size())
    return "None";

  return setlist.blocks[block];
}


void MTGSetInfo::processConfLine(string line){
  size_t i = line.find_first_of("=");
  if (i == string::npos)
    return;

  string key = line.substr(0,i);
  std::transform(key.begin(),key.end(),key.begin(),::tolower);
  string value = line.substr(i+1);

  if(key.compare("name") == 0)
    name = value;
  else if(key.compare("author") == 0)
    author = value;
  else if(key.compare("block") == 0)
    block = setlist.findBlock(value.c_str());
  else if(key.compare("year") == 0)
    year = atoi(value.c_str());
 }
