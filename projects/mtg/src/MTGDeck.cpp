#include <string.h>
#include <algorithm>
#include <string>
#include <sstream>
#include "../include/config.h"
#include "../include/DebugRoutines.h"
#include "../include/MTGDeck.h"
#include "../include/utils.h"
#include "../include/Subtypes.h"
#include "../include/Translate.h"
#include "../include/DeckMetaData.h"
#include "../include/PriceList.h"
#include "../include/WDataSrc.h"
#include "../include/MTGPack.h"
#include "../include/utils.h"

#include <JGE.h>

#if defined (WIN32) || defined (LINUX)
#include <time.h>
#endif


static inline int getGrade(int v) {
  switch (v) {
  case 'P': case 'p': return Constants::GRADE_SUPPORTED;
  case 'R': case 'r': return Constants::GRADE_BORDERLINE;
  case 'O': case 'o': return Constants::GRADE_UNOFFICIAL;
  case 'A': case 'a': return Constants::GRADE_CRAPPY;
  case 'S': case 's': return Constants::GRADE_UNSUPPORTED;
  case 'N': case 'n': return Constants::GRADE_DANGEROUS;
  }
  return 0;
}


//MTGAllCards
int MTGAllCards::processConfLine(string &s, MTGCard *card, CardPrimitive * primitive){
  if ('#' == s[0]) return 0;
  size_t i = s.find_first_of('=');
  if (i == string::npos || 0 == i){
    DebugTrace("MTGDECK: Bad Line:\n\t" << s);
    return 0;
  }

  char* key = const_cast<char*>(s.c_str()); // I know what I'm doing, let me do it
  key[i] = 0;
  char* val = key + i + 1;

  switch (key[0]) {
    case 'a':
      if (0 == strcmp("auto", key)) {
        if (!primitive) primitive = NEW CardPrimitive();
        primitive->addMagicText(val);
      } else if (0 == strncmp("auto", key, 4)) {
        if (!primitive) primitive = NEW CardPrimitive();
        primitive->addMagicText(val, key + 4);
      } else if (0 == strcmp("alias", key)) {
        if (!primitive) primitive = NEW CardPrimitive();
        primitive->alias = atoi(val);
      } else if (0 == strcmp("abilities", key)) {
        if (!primitive) primitive = NEW CardPrimitive();
        string value = val;
        //Specific Abilities
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        while (value.size()) {
          string attribute;
          size_t found2 = value.find(',');
          if (found2 != string::npos){
            attribute = value.substr(0, found2);
            value = value.substr(found2 + 1);
          } else {
            attribute = value;
            value = "";
          }
          for (int j = Constants::NB_BASIC_ABILITIES-1; j >= 0 ; --j) {
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
			if (!primitive) primitive = NEW CardPrimitive();
			{
				string value = val;
				std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        vector<string> values = split(value, ',');
        int removeAllOthers = 1;
        for (size_t values_i = 0; values_i < values.size(); ++values_i) {
          primitive->setColor(values[values_i], removeAllOthers);
          removeAllOthers = 0;
        }
			}
			break;

    case 'g': //grade
      if (s.size() - i - 1 > 2) currentGrade = getGrade(val[2]);
      break;
    case 'k': //kicker
      if (!primitive) primitive = NEW CardPrimitive();
      if (ManaCost * cost = primitive->getManaCost())
      {
        string value = val;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        cost->kicker = ManaCost::parseManaCost(value);
      }
      break;

		case 'l': //Levelup, this is just to give Ai an idea when to stop leveling a creature.
		if(!card) card = NEW MTGCard();
			card->setLevelcap(atoi(val));
      break;

    case 'o': //othercost
      if (!primitive) primitive = NEW CardPrimitive();
      if (ManaCost * cost = primitive->getManaCost())
      {
        string value = val;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        cost->alternative = ManaCost::parseManaCost(value);
      }
      break;
    case 'b': //buyback
      if (!primitive) primitive = NEW CardPrimitive();
      if (ManaCost * cost = primitive->getManaCost())
      {
        string value = val;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        cost->BuyBack = ManaCost::parseManaCost(value);
      }
      break;
    case 'f': //flashback
      if (!primitive) primitive = NEW CardPrimitive();
      if (ManaCost * cost = primitive->getManaCost())
      {
        string value = val;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        cost->FlashBack = ManaCost::parseManaCost(value);
      }
      break;

    case 'i': //id
      if (!card) card = NEW MTGCard();
      card->setMTGId(atoi(val));
      break;

    case 'm': //mana
      if(!primitive) primitive = NEW CardPrimitive();
      {
        string value = val;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        primitive->setManaCost(value);
      }
      break;

    case 'n': //name
      if(!primitive) primitive = NEW CardPrimitive();
      if (0 == strcmp("Bloodrock Cyclops", val))
        cout << "val" << endl;
      primitive->setName(val);
      break;

    case 'p':
      if ('r' == key[1]) { // primitive
        if (!card) card = NEW MTGCard();
        map<string, CardPrimitive*>::iterator it = primitives.find(val);
        if (it != primitives.end()) card->setPrimitive(it->second);
      } else { //power
        if (!primitive) primitive = NEW CardPrimitive();
        primitive->setPower(atoi(val));
      }
      break;

    case 'r': //retrace/rarity
      if ('e' == key[1]) { //retrace
        if (!primitive) primitive = NEW CardPrimitive();
        if (ManaCost * cost = primitive->getManaCost())
        {
          string value = val;
          std::transform(value.begin(), value.end(), value.begin(), ::tolower);
          cost->Retrace = ManaCost::parseManaCost(value);
        }
      } else {//rarity
        if(!card) card = NEW MTGCard();
        card->setRarity(val[0]);
      }
      break;

    case 's': //subtype
      if(!primitive) primitive = NEW CardPrimitive();
      while (true){
        char* found = strchr(val, ' ');
        if (found) {
          string value(val, found - val);
          primitive->setSubtype(value);
          val = found + 1;
        }
        else {
          primitive->setSubtype(val);
          break;
        }
      }
      break;

    case 't':
      if (!primitive) primitive = NEW CardPrimitive();
      if (0 == strcmp("target", key)) {
        string value = val;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        primitive->spellTargetType = value;
      } else if (0 == strcmp("text", key))
        primitive->setText(val);
      else if (0 == strcmp("type", key)) {
        while (true){
          char* found = strchr(val, ' ');
          if (found) {
            string value(val, found - val);
            primitive->setType(value);
            val = found + 1;
          }
          else {
            primitive->setType(val);
            break;
          }
        }
      }
      else if (0 == strcmp("toughness", key))
        primitive->setToughness(atoi(val));
      break;

    default:
      DebugTrace("MTGDECK Parsing Error: " << s);
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
  const int set_id = set_name ? setlist.Add(set_name) : MTGSets::INTERNAL_SET;
  MTGSetInfo *si = setlist.getInfo(set_id);

  std::ifstream setFile(config_file, ios::in | ios::ate);
  if (!setFile) return total_cards;

  streampos fileSize = setFile.tellg();
  setFile.seekg(0, ios::beg);

	std::string contents;
	contents.resize((std::string::size_type)fileSize);
	setFile.read(&contents[0], fileSize);
	std::stringstream stream(contents);

  string s;

  while (true) {
    if (!std::getline(stream, s)) return total_cards;
    if (!s.size()) continue;

    if (s[s.size()-1] == '\r') s.erase(s.size()-1); // Handle DOS files
    switch (conf_read_mode) {
    case MTGAllCards::READ_ANYTHING:
      if (s[0] == '['){
        currentGrade = Constants::GRADE_SUPPORTED; // Default value
        conf_read_mode = ('m' == s[1]) ? MTGAllCards::READ_METADATA : MTGAllCards::READ_CARD; // M for metadata.
      } else {
        //Global grade for file, to avoid reading the entire file if unnnecessary
        if (s[0]=='g' && s.size() > 8) {
          int fileGrade = getGrade(s[8]);
          int maxGrade = options[Options::MAX_GRADE].number;
          if (!maxGrade) maxGrade = Constants::GRADE_BORDERLINE; //Default setting for grade is borderline?
          if (fileGrade > maxGrade) {
            return total_cards;
          }
        }
      }
      continue;
    case MTGAllCards::READ_METADATA:
      if (s[0] == '[' && s[1] == '/') conf_read_mode = MTGAllCards::READ_ANYTHING;
      else if (si) si->processConfLine(s);
      continue;
    case MTGAllCards::READ_CARD:
      if (s[0] == '[' && s[1] == '/') {
        conf_read_mode = MTGAllCards::READ_ANYTHING;
        if (tempPrimitive) tempPrimitive = addPrimitive(tempPrimitive, tempCard);
        if (tempCard) {
          if (tempPrimitive) tempCard->setPrimitive(tempPrimitive);
          addCardToCollection(tempCard, set_id);
        }
        tempCard = NULL;
        tempPrimitive = NULL;
      } else {
        processConfLine(s, tempCard, tempPrimitive);
      }
      continue;
    }
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
#if defined (_DEBUG)
    string cardName = card->data ? card->data->name : card->getImageName();
    string setName = setId != -1 ? setlist.getInfo(setId)->getName() : "";
    DebugTrace("warning, card id collision! : " << newId << " -> " << cardName << "(" << setName << ")");
#endif
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
  if (si) si->count(card);  //Count card in set info
  ++total_cards;
  return true;
}

CardPrimitive * MTGAllCards::addPrimitive(CardPrimitive * primitive, MTGCard * card){
  int maxGrade = options[Options::MAX_GRADE].number;
  if (!maxGrade) maxGrade = Constants::GRADE_BORDERLINE; //Default setting for grade is borderline?
  if (currentGrade > maxGrade) {
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
    DebugTrace("MTGDECK: primitives conflict: "<< key);
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

  int cardnb = atoi(name.c_str());
  if (cardnb){
    return getCardById(cardnb);
  }

  std::transform(name.begin(), name.end(), name.begin(),::tolower );
  int setId = -1;
  size_t found = name.find(" (");
  if (found != string::npos){
    size_t end = name.find(")");
    string setName = name.substr(found+2,end-found-2);
    trim(setName);
    name = name.substr(0,found);
    trim(name);
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
int MTGDeck::totalPrice(){
  int total = 0;
  PriceList * pricelist = NEW PriceList(RESPATH"/settings/prices.dat",GameApp::collection);
  map<int,int>::iterator it;
  for ( it=cards.begin() ; it != cards.end(); it++ ){
    int nb =  it->second;
    if (nb) total += pricelist->getPrice(it->first);
  }
  SAFE_DELETE(pricelist);
  return total;
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
          s = s.substr(0,found);
        }
        MTGCard * card = database->getCardByName(s);
        if (card){
          for (int i = 0; i < nb; i++){
            add(card);
          }
        } else {
          DebugTrace("could not find Card matching name: " << s);
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
    if (r != Constants::RARITY_T && (rarity == -1 || r==rarity) && // remove tokens
      card->setId != MTGSets::INTERNAL_SET && //remove cards that are defined in primitives. Those are workarounds (usually tokens) and should only be used internally 
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
	bool StypeIsNothing;
  size_t databaseSize = database->ids.size();
  for (size_t it = 0 ; it < databaseSize ; it++) {
    id = database->ids[it];
		StypeIsNothing = false;
		if(database->getCardById(id)->data->hasType("nothing"))
		{
			StypeIsNothing = true;
		}
		if(!StypeIsNothing == true)
		{
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

int MTGDeck::save() {
  return save( filename, false );
}

int MTGDeck::save(string destFileName, bool useExpandedDescriptions){
  string tmp = destFileName;
  tmp.append(".tmp"); //not thread safe
  std::ofstream file(tmp.c_str());
  char writer[512];
  if (file){
    DebugTrace("Saving Deck: " << meta_name << " to " << destFileName );
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

    if ( useExpandedDescriptions )
    {
      map<int,int>::iterator it;
      for (it = cards.begin(); it!=cards.end(); it++)
      {
        int nbCards = it->second;
        MTGCard *card = this->getCardById( it->first );
        if ( card == NULL )
        {
          continue;
        }
        MTGSetInfo *setInfo = setlist.getInfo( card->setId );
        string setName = setInfo->getName();
        string cardName = card->data->getName();
        file<< cardName << "\t " << "(" << setName << ") *" << nbCards << endl;
      }
    }
    else
    {
      map<int,int>::iterator it;
      for (it = cards.begin(); it!=cards.end(); it++){
        sprintf(writer,"%i\n", it->first);
        for (int j = 0; j<it->second; j++){
          file<<writer;
        }
      }
    }
    file.close();
    std::remove(destFileName.c_str());
    rename(tmp.c_str(),destFileName.c_str());
  }
  DeckMetaDataList::decksMetaData->invalidate(destFileName);
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

MTGSetInfo* MTGSets::randomSet(int blockId, int atleast){
  char * unlocked = (char *)calloc(size(),sizeof(char));
  int attempts = 50;
  //Figure out which sets are available.
  for (int i = 0; i < size(); i++){
    unlocked[i] = options[Options::optionSet(i)].number;
  }
  //No luck randomly. Now iterate from a random location.
  int a = 0, iter = 0;
  while(iter < 3){
    a = rand()%size();
    for(int i=a;i<size();i++){
      if(unlocked[i]
      && (blockId == -1 || setinfo[i]->block == blockId)
        && (atleast == -1 || setinfo[i]->totalCards() >= atleast)){
          free(unlocked);
          return setinfo[i];
      }
    }
    for(int i=0;i<a;i++){
      if(unlocked[i]
      && (blockId == -1 || setinfo[i]->block == blockId)
        && (atleast == -1 || setinfo[i]->totalCards() >= atleast)){
          free(unlocked);
          return setinfo[i];
      }
    }
    blockId = -1;
    iter++;
    if(iter == 2)
      atleast = -1;
  }
  free(unlocked);
  return NULL;
}
int blockSize(int blockId);

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
int MTGSets::getSetNum(MTGSetInfo*i){
  int it;
  for(it=0;it<size();it++){
    if(setinfo[it] == i)
      return it;
  }
  return -1;
}
int MTGSets::size(){
  return (int) setinfo.size();
}

//MTGSetInfo
MTGSetInfo::~MTGSetInfo(){
  SAFE_DELETE(mPack);
}
MTGSetInfo::MTGSetInfo(string _id) {
  string whitespaces (" \t\f\v\n\r");
  id = _id;
  block = -1;
  year = -1;

  for(int i=0;i<MTGSetInfo::MAX_COUNT;i++)
    counts[i] = 0;

  char myFilename[4096];
  sprintf(myFilename, RESPATH"/sets/%s/booster.txt", id.c_str());
  mPack = NEW MTGPack(myFilename);
  if(!mPack->isValid()){
    SAFE_DELETE(mPack);
  }
  bZipped = false;
  bThemeZipped = false;
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

string MTGSetInfo::getName(){
  if(name.size())
    return name; //Pretty name is translated when rendering.
  return id;  //Ugly name as well.
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
