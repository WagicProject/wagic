#ifndef _MTGDECK_H_
#define _MTGDECK_H_

#define MTG_ERROR -1

#include "../include/MTGDefinitions.h"
#include "../include/GameApp.h"
#include "../include/WResourceManager.h"


#include <string>

using std::string;
class GameApp;
class MTGCard;
class CardPrimitive;
class MTGPack;
class MTGSetInfo{
public:
  MTGSetInfo(string _id);
  ~MTGSetInfo();
  string id;      //Short name: 10E, RAV, etc. Automatic from folder.
  string author;  //Author of set, for crediting mod makers, etc.
  string name;    //Long name: Tenth Edition
  int block;      //For future use by tournament mode, etc.
  int year;       //The year the set was released.
  //TODO Way to group cards by name, rather than mtgid.

  void count(MTGCard * c);

  int totalCards();
  string getName();
  string getBlock();
  void processConfLine(string line);

  enum {
    //For memoized counts
    LAND = 0,
    COMMON = 1,
    UNCOMMON = 2,
    RARE = 3, 
    MAX_RARITY = 4, //For boosters, mythic is part of rare... always.
    MYTHIC = 4,
    TOTAL_CARDS = 5,
    MAX_COUNT = 6
  };

  MTGPack * mPack; //Does it use a specialized booster pack?
  bool bZipped;       //Is this set's images present as a zip file?
  bool bThemeZipped;  //[...] in the theme?
  int counts[MTGSetInfo::MAX_COUNT];  
};

class MTGSets{
public:
  friend class MTGSetInfo;
  MTGSets();
  ~MTGSets();
  
  int Add(const char * subtype);
  int findSet(string value);
  int findBlock(string s);
  int size();

  int getSetNum(MTGSetInfo*i);

  int operator[](string id); //Returns set id index, -1 for failure.
  string operator[](int id); //Returns set id name, "" for failure.
    
  MTGSetInfo* getInfo(int setID); 
  MTGSetInfo* randomSet(int blockId = -1, int atleast = -1); //Tries to match, otherwise 100% random unlocked set
 
protected:
  vector<string> blocks;
  vector<MTGSetInfo*> setinfo;
};

extern MTGSets setlist;

class MTGAllCards {
private:
  MTGCard * tempCard; //used by parser
  CardPrimitive * tempPrimitive; //used by parser
  int currentGrade; //used by Parser (we don't want an additional attribute for the primitives for that as it is only used at load time)
 protected:
  int conf_read_mode;
  int colorsCount[Constants::MTG_NB_COLORS];
  int total_cards;
  GameApp * parent;
  void init();
  void initCounters();
 public:
  enum {
   READ_ANYTHING = 0,
   READ_CARD = 1,
   READ_METADATA = 2,
  };
  vector<int> ids;
  map<int, MTGCard *> collection;
  map<string,CardPrimitive *>primitives;
  MTGAllCards();
  ~MTGAllCards();
  MTGCard * _(int id);
  void destroyAllCards();
  MTGAllCards(const char * config_file, const char * set_name);
  MTGCard * getCardById(int id);
  MTGCard * getCardByName(string name);
  int load(const char * config_file, const char * setName = NULL, int autoload = 1);
  int countByType(const char * _type);
  int countByColor(int color);
  int countBySet(int setId);
  int totalCards();
  int randomCardId();
 private:
  int processConfLine(string &s, MTGCard* card, CardPrimitive * primitive);
  bool addCardToCollection(MTGCard * card, int setId);
  CardPrimitive * addPrimitive(CardPrimitive * primitive, MTGCard * card = NULL);
};


class MTGDeck{
 protected:
  string filename;
  int total_cards;

 public:
  MTGAllCards * database;
   map <int,int> cards;
  string meta_desc;
  string meta_name;
  int totalCards();
  int totalPrice();
  MTGDeck(MTGAllCards * _allcards);
  MTGDeck(const char * config_file, MTGAllCards * _allcards, int meta_only = 0);
  int addRandomCards(int howmany, int * setIds = NULL, int nbSets = 0, int rarity = -1, const char * subtype = NULL, int * colors = NULL, int nbcolors = 0);
  int add(int cardid);
  int add(MTGDeck * deck); // adds the contents of "deck" into myself
  int complete();
  int remove(int cardid);
  int removeAll();
  int add(MTGCard * card);
  int remove(MTGCard * card);
  int save();
  MTGCard * getCardById(int id);
 
};


#endif
