/*
  Filter-like system for determining if a card meats certain criteria, for this and thisforeach autos
*/

#ifndef _THISDESCRIPTOR_H_
#define _THISDESCRIPTOR_H_

#include "Counters.h"
#include "MTGGameZones.h"
#include "MTGCardInstance.h"
#include "CardDescriptor.h"

class ThisDescriptor{
 public:
   int compareAbility;
   int comparisonMode;
   int comparisonCriterion;
   virtual int match(MTGCardInstance * card) = 0;
   virtual int match(MTGAbility * ability) = 0;
   int matchValue(int value);  
};

class ThisDescriptorFactory{
public:
  ThisDescriptor * createThisDescriptor(string s);
};

class ThisCounter:public ThisDescriptor{
 public:
  Counter * counter;
  virtual int match(MTGCardInstance * card);
  virtual int match(MTGAbility * ability) {return 0;};
  ThisCounter(Counter * _counter);
  ThisCounter(int power, int toughness, int nb, const char * name);
  ~ThisCounter();
};

class ThisCounterAny:public ThisDescriptor{
 public:
  virtual int match(MTGCardInstance *card);
  virtual int match(MTGAbility * ability) {return 0;};
  ThisCounterAny(int nb);
};

class ThisPower:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
    virtual int match(MTGAbility * ability) {return 0;};
    ThisPower(int power);
};

class ThisToughness:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
    virtual int match(MTGAbility * ability) {return 0;};
    ThisToughness(int toughness);
};

class ThisX:public ThisDescriptor{
  public:
    virtual int match(MTGAbility * ability);
    virtual int match(MTGCardInstance * card) {return 0;};
    ThisX(int x);
};

#endif