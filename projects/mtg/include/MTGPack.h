#ifndef _MTGPACCK_H_
#define _MTGPACK_H_

class ShopBooster;

class MTGPackEntry{
public:
  virtual ~MTGPackEntry() {};
  virtual int addCard(WSrcCards * pool,MTGDeck * to) = 0;
  int copies;  
};

class MTGPackEntryRandom: public MTGPackEntry{
public:
  MTGPackEntryRandom() {filter = ""; copies=1;};
  MTGPackEntryRandom(string f, int c=1) {filter = f; copies = c;};
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
  friend class ShopBooster;
  friend class MTGSetInfo;
  bool meetsRequirements(); //Check if pool contains locked cards.
  bool isUnlocked();
  bool isValid() {return bValid;};
  void load(string filename);
  int assemblePack(MTGDeck * to);
  
  MTGPack() {bValid = false; unlockStatus = 0; price=Constants::PRICE_BOOSTER;};
  MTGPack(string s) {bValid = false; load(s); unlockStatus = 0;};
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
  vector<MTGPackSlot*> slotss;
};

class MTGPacks{
public:
  ~MTGPacks();
  MTGPack * randomPack(int key=0);
  void loadAll();
  int size() {return (int)packs.size();};
  void refreshUnlocked();
  
  static MTGPack * getDefault();
private:
  static MTGPack defaultBooster;
  vector<MTGPack*> packs;
};
#endif