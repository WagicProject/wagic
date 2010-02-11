#include <JGui.h>
#include <vector>
#include <string>
#include <dirent.h>
#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/Translate.h"
#include "../include/WDataSrc.h"
#include "../include/WFilter.h"
#include "../include/DeckDataWrapper.h"
#include "../include/MTGPack.h"
#include "../../../JGE/src/TinyXML/tinyxml.h"

int MTGPackEntryRandom::addCard(WSrcCards *pool, MTGDeck *to){
  int fails = 0;
  if(!pool) return 1;
  WCFilterFactory * ff = WCFilterFactory::GetInstance();
  WCardFilter * oldf = pool->unhookFilters();
  pool->addFilter(ff->Construct(filter));
  fails = pool->addRandomCards(to,copies);
  pool->clearFilters();
  pool->addFilter(oldf);
  return fails;
}
int MTGPackEntrySpecific::addCard(WSrcCards *pool, MTGDeck *to){
  int fails = 0;
  //Ignores pool entirely.
  MTGAllCards * ac = GameApp::collection;
 
  if(!card) return copies;
  for(int i=0;i<copies;i++)
    to->add(card);
  return 0;
}

int MTGPackSlot::add(WSrcCards * ocean, MTGDeck *to, int carryover){
  if(!entries.size()) return copies;
  int fails = 0;
  int amt = copies + carryover;
  WSrcCards * myPool = MTGPack::getPool(pool);
  if(!myPool) myPool = ocean;
  for(int i=0;i<amt;i++){
    size_t pos = rand() % entries.size();
#if defined WIN32 || defined LINUX //First try other entries in slot
    while(pos < entries.size() && entries[pos]->addCard(myPool,to)) 
      pos++; 
    if(pos == entries.size()) fails++; 
#else //Fall straight through to next slot
  fails = entries[pos]->addCard(myPool,to); 
#endif
  }
  return fails;
}

WSrcCards * MTGPack::getPool(string poolstr){
  WSrcCards * mySrc = NULL;
  size_t s = poolstr.find("all");
  WCFilterFactory * ff = WCFilterFactory::GetInstance();

  if(s == string::npos){ //Default to just unlocked cards
    mySrc = NEW WSrcUnlockedCards();
    s = poolstr.find("unlocked");
    string sub = poolstr;
    if(s != string::npos) sub = poolstr.substr(s+8);
    if(sub.size()){
      mySrc->addFilter(ff->Construct(sub));
      mySrc->bakeFilters();
    }
  }
  else{ //Use everything.
    mySrc = NEW WSrcCards();
    string sub = poolstr.substr(s+3);
    if(sub.size()){
      mySrc->addFilter(ff->Construct(sub));
      mySrc->loadMatches(GameApp::collection);
      mySrc->bakeFilters();
    }else
      mySrc->loadMatches(GameApp::collection);
  }
  mySrc->Shuffle();
  return mySrc;
}

