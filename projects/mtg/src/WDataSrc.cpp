#include "../include/config.h"
#include "../include/OptionItem.h"
#include "../include/PlayerData.h"
#include "../include/Translate.h"
#include "../include/PriceList.h"
#include "../include/Subtypes.h"
#include <algorithm>

//WSyncable
bool WSyncable::Hook(WSyncable* s){
  if(hooked)
    return false;
  hooked = s;
  return true;
}
int WSyncable::getPos(){
  if(hooked)
    return hooked->getPos()+currentPos;
  return currentPos;
}
bool WSyncable::next(){
  if(hooked)
    return hooked->next();
  ++currentPos;
  return true;
}
bool WSyncable::prev(){
  if(hooked)
    return hooked->prev();
  --currentPos;
  return true;
}

//WSrcImage
JQuad * WSrcImage::getImage( int offset){
  return resources.RetrieveTempQuad(filename);
}

WSrcImage::WSrcImage(string s){
  filename = s;
}
//WSrcCards
WSrcCards::WSrcCards(float delay){
  mDelay = delay;
  mLastInput = 0;
  currentPos = 0;
  filtersRoot=NULL;
}
JQuad * WSrcCards::getImage( int offset){
#if defined WIN32 || defined LINUX //Loading delay only on PSP.
#else
  if(mDelay && mLastInput < mDelay){
      return resources.RetrieveCard(getCard(offset),RETRIEVE_EXISTING);
  }
#endif
  return resources.RetrieveCard(getCard(offset));
}
JQuad * WSrcCards::getThumb( int offset){
  return resources.RetrieveCard(getCard(offset),RETRIEVE_THUMB);
}
WSrcCards::~WSrcCards(){
  clearFilters();
  cards.clear();
}
void WSrcCards::bakeFilters(){
  vector<MTGCard*> temp;
  
  setOffset(0);
  for(int t=0;t<Size();t++){
    temp.push_back(getCard(t));
  }
  setOffset(0);
  cards.clear();
  cards.swap(temp);
  clearFilters();
  return;
}

bool WSrcCards::matchesFilters(MTGCard * c){
  if(!c)  return false;
  if(!filtersRoot) return true;
  return filtersRoot->isMatch(c);
}
int WSrcCards::Size(bool all){
  if(!all && filtersRoot)
    return (int) validated.size();
  return (int) cards.size();
}
MTGCard * WSrcCards::getCard(int offset, bool ignore){
  int oldpos;
  int size = (int) cards.size();
  MTGCard * c = NULL;
  if(!ignore && filtersRoot)
    size = (int) validated.size();
    
  if(!size)
   return NULL;

  oldpos = currentPos;
  if(offset != 0)
    currentPos += offset;
  while(currentPos < 0)
    currentPos = size+currentPos;
  currentPos = currentPos % size;

  if(!ignore && filtersRoot)
    c = cards[validated[currentPos]];
  else
    c = cards[currentPos];
  currentPos = oldpos;
  return c;
}
int WSrcCards::loadMatches(MTGAllCards* ac){
  map<int, MTGCard *>::iterator it;
  int count = 0;
  if(!ac)
    return count;

  for(it=ac->collection.begin();it!=ac->collection.end();it++){
    if(it->second && (matchesFilters(it->second))){
      cards.push_back(it->second);
      count++;
    }
  }
  validate();
  updateCounts();
  return count;
}
int WSrcCards::loadMatches(MTGDeck * deck){
  map<int, int>::iterator it;
  int count = 0;
  if(!deck)
    return count;
  for(it=deck->cards.begin();it!=deck->cards.end();it++){
    MTGCard * c = deck->getCardById(it->first);
    if(c && (matchesFilters(c))){
      cards.push_back(c);
      count++;
    }
  }
  validate();
  updateCounts();
  return count;
}
int WSrcCards::loadMatches(WSrcCards* src, bool all){
  int count = 0;
  if(!src)
    return count;

  MTGCard * c = NULL;
  size_t t = 0;
  int oldp = src->getOffset();
  src->setOffset(0);
  for(int t=0;t<src->Size(all);t++){
    c = src->getCard(t,all);
    if(matchesFilters(c)){
      cards.push_back(c);
      count++;    
    }
  }
  src->setOffset(oldp);
  validate();
  updateCounts();
  return count;  
}
int WSrcCards::addRandomCards(MTGDeck * i, int howmany){
  if(!cards.size() || (filtersRoot && !validated.size()))
    return howmany;
  for(int x=0;x<howmany;x++){
    if(validated.size()){
      size_t pos = rand() % validated.size();
      MTGCard * c = cards[validated[pos]];
      i->add(c);
    }
    else{
      size_t pos = rand() % cards.size();
      i->add(cards[pos]);
    }
  }
  return 0;
}

