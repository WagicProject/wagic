#ifndef _MTGPACCK_H_
#define _MTGPACK_H_

class MTGPackEntry{
public:
  virtual int addCard(WSrcCards * pool,MTGDeck * to) = 0;
  int copies;  
};

class MTGPackEntryRandom: public MTGPackEntry{
public:
  int addCard(WSrcCards * pool,MTGDeck * to);
  string filter;
};
class MTGPackEntrySpecific: public MTGPackEntry{
public:
  int addCard(WSrcCards * pool,MTGDeck * to);
  MTGCard * card;
};

class MTGPackEntryNothing: public MTGPackEntry{
public:
  int addCard(WSrcCards * pool,MTGDeck * to) {return 0;};
};

class MTGPackSlot{
public:
  ~MTGPackSlot();
  int add(WSrcCards * ocean, MTGDeck * to, int carryover);
  void addEntry(MTGPackEntry*item);
  int copies;  
  string pool;
  vector<MTGPackEntry*> entries;
};

class MTGPack{
public:
  friend class MTGPacks;
  bool meetsRequirements(); //Check if pool contains locked cards.
  bool isUnlocked();
  bool isValid() {return bValid;};
  void load(string filename);
  int assemblePack(MTGDeck * to);
  
  MTGPack(string s) {bValid = false; load(s); unlockStatus=0;};
  ~MTGPack();
  string getName();
  string getSort() {return sort;};
  int getPrice() {return price;};
  static WSrcCards * getPool(string poolstr);
protected:
  void countCards();
  string name;   //Name of the pack.
  string type;   //"Booster", "Deck", "Whatever"
  string pool;   //The starting pool.
  string sort;   //The sorting method.
  string check;  //Unlock requirements.
  string desc;   //Big card description.
  bool bValid;
  int unlockStatus;

  int price;     //Base price.
  int minCards, maxCards;
  vector<MTGPackSlot*> slots;
};

class MTGPacks{
public:
  ~MTGPacks();
  MTGPack * randomPack(int key=0);
  void loadAll();
  int size() {return (int)packs.size();};
  void refreshUnlocked();
  

private:
  vector<MTGPack*> packs;
};
#endif