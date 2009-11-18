#ifndef _MTGCARD_H_
#define _MTGCARD_H_

#define MTGCARD_NAME_SIZE 30

#define MTG_IMAGE_WIDTH 200
#define MTG_IMAGE_HEIGHT 285

#define MTG_MINIIMAGE_WIDTH 45
#define MTG_MINIIMAGE_HEIGHT 64

#define MAX_TYPES_PER_CARD 10


#include <string>
#include <vector>
#include <map>

#include "ManaCost.h"

using namespace std;

class MTGCard {
 protected:
  int mtgid;
  ManaCost manaCost;
  char rarity;
  char image_name[MTGCARD_NAME_SIZE];
  vector<string> ftdText;
  int init();
  string lcname;

 public:
  string text;
  string name;

  int colors[Constants::MTG_NB_COLORS];
  map<int,int> basicAbilities;
  string magicText;
  int alias;
  string spellTargetType;
  int power;
  int toughness;
  int setId;
  static const char * const Colors_To_Text[];
  int nb_types;
  int types[MAX_TYPES_PER_CARD];
  MTGCard();
  MTGCard(int set_id);
  MTGCard(MTGCard * source);

  void setColor(int _color, int removeAllOthers = 0);
  void setColor(string _color, int removeAllOthers = 0);
  void removeColor(int color);
  int getColor();
  int hasColor(int _color);
  int countColors();
  const char * colorToString();

  void setMTGId(int id);
  int getMTGId();
  int getId();

  int has(int ability);

  char getRarity();
  void setRarity(char _rarity);

  //void setImageName( char * value);
  char * getImageName ();

  void setText(string value);
  const char * getText();

  void addMagicText(string value);

  void setName(string value);
  const string getName() const;
  const string getLCName() const;

  void addType(char * type_text);
  void addType(int id);
  void setType(const char * type_text);
  void setSubtype( string value);
  int removeType(string value, int removeAll = 0);
  int removeType(int value, int removeAll = 0);
  bool hasSubtype(int _subtype);
  bool hasSubtype(const char * _subtype);
  bool hasSubtype(string _subtype);
  bool hasType(int _type);
  bool hasType(const char * type);

  void setManaCost(string value);
  ManaCost * getManaCost();
  bool isCreature();
  bool isLand();
  bool isSpell();

  void setPower(int _power);
  int getPower();
  void setToughness(int _toughness);
  int getToughness();

  const vector<string>& formattedText();
};




#endif