int WSrcCards::addToDeck(MTGDeck * i, int num){
  int oldpos = getOffset();
  int added = 0;
  int cycles = 0;

  if(!i){
    if(num < 0)
      return 0;
    return num;
  }

  setOffset(0);
  if(num < 0){ //Add it all;
    MTGCard * c;
    for(;;){
      c = getCard();
      if(!c || !next())
        break;
      i->add(c);
    }
  }else while(added < num){
    MTGCard * c = getCard();
    if(!next() || !c){
      if(++cycles == WSrcCards::MAX_CYCLES){ //Abort the search, too many cycles.
        setOffset(oldpos);
        return num - added;
      }
      setOffset(0);
      continue;
    }else{
      i->add(c);
      added++;
    }
  }
  setOffset(oldpos);
  return 0;
}
bool WSrcCards::next(){
  int oldpos = currentPos;
  bool bMatch = true;
  int size = (int)cards.size();
  if(filtersRoot)
    size = (int)validated.size();
  if(currentPos+1 >= size)
    return false;
  currentPos++;
  return true;
}

bool WSrcCards::prev(){
  int oldpos = currentPos;
  bool bMatch = true;
  if(currentPos == 0)
    return false;

  currentPos--;
  return true;
}

bool WSrcCards::setOffset(int pos){
  if(pos < 0 || pos >= (int) cards.size())
    return false;

  currentPos = pos;
  if(!matchesFilters(cards[currentPos]))
    return next();

  return true;
}
void WSrcCards::Shuffle(){
  vector<MTGCard*> a;
  MTGCard * temp;
  vector<MTGCard*>::iterator k;

  while(cards.size()){
    k = cards.begin();
    k += rand() % cards.size();
    temp = *k;
    cards.erase(k);
    a.push_back(temp);
  }
#if defined WIN32 || defined LINUX //PC performs a double shuffle for less streaking.
  while(a.size()){
    k = a.begin();
    k += rand() % a.size();
    temp = *k;
    a.erase(k);
    cards.push_back(temp);
  }
#else //PSP does a straight swap for speed.
  cards.swap(a);
#endif
  validate();
}
void WSrcCards::validate(){
  validated.clear();
  if(!filtersRoot) return;
  for(size_t t=0;t<cards.size();t++){
    if(matchesFilters(cards[t]))
      validated.push_back(t);
  }
}
bool WSrcCards::thisCard(int mtgid){
  for(size_t t=0;t<cards.size();t++){
    if(cards[t] && cards[t]->getId() == mtgid){
      currentPos = (int) t;
      return matchesFilters(cards[currentPos]);
    }
  }
  return false;
}
bool WSrcCards::isEmptySet(WCardFilter * f){
  size_t max = cards.size();
  if(validated.size()) max = validated.size();
  if(!f) return (max > 0);
  for(size_t t=0;t<max;t++){
    if(validated.size()){
      if(f->isMatch(cards[validated[t]]))
        return false;
    }else if(f->isMatch(cards[t]))
        return false;
  }
  return true;
}

void WSrcCards::addFilter(WCardFilter * f) {
  if(filtersRoot == NULL)
    filtersRoot = f;
  else
    filtersRoot = NEW WCFilterAND(f,filtersRoot);
  validate();
  updateCounts();
  currentPos = 0;
}
float WSrcCards::filterFee(){
  if(filtersRoot) 
    return filtersRoot->filterFee(); 
  return 0;
};
void WSrcCards::clearFilters() {
  SAFE_DELETE(filtersRoot);
  validated.clear();
}
WCardFilter* WSrcCards::unhookFilters(){
  WCardFilter* temp = filtersRoot;
  filtersRoot = NULL;
  clearFilters();
  return temp;
}
void WSrcCards::Sort(int method){
  switch(method){
    case WSrcCards::SORT_COLLECTOR:
      std::sort(cards.begin(),cards.end(),WCSortCollector());
      break;
    case WSrcCards::SORT_RARITY:
      std::sort(cards.begin(),cards.end(),WCSortRarity());
      break;
    case WSrcCards::SORT_ALPHA:
    default:
      std::sort(cards.begin(),cards.end(),WCSortAlpha());
      break;
  }
  validate();
}
//WSrcUnlockedCards
WSrcUnlockedCards::WSrcUnlockedCards(float delay): WSrcCards(delay){
  MTGAllCards * ac = GameApp::collection;
  map<int, MTGCard*>::iterator it;

  char * unlocked = NULL; 
  unlocked = (char *)calloc(setlist.size(),sizeof(char));
  //Figure out which sets are available.
  for (int i = 0; i < setlist.size(); i++){    
    unlocked[i] = options[Options::optionSet(i)].number;
  }

  for(it=ac->collection.begin();it!=ac->collection.end();it++){
    if(it->second && unlocked[it->second->setId])
      cards.push_back(it->second);
  }
  if(unlocked){
    free(unlocked);
    unlocked = NULL;
  }

  
  if(cards.size()){
    Shuffle();
    currentPos = 0;
  }
}

