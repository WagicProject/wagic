#ifndef _MANACOST_H_
#define _MANACOST_H_

#include "../include/utils.h"
#include "../include/MTGDefinitions.h"


class ManaCostHybrid;

class ManaCost{
 protected:
  int cost[Constants::MTG_NB_COLORS+1];
  ManaCostHybrid * hybrids[10];
  int nbhybrids;
 public:
  static ManaCost * parseManaCost(string value, ManaCost * _manacost = NULL);
  void init();
  void x();
  ManaCost(int _cost[], int nb_elems);
  ManaCost();
  ~ManaCost();
  ManaCost(ManaCost * _manaCost);
  void copy (ManaCost * _manaCost);
  int getConvertedCost();
  string toString();
  int getCost(int color);
  //Returns NULL if i is greater than nbhybrids
  ManaCostHybrid * getHybridCost(unsigned int i);
  int getMainColor();
  int hasColor(int color);
  int remove (int color, int value);
  int add(int color, int value);
  int addHybrid(int c1, int v1, int c2, int v2);
  int tryToPayHybrids(ManaCostHybrid * _hybrids[], int _nbhybrids, int diff[]);
  void randomDiffHybrids(ManaCost * _cost, int diff[]);
  int add(ManaCost * _cost);
  int pay (ManaCost * _cost);

  //return 1 if _cost can be paid with current data, 0 otherwise
  int canAfford(ManaCost * _cost);

  int isPositive();
  ManaCost * Diff(ManaCost * _cost);
};

#endif
