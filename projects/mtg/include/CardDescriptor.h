/*
  A Filter/Mask system for Card Instances to find cards matching specific settings such as color, type, etc...
*/

#ifndef _CARDDESCRIPTOR_H_
#define _CARDDESCRIPTOR_H_

#include "MTGCardInstance.h"
#include "MTGGameZones.h"
#include "Counters.h"

enum ENUM_COMPARISON_MODES
  {
    COMPARISON_NONE = 0,  // Needs to remain 0 for quick if(comparison_mode) checks
    COMPARISON_AT_MOST,
    COMPARISON_AT_LEAST,
    COMPARISON_EQUAL,
    COMPARISON_GREATER,
    COMPARISON_LESS,
    COMPARISON_UNEQUAL
  };

class CardDescriptor: public MTGCardInstance
{
 protected:
    MTGCardInstance * match_or(MTGCardInstance * card);
    MTGCardInstance * match_and(MTGCardInstance * card);
    MTGCardInstance * match_not(MTGCardInstance * card);
  bool valueInRange(int comparisonMode, int value, int criterion);
 public:
  enum Operator{
      CD_OR = 1,
      CD_AND = 2,
      CD_NOR = 3,
      CD_NAND = 4
  };

  Operator mode;
  int powerComparisonMode;
  int foretoldComparisonMode;
  int kickedComparisonMode;
  int toughnessComparisonMode;
  int manacostComparisonMode;
  int counterComparisonMode;
  int convertedManacost; // might fit better into MTGCardInstance?
  int zposComparisonMode;
  int zposition;
  int hasKickerCost;
  int hasFlashbackCost;
  int hasBackSide;
  int hasXCost;
  int anyCounter;
  int init();
  CardDescriptor();
  void unsecureSetKicked(int i);
  void unsecureSetHasKickerCost(int i);
  void unsecureSetHasFlashbackCost(int i);
  void unsecureSetHasBackSide(int i);
  void unsecureSetTapped(int i);
  void unsecuresetfresh(int k);
  void unsecuresetrecent(int j);
  void setisMultiColored(int w);
  void setNegativeSubtype( string value);
  int counterPower;
  int counterToughness;
  int counterNB;
  string counterName;
  MTGCardInstance * match(MTGCardInstance * card);
  MTGCardInstance * match(MTGGameZone * zone);
  MTGCardInstance * nextmatch(MTGGameZone * zone, MTGCardInstance * previous);
  
  void SetExclusionColor(int _color, int removeAllOthers = 0);

  uint8_t mColorExclusions;
  BasicAbilitiesSet mAbilityExclusions;

  int nameComparisonMode;
  int colorComparisonMode;
  string compareName;
  int CDopponentDamaged;
  int CDcontrollerDamaged;
  int CDdamager;
  int CDgeared;
  int CDdiscarded;
  int CDattached;
  int CDblocked;
  int CDcanProduceC;
  int CDcanProduceG;
  int CDcanProduceU;
  int CDcanProduceR;
  int CDcanProduceB;
  int CDcanProduceW;
  int CDnocolor;
};

#endif
