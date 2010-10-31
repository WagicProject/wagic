#ifndef _CARDPRIMITIVE_H_
#define _CARDPRIMITIVE_H_


#include <string>
#include <vector>
#include <map>

#include "ManaCost.h"

using namespace std;

class CardPrimitive {
 protected:
  vector<string> ftdText;
  string lcname;
  ManaCost manaCost;

 public:
  string text;
  string name;
  int init();

  int colors[Constants::MTG_NB_COLORS];
  map<int,int> basicAbilities;
  map<string,string> magicTexts;
  string magicText;
  int alias;
  string spellTargetType;
  int power;
  int toughness;
  vector<int>types;
  CardPrimitive();
  CardPrimitive(CardPrimitive * source);

  void setColor(int _color, int removeAllOthers = 0);
  void setColor(string _color, int removeAllOthers = 0);
  void removeColor(int color);
  int getColor();
  int hasColor(int _color);
  int countColors();

  int has(int ability);

  void setText(const string& value);
  const char * getText();

  void addMagicText(string value);
  void addMagicText(string value, string zone);

  void setName(const string& value);
  const string getName() const;
  const string getLCName() const;

  void addType(char * type_text);
  void addType(int id);
  void setType(const string& type_text);
  void setSubtype(const string& value);
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
