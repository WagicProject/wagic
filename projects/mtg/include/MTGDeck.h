#ifndef _MTGDECK_H_
#define _MTGDECK_H_

#define MTG_ERROR -1

#include "../include/MTGDefinitions.h"






#include "../include/GameApp.h"
#include "../include/TexturesCache.h"


#include <string>

using std::string;


class GameApp;
class MTGCard;


#define MAX_SETS 50



class MtgSets{
 protected:
 public:
  int nb_items;
  string values[MAX_SETS];

 public:
  static MtgSets * SetsList;
  MtgSets();
  int Add(const char * subtype);
  int find(string value);

};


class MTGAllCards {
private:
  MTGCard * tempCard;
 protected:
  int conf_read_mode;
  int colorsCount[Constants::MTG_NB_COLORS];
  int total_cards;
  GameApp * parent;
  void init();
  void initCounters();
 public:

  TexturesCache * mCache;
  vector<int> ids;
  map<int, MTGCard *> collection;
  MTGAllCards();
  ~MTGAllCards();
  MTGAllCards(TexturesCache * cache);
  MTGCard * _(int id);
  void destroyAllCards();
  MTGAllCards(const char * config_file, const char * set_name);
  MTGAllCards(const char * config_file, const char * set_name, TexturesCache * cache);
  MTGCard * getCardById(int id);
  MTGCard * getCardByName(string name);
  int load(const char * config_file, const char * setName, int autoload = 1);
  int countByType(const char * _type);
  int countByColor(int color);
  int countBySet(int setId);
  int readConfLine(ifstream &file, int set_id);
  int totalCards();
  int randomCardId();
 private:
  int processConfLine(string s, MTGCard* card);
};


class MTGDeck{
 protected:
  string filename;

  
  int total_cards;

 public:
   TexturesCache * mCache;
  MTGAllCards * database;
   map <int,int> cards;
  string meta_desc;
  string meta_name;
  int totalCards();
  MTGDeck(TexturesCache * cache, MTGAllCards * _allcards);
  MTGDeck(const char * config_file, TexturesCache * cache, MTGAllCards * _allcards, int meta_only = 0);
  int addRandomCards(int howmany, int setId = -1, int rarity = -1, const char * subtype = NULL, int * colors = NULL, int nbcolors = 0);
  int add(int cardid);
  int add(MTGDeck * deck); // adds the contents of "deck" into myself
  int remove(int cardid);
  int removeAll();
  int add(MTGCard * card);
  int remove(MTGCard * card);
  int save();
  MTGCard * getCardById(int id);
 
};


#endif