//WSrcDeck
int WSrcDeck::loadMatches(MTGDeck * deck){
  map<int, int>::iterator it;
  int count = 0;
  if(!deck)
    return count;
  for(it=deck->cards.begin();it!=deck->cards.end();it++){
    MTGCard * c = deck->getCardById(it->first);
    if(c && matchesFilters(c)){
      Add(c,it->second);
      count++;
    }
  }
  validate();
  updateCounts();
  return count;
}
void WSrcDeck::updateCounts(){
  vector<MTGCard*>::iterator it;
  map<int,int>::iterator ccount;
  clearCounts();
  for(it=cards.begin();it!=cards.end();it++){
    ccount = copies.find((*it)->getMTGId());
    if(ccount == copies.end()) continue;
    addCount((*it),ccount->second);
  }
}
void WSrcDeck::clearCounts(){
  counts[UNFILTERED_MIN_COPIES] = -1;
  counts[UNFILTERED_MAX_COPIES] = 0;
  for(int i=0;i<MAX_COUNTS;i++)
    counts[i] = 0;
}
void WSrcDeck::addCount(MTGCard * c, int qty){
  if(!c || !c->data) return;
  if(matchesFilters(c)){
    counts[FILTERED_COPIES]+=qty;
    counts[FILTERED_UNIQUE]++;
  }
  counts[UNFILTERED_COPIES] += qty;
  counts[UNFILTERED_UNIQUE]++;
  for(int i=Constants::MTG_COLOR_GREEN;i<=Constants::MTG_COLOR_WHITE;i++)
    if (c->data->hasColor(i)) counts[i]+= qty;
  if(counts[UNFILTERED_MIN_COPIES] < 0 || qty < counts[UNFILTERED_MIN_COPIES]) counts[UNFILTERED_MIN_COPIES] = qty;
  if(qty > counts[UNFILTERED_MAX_COPIES]) counts[UNFILTERED_MAX_COPIES] = qty;

}
int WSrcDeck::Add(MTGCard * c, int quantity){
  if(!c)
    return 0;
  if(copies.find(c->getMTGId()) == copies.end())
    cards.push_back(c); //FIXME Make sure these two stay synced.
  copies[c->getMTGId()] += quantity;
  addCount(c,quantity);
  validate();
  return 1;
}

int WSrcDeck::Remove(MTGCard * c, int quantity, bool erase){
  if(!c) return 0;
  map<int,int>::iterator it = copies.find(c->getMTGId());
  if(it == copies.end()) return 0;
  int amt = it->second;
  if(amt < quantity)
    return 0;
  amt -= quantity;
  it->second = amt;
  if(erase && amt == 0){ 
    copies.erase(it);
    vector<MTGCard*>::iterator i = find(cards.begin(),cards.end(),c);
    if(i != cards.end())
      cards.erase(i);
  }
  validate();
  addCount(c,-quantity);
  return 1;
}

void WSrcDeck::Rebuild(MTGDeck * d){
  d->removeAll();
  map<int,int>::iterator it;
  for ( it=copies.begin() ; it != copies.end(); it++ ){
    for (int i = 0; i < it->second; i++){
      d->add(it->first);
    }
  }
}

int WSrcDeck::count(MTGCard * c){
  if(!c)
    return counts[UNFILTERED_COPIES];
  if(copies.find(c->getMTGId()) == copies.end())
    return 0;
  return copies[c->getMTGId()];
}

