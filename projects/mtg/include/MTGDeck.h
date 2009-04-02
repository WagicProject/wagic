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
 protected:
  int conf_read_mode;
  int colorsCount[Constants::MTG_NB_COLORS];
  int total_cards;
  GameApp * parent;
  void init();
  void initCounters();
 public:

  TexturesCache * mCache;
  MTGCard * _(int i);
  MTGCard * collection[Constants::TOTAL_NUMBER_OF_CARDS];
  MTGAllCards();
  ~MTGAllCards();
  MTGAllCards(TexturesCache * cache);
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


class MTGDeck:public MTGAllCards{
 protected:
  string filename;
  MTGAllCards * allcards;
 public:
  MTGDeck(const char * config_file, TexturesCache * cache, MTGAllCards * _allcards);
  int addRandomCards(int howmany, int setId = -1, int rarity = -1, const char * subtype = NULL);
  int add(int cardid);
  int remove(int cardid);
  int removeAll();
  int add(MTGCard * card);
  int remove(MTGCard * card);
  int save();
};


#endif