void MTGPackSlot::addEntry(MTGPackEntry*item){
  if(item)
    entries.push_back(item);
}
int MTGPack::assemblePack(MTGDeck *to){
  int carryover = 0;
  WSrcCards * p = getPool(pool);
  if(!p)
    return -1;
  
  for(size_t i=0;i<slots.size();i++){
    carryover = slots[i]->add(p,to,carryover);
  }
  SAFE_DELETE(p);
  return carryover;
}
void MTGPack::countCards(){
  minCards = 0;
  maxCards = 0;
  for(size_t i=0;i<slots.size();i++){
    MTGPackSlot * ps = slots[i];
    int top = 0;
    int bot = 999999999;
    for(size_t y=0;y<ps->entries.size();y++){
      int test = ps->entries[y]->copies * ps->copies;
      if(test > top) top = test;
      if(test < bot) bot = test;
    }
    maxCards += top;
    minCards += bot;
  }
}
void MTGPack::load(string filename){
  //TODO Placeholder until XML format available.
  TiXmlDocument packfile(filename.c_str());
  if(!packfile.LoadFile())
    return;
  TiXmlHandle hDoc(&packfile);
  TiXmlElement * pPack;
  pPack = hDoc.FirstChildElement().Element();
  if(!pPack ){ 
    
    return;
  }
  //root should be "pack"
  string tag = pPack->Value();
  std::transform(tag.begin(),tag.end(),tag.begin(),::tolower);
  if(tag != "pack")
    return;
  //After validating, handle actual loading.
  TiXmlElement * pSlot;
  const char * holder = NULL;
  holder = pPack->Attribute("price");
  if(holder) price = atoi(holder); else price = Constants::PRICE_BOOSTER;
  holder = pPack->Attribute("pool");
  if(holder) pool = holder; else pool = "";
  holder = pPack->Attribute("type");
  if(holder) type = holder; else type = "Booster";
  holder = pPack->Attribute("name");
  if(holder) name = holder; else name = "Special";
  holder = pPack->Attribute("requires");
  if(holder) check = holder; 
  holder = pPack->Attribute("sort");
  if(holder) sort = holder; else sort = "";
  std::transform(sort.begin(),sort.end(),sort.begin(),::tolower);

  for (pSlot=pPack->FirstChildElement();pSlot!=NULL;pSlot=pSlot->NextSiblingElement()){
    TiXmlElement * pEntry;
    //Load slot.
    tag = pSlot->Value();
    std::transform(tag.begin(),tag.end(),tag.begin(),::tolower);
    if(tag != "slot") continue;
    MTGPackSlot * s = NEW MTGPackSlot(); 
    slots.push_back(s); 
    holder = pSlot->Attribute("copies");
    if(holder) s->copies = atoi(holder);
    else       s->copies = 1;

    for(pEntry = pSlot->FirstChildElement();pEntry!=NULL;pEntry=pEntry->NextSiblingElement()){
      tag = pEntry->Value();
      std::transform(tag.begin(),tag.end(),tag.begin(),::tolower);
      if(tag == "card"){ //Load specific card
        MTGPackEntrySpecific * es = NEW MTGPackEntrySpecific();
        holder = pEntry->Attribute("copies");
        if(holder) es->copies = atoi(holder);
        else       es->copies = 1;
        es->card = GameApp::collection->getCardByName(pEntry->Value());
        s->addEntry(es);
      }else if(tag == "random_card"){ //Load random card
        MTGPackEntryRandom * er = NEW MTGPackEntryRandom();
        holder = pEntry->Attribute("copies");
        if(holder) er->copies = atoi(holder);
        else       er->copies = 1;
        const char * text = pEntry->GetText();
        if(text) er->filter = text;
        s->addEntry(er);
      }else if(tag == "nothing"){
        MTGPackEntryNothing * nt = NEW MTGPackEntryNothing();
        s->addEntry(nt);
      }
    }
  }  
  bValid = true;
  countCards();
  return;
}
MTGPackSlot::~MTGPackSlot(){
  for(size_t t=0;t<entries.size();t++){
    SAFE_DELETE(entries[t]);
  }
  entries.clear();
}
MTGPack::~MTGPack(){
  for(size_t t=0;t<slots.size();t++){
    SAFE_DELETE(slots[t]);
  }
  slots.clear();
}
MTGPacks::~MTGPacks(){
  for(size_t t=0;t<packs.size();t++){
    SAFE_DELETE(packs[t]);
  }
  packs.clear();
}
MTGPack * MTGPacks::randomPack(int key){
  if(!key) key = rand();
  size_t s = packs.size();
  if(!s) return NULL;
  return packs[key%s];
}
void MTGPacks::loadAll(){
  DIR *mDip = opendir(RESPATH"/packs/");  
  struct dirent *mDit;
  if(!mDip) return;

  while ((mDit = readdir(mDip))){
    char myFilename[4096];
    sprintf(myFilename, RESPATH"/packs/%s", mDit->d_name);
    if(myFilename[0] == '.') continue;
    MTGPack * p = NEW MTGPack(myFilename);
    if(!p->isValid()){
      SAFE_DELETE(p);
      continue;
    }
    packs.push_back(p);
  }
  closedir(mDip);
}
string MTGPack::getName(){
  string n = _(name);
  string t = _(type);
  char buf[1024];
  if(minCards != maxCards)
    sprintf(buf,"%s %s (%i-%i cards)",n.c_str(),t.c_str(),minCards,maxCards);
  else
    sprintf(buf,"%s %s (%i cards)",n.c_str(),t.c_str(),maxCards);
  return buf;
}

bool MTGPack::meetsRequirements(){
  bool unlocked = true;
  WCFilterFactory * ff = WCFilterFactory::GetInstance();
  WSrcCards * myC = getPool(pool); 
  if(!myC || myC->Size() < maxCards) unlocked = false; //Top pool lacks cards.
  SAFE_DELETE(myC);
  if(!check.size() || !unlocked) return unlocked;
  myC = NEW WSrcUnlockedCards(); //Requirements are independent of pool;
  WCardFilter * cf = ff->Construct(check);
  unlocked = !myC->isEmptySet(cf); //Quick check for empty set status.
  SAFE_DELETE(cf); //delete requirement filter
  SAFE_DELETE(myC); //delete pool.
  return unlocked;
}

bool MTGPack::isUnlocked(){
  if(unlockStatus == 0){
    if(meetsRequirements())
      unlockStatus = 1;
    else
      unlockStatus = -1;
  }
  return (unlockStatus > 0);
}

void MTGPacks::refreshUnlocked(){
  for(size_t t=0;t<packs.size();t++){
    if(packs[t]->unlockStatus < 0)
      packs[t]->unlockStatus = 0;
  }
}