int WSrcDeck::countByName(MTGCard * card, bool editions){
  string name = card->data->getLCName();
  int total = 0;
  vector<MTGCard*>::iterator it;
  for(it = cards.begin();it!=cards.end();it++){
    if(*it && (*it)->data->getLCName() == name){
      if(editions)
        total++;
      else{
        map<int,int>::iterator mi = copies.find((*it)->getMTGId());
        if(mi != copies.end())
          total += mi->second;
      }
    }
  }
 return total;
}
int WSrcDeck::getCount(int count){
  if(count < 0 || count >=MAX_COUNTS)
    return counts[UNFILTERED_COPIES];
  return counts[count];
}
int WSrcDeck::totalPrice(){
  int total = 0;
  PriceList * pricelist = NEW PriceList(RESPATH"/settings/prices.dat",GameApp::collection);
  map<int,int>::iterator it;
  for ( it=copies.begin() ; it != copies.end(); it++ ){
      int nb =  it->second;
      if (nb) total += pricelist->getPrice(it->first);
  }
  SAFE_DELETE(pricelist);
  return total;
}

//Sorting methods:
int WCSortRarity::rareToInt(char r){
  switch(r){
    default: case Constants::RARITY_T: return 0;
    case Constants::RARITY_L: return 1;
    case Constants::RARITY_C: return 2;
    case Constants::RARITY_U: return 3;
    case Constants::RARITY_R: return 4;
    case Constants::RARITY_M: return 5;
  }
}
bool WCSortRarity::operator()(const MTGCard*l, const MTGCard*r){
  if(!l || !r || !l->data || !r->data)
    return false;
  return (rareToInt(l->getRarity()) < rareToInt(r->getRarity()));
}
bool WCSortAlpha::operator()(const MTGCard*l, const MTGCard*r){
  if(!l || !r || !l->data || !r->data)
    return false;
  string ln = l->data->getLCName();
  string rn = r->data->getLCName();
  if(ln == rn)
    return l->getMTGId() < r->getMTGId();
  return (ln < rn);
}
bool WCSortCollector::operator()(const MTGCard*l, const MTGCard*r){
  if(!l || !r || !l->data || !r->data)
    return false;

  if(l->setId != r->setId)
    return (l->setId < r->setId);

  int lc, rc;
  lc = l->data->countColors(); rc = r->data->countColors();
  if(lc == 0) lc = 999;
  if(rc == 0) rc = 999;

  int isW = (int)l->data->hasColor(Constants::MTG_COLOR_WHITE) - (int) r->data->hasColor(Constants::MTG_COLOR_WHITE);
  int isU = (int)l->data->hasColor(Constants::MTG_COLOR_BLUE) - (int) r->data->hasColor(Constants::MTG_COLOR_BLUE);
  int isB = (int)l->data->hasColor(Constants::MTG_COLOR_BLACK) - (int) r->data->hasColor(Constants::MTG_COLOR_BLACK);
  int isR = (int)l->data->hasColor(Constants::MTG_COLOR_RED) - (int) r->data->hasColor(Constants::MTG_COLOR_RED);
  int isG = (int)l->data->hasColor(Constants::MTG_COLOR_GREEN) - (int) r->data->hasColor(Constants::MTG_COLOR_GREEN);
  int isArt = (int)l->data->hasType(Subtypes::TYPE_ARTIFACT) - (int) r->data->hasType(Subtypes::TYPE_ARTIFACT);
  int isLand = (int)l->data->hasType(Subtypes::TYPE_LAND) - (int) r->data->hasType(Subtypes::TYPE_LAND);

  //Nested if hell. TODO: Farm these out to their own objects as a user-defined filter/sort system.
  if(!isLand){
    int isBasic = (int)l->data->hasType("Basic") - (int) r->data->hasType("Basic");
    if(!isBasic){
      if(!isArt){
        if(lc == rc){
          if(!isG){
            if(!isR){
              if(!isB){
                if(!isU){
                  if(!isW){
                    string ln = l->data->getLCName();
                    string rn = r->data->getLCName();
                    if(ln.substr(0,4) == "the ")
                      ln = ln.substr(4);
                    if(rn.substr(0,4) == "the ")
                      rn = rn.substr(4);
                    return (ln < rn);
                  }
                  return (isW < 0);
                }
                return (isU < 0);
              }
              return (isB < 0);
            }
            return (isR < 0);
          }
          return (isG < 0);
        }
        return (lc < rc);
      }
      return (isArt < 0);
    }
    else return(isBasic < 0);
  }
  return (isLand < 0);
}