#ifndef _MANACOST_H_
#define _MANACOST_H_

#include "../include/utils.h"
#include "../include/MTGDefinitions.h"


class ManaCostHybrid;
class ExtraCosts;
class ExtraCost;
class MTGAbility;
class MTGCardInstance;

class ManaCost{
 protected:
  int cost[Constants::MTG_NB_COLORS+1];
  ManaCostHybrid * hybrids[10];
  unsigned int nbhybrids;
  int extraCostsIsCopy;
  
 public:
  ExtraCosts * extraCosts;
  static ManaCost * parseManaCost(string value, ManaCost * _manacost = NULL, MTGCardInstance * c = NULL);
  void init();
  void x();
  ManaCost(int _cost[], int nb_elems = 1);
  ManaCost();
  ~ManaCost();
  ManaCost(ManaCost * _manaCost);
  void copy (ManaCost * _manaCost);
  int isNull();
  int getConvertedCost();
  string toString();
  int getCost(int color);
  //Returns NULL if i is greater than nbhybrids
  ManaCostHybrid * getHybridCost(unsigned int i);
  int getMainColor();
  int hasColor(int color);
  int remove (int color, int value);
  int add(int color, int value);

  //
  // Extra Costs (sacrifice...)
  //
  int addExtraCost(ExtraCost * _cost);
  int setExtraCostsAction(MTGAbility * action, MTGCardInstance * card);
  int isExtraPaymentSet();
  int resetExtraPayment();
  int doPayExtra();

  int addHybrid(int c1, int v1, int c2, int v2);
  int tryToPayHybrids(ManaCostHybrid * _hybrids[], int _nbhybrids, int diff[]);
  void randomDiffHybrids(ManaCost * _cost, int diff[]);
  int add(ManaCost * _cost);
  int pay (ManaCost * _cost);

  //return 1 if _cost can be paid with current data, 0 otherwise
  int canAfford(ManaCost * _cost);

  int isPositive();
  ManaCost * Diff(ManaCost * _cost);
#ifdef WIN32
  void Dump();
#endif
};

std::ostream& operator<<(std::ostream& out, const ManaCost& m);

#endif
