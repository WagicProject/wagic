#ifndef _WDATASRC_H_
#define _WDATASRC_H_

class WCardFilter;
struct WCardSort;
struct WDistort;
class PriceList;
class MTGCard;
class MTGDeck;
class MTGAllCards;
class JQuad;

class WSyncable{
public:
  WSyncable(int i=0) {hooked = NULL;currentPos = 0;};
  virtual ~WSyncable() {};
  //Local
  virtual bool Hook(WSyncable* s);
  virtual int getOffset() {return currentPos;};
  virtual bool setOffset(int i) {currentPos = i; return true;};
  //Recursive
  virtual int getPos();  
  virtual bool next();
  virtual bool prev();
protected:
  WSyncable * hooked; //Simple link list
  int currentPos; 
};

class WDataSource: public WSyncable{
public:
  WDataSource() {};
  virtual JQuad * getImage(int offset=0) {return NULL;};
  virtual JQuad * getThumb(int offset=0) {return NULL;};
  virtual MTGCard * getCard(int offset=0, bool ignore=false) {return NULL;};
  virtual MTGDeck * getDeck(int offset=0) {return NULL;};
  virtual WDistort * getDistort(int offset=0) {return NULL;};
  virtual bool thisCard(int mtgid) {return false;};
  virtual int getControlID() {return -1;}; //TODO FIXME: Need a "not a valid button" define.
  virtual void Update(float dt) {mLastInput += dt;};
  virtual void Touch() {mLastInput = 0;};
  virtual float getElapsed() {return mLastInput;};
  virtual void setElapsed(float f) {mLastInput = f;};
protected:
  float mLastInput;
};

class WSrcImage: public WDataSource{
public:
  virtual JQuad * getImage(int offset=0);
  WSrcImage(string s);

protected:
  string filename;
};

class WSrcCards: public WDataSource{
public:
  WSrcCards(float delay=0.2);
  ~WSrcCards();
  
  virtual JQuad * getImage(int offset=0);
  virtual JQuad * getThumb(int offset=0);
  virtual MTGCard * getCard(int offset=0, bool ignore=false);
  virtual int Size(bool all=false); //Returns the number of cards currently matched

  virtual void Shuffle();
  virtual bool thisCard(int mtgid);  
  virtual bool next();
  virtual bool prev();
  
  virtual void Sort(int method);
  virtual bool setOffset(int pos);
  virtual bool isEmptySet(WCardFilter * f);
  virtual void addFilter(WCardFilter * f);
  virtual void clearFilters();
  virtual WCardFilter* unhookFilters();
  virtual bool matchesFilters(MTGCard * c);
  virtual void validateFilters();
  virtual void bakeFilters(); //Discards all invalidated cards.
  virtual float filterFee();

  //Loads into us.
  virtual int loadMatches(MTGAllCards* ac); //loadMatches adds the cards from something
  virtual int loadMatches(MTGDeck * deck);  //into this, if it matches our filter
  virtual int loadMatches(WSrcCards* src, bool all=false);  //If all==true, ignore filters on src.
  
  //We load it
  virtual int addRandomCards(MTGDeck * i, int howmany=1);
  virtual int addToDeck(MTGDeck * i, int num=-1); //Returns num that didn't add

  enum {
     MAX_CYCLES = 4, //How many cycles to search, for addToDeck
     SORT_COLLECTOR,
     SORT_ALPHA,
     SORT_RARITY
  };
protected:
  vector<MTGCard*> cards;
  vector<size_t> validated; 
  WCardFilter * filtersRoot;
  float mDelay;
};

class WSrcUnlockedCards: public WSrcCards{ //Only unlocked cards.
public:
  WSrcUnlockedCards(float mDelay=0.2);
};

class WSrcDeck: public WSrcCards{
public:
  WSrcDeck(float delay=0.2) : WSrcCards(delay) {totalCards=0;};
  virtual int loadMatches(MTGDeck * deck);
  virtual int Add(MTGCard * c, int quantity=1);
  virtual int Remove(MTGCard * c, int quantity=1, bool erase=false);
  void Rebuild(MTGDeck * d);
  int count(MTGCard * c);
  int countByName(MTGCard * card, bool editions=false);
  int totalPrice(); 
  int totalCopies();
protected:
  map<int,int> copies; //Maps MTGID to card counts.
  int totalCards;
};

struct WCSortCollector{
  bool operator()(const MTGCard*l, const MTGCard*r);
};

struct WCSortAlpha{
  bool operator()(const MTGCard*l, const MTGCard*r);
};

struct WCSortRarity{
  int rareToInt(char r);
  bool operator()(const MTGCard*l, const MTGCard*r);
};


#endif