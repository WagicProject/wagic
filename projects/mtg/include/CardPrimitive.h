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
  enum {
     NO_RESTRICTION = 0,
     PLAYER_TURN_ONLY = 1,
     AS_SORCERY = 2,
    MY_BEFORE_BEGIN = 3,
    MY_UNTAP = 4,
    MY_UPKEEP = 5,
    MY_DRAW = 6,
    MY_FIRSTMAIN = 7,
    MY_COMBATBEGIN = 8,
    MY_COMBATATTACKERS = 9,
    MY_COMBATBLOCKERS = 10,
    MY_COMBATDAMAGE = 11,
    MY_COMBATEND = 12,
    MY_SECONDMAIN = 13,
    MY_ENDOFTURN = 14,
    MY_EOT = 15,
    MY_CLEANUP = 16,
    MY_AFTER_EOT = 17,

    OPPONENT_BEFORE_BEGIN = 23,
    OPPONENT_UNTAP = 24,
    OPPONENT_UPKEEP = 25,
    OPPONENT_DRAW = 26,
    OPPONENT_FIRSTMAIN = 27,
    OPPONENT_COMBATBEGIN = 28,
    OPPONENT_COMBATATTACKERS = 29,
    OPPONENT_COMBATBLOCKERS = 30,
    OPPONENT_COMBATDAMAGE = 31,
    OPPONENT_COMBATEND = 32,
    OPPONENT_SECONDMAIN = 33,
    OPPONENT_ENDOFTURN = 34,
    OPPONENT_EOT = 35,
    OPPONENT_CLEANUP = 36,
    OPPONENT_AFTER_EOT = 37,

    BEFORE_BEGIN = 43,
    UNTAP = 44,
    UPKEEP = 45,
    DRAW = 46,
    FIRSTMAIN = 47,
    COMBATBEGIN = 48,
    COMBATATTACKERS = 49,
    COMBATBLOCKERS = 50,
    COMBATDAMAGE = 51,
    COMBATEND = 52,
    SECONDMAIN = 53,
    ENDOFTURN = 54,
    EOT = 55,
    CLEANUP = 56,
    AFTER_EOT = 57,
    
    VAMPIRES = 60,
    LESS_CREATURES = 61,
    SNOW_LAND_INPLAY =62,
    CASTED_A_SPELL = 63,
    ONE_OF_AKIND = 64,
    FOURTHTURN = 65,
    BEFORECOMBATDAMAGE = 66,
    AFTERCOMBAT = 67,
    DURINGCOMBAT = 68,
    OPPONENT_TURN_ONLY = 69,
    
   };
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
  bool hasRestriction;
  int restriction;
  string otherrestriction;
  int suspendedTime;

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
  const string& getName() const;
  const string& getLCName() const;

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
  void setRestrictions(int _restriction);
  int getRestrictions();
  void setOtherRestrictions(string _restriction);
  void getOtherRestrictions();
  const vector<string>& formattedText();
};


#endif
