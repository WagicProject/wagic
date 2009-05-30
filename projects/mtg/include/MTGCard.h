#ifndef _MTGCARD_H_
#define _MTGCARD_H_

#define MTGCARD_NAME_SIZE 30
#define MTGCARD_TEXT_SIZE 300

#define MTG_IMAGE_WIDTH 200
#define MTG_IMAGE_HEIGHT 285

#define MTG_MINIIMAGE_WIDTH 45
#define MTG_MINIIMAGE_HEIGHT 64


#define MAX_TYPES_PER_CARD 10




#include "ManaCost.h"


class TexturesCache;


#include <string>
#include <vector>
using namespace std;

class MTGCard {
 protected:



  int mtgid;
  ManaCost manaCost;


  char rarity;

  char image_name[MTGCARD_NAME_SIZE];

  int init();

 public:
  TexturesCache * mCache;
  string text;
  string name;

  int colors[Constants::MTG_NB_COLORS];
  int basicAbilities[Constants::NB_BASIC_ABILITIES];
  vector<string> formattedText;
  string magicText;
  int alias;
  string spellTargetType;
  int formattedTextInit;
  int power;
  int toughness;
  int setId;
  static const char * const Colors_To_Text[];
  int nb_types;
  int types[MAX_TYPES_PER_CARD];
  MTGCard();
  MTGCard(TexturesCache * cache, int set_id);
  MTGCard(MTGCard * source);
  JQuad * getQuad(TexturesCache * cache);
  JQuad * getQuad(int type=1);
  JQuad * getThumb();

  void setColor(int _color, int removeAllOthers = 0);
  int getColor();
  int hasColor(int _color);
  const char * colorToString();

  void setMTGId(int id);
  int getMTGId();
  int getId();

  int has(int ability);

  char getRarity();
  void setRarity(char _rarity);

  const	char * getSetName();

  //void setImageName( char * value);
  char * getImageName ();

  void setText( string value);
  const char * getText();

  void addMagicText(string value);

  void setName( string value);
  const char * getName();

  void addType(char * type_text);
  void addType(int id);
  void setType(const char * type_text);
  void setSubtype( string value);
  int removeType(string value, int removeAll = 0);
  int removeType(int value, int removeAll = 0);
  int hasSubtype(int _subtype);
  int hasSubtype(const char * _subtype);
  int hasSubtype(string _subtype);
  int hasType(int _type);
  int hasType(const char * type);

  void setManaCost(string value);
  ManaCost * getManaCost();
  int isACreature();

  void setPower(int _power);
  int getPower();
  void setToughness(int _toughness);
  int getToughness();


};




#endif
