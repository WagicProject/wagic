/*
  Filter-like system for determining if a card meets certain criteria, for this and thisforeach autos
*/

#ifndef _THISDESCRIPTOR_H_
#define _THISDESCRIPTOR_H_

#include "Counters.h"
#include "MTGGameZones.h"
#include "MTGCardInstance.h"
#include "CardDescriptor.h"

class ThisDescriptor{
 public:
   int comparisonMode;
   int comparisonCriterion;
   virtual int match(MTGCardInstance * card) = 0;
   int matchValue(int value);  
   virtual ~ThisDescriptor();
   virtual ThisDescriptor * clone() const = 0;
};

class ThisDescriptorFactory{
public:
  ThisDescriptor * createThisDescriptor(string s);
};

class ThisCounter:public ThisDescriptor{
 public:
  Counter * counter;
  virtual int match(MTGCardInstance * card);

  ThisCounter(Counter * _counter);
  ThisCounter(int power, int toughness, int nb, const char * name);
  ~ThisCounter();
  ThisCounter * clone() const;
};

class ThisCounterAny:public ThisDescriptor{
 public:
  virtual int match(MTGCardInstance *card);

  ThisCounterAny(int nb);
  ThisCounterAny * clone() const;
};

class ThisControllerlife:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisControllerlife(int life);
    ThisControllerlife * clone() const;
};

class ThisOpponentlife:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisOpponentlife(int olife);
    ThisOpponentlife * clone() const;
};

class ThisEquip:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisEquip(int equipment);
    ThisEquip * clone() const;
};

class ThisAuras:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisAuras(int auras);
    ThisAuras * clone() const;
};

class ThisOpponentDamageAmount:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisOpponentDamageAmount(int damagecount);
    ThisOpponentDamageAmount * clone() const;
};

class ThisUntapped:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisUntapped(int untapped);
    ThisUntapped * clone() const;
};

class ThisTapped:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisTapped(int tapped);
    ThisTapped * clone() const;
};


class ThisAttacked:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);

	ThisAttacked(int attack);
    ThisAttacked * clone() const;
};

class ThisBlocked:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);

	ThisBlocked(int block);
    ThisBlocked * clone() const;
};

class ThisNotBlocked:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);

	ThisNotBlocked(int unblocked);
    ThisNotBlocked * clone() const;
};

class ThisDamaged:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);

	ThisDamaged(int wasDealtDamage);
    ThisDamaged * clone() const;
};

class ThisPower:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisPower(int power);
    ThisPower * clone() const;
};

class ThisToughness:public ThisDescriptor{
 public:
    virtual int match(MTGCardInstance * card);
  
    ThisToughness(int toughness);
    ThisToughness * clone() const;
};

class ThisX:public ThisDescriptor{
  public:
    virtual int match(MTGCardInstance * card);
    ThisX(int x);
    ThisX * clone() const;
};

#endif
