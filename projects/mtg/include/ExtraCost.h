#ifndef _EXTRACOST_H_
#define _EXTRACOST_H_

#include <vector>
#include "Counters.h"
using std::vector;

class TargetChooser;
class MTGCardInstance;
class MTGAbility;

class ExtraCost{
public:
  TargetChooser * tc;
  MTGCardInstance * source;
  MTGCardInstance * target;
  MTGCardInstance * targets[20];
  std::string mCostRenderString;

  ExtraCost(const std::string& inCostRenderString, TargetChooser *_tc = NULL);
  virtual ~ExtraCost();
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet() { return (target != NULL && targets != NULL); }
  virtual int canPay() { return 1; }
  virtual int doPay() = 0;
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual ExtraCost* clone() const = 0;
};

class ExtraCosts{
public:
  vector<ExtraCost *>costs;
  MTGCardInstance * source;
  MTGAbility * action;
  ExtraCosts();
  ~ExtraCosts();
  void Render();
  int tryToSetPayment(MTGCardInstance * card);
  int isPaymentSet();
  int canPay();
  int doPay();
  int reset();
  int setAction(MTGAbility * _action, MTGCardInstance * _source);
  void Dump();
  ExtraCosts * clone() const;
};

class SacrificeCost: public ExtraCost{
public:
  SacrificeCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual SacrificeCost * clone() const;
};

//life cost 
class LifeCost: public ExtraCost{
public:
  LifeCost(TargetChooser *_tc = NULL);

  virtual int doPay();
  virtual LifeCost * clone() const;
};

//Discard a random card cost 
class DiscardRandomCost: public ExtraCost{
public:
  DiscardRandomCost(TargetChooser *_tc = NULL);
  virtual int canPay();
  virtual int doPay();
  virtual DiscardRandomCost * clone() const;
};

//a choosen discard
class DiscardCost: public ExtraCost{
public:
  DiscardCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual DiscardCost * clone() const;
};

//tolibrary cost 
class ToLibraryCost: public ExtraCost{
public:
  ToLibraryCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual ToLibraryCost * clone() const;
};

//Millyourself cost 
class MillCost: public ExtraCost{
public:
  MillCost(TargetChooser *_tc = NULL);
  virtual int canPay();
  virtual int doPay();
  virtual MillCost * clone() const;
};

//Mill to exile yourself cost 
class MillExileCost: public MillCost{
public:
	MillExileCost(TargetChooser *_tc = NULL);
  virtual int doPay();
};

//tap other cost
class TapTargetCost: public ExtraCost{
public:
    TapTargetCost(TargetChooser *_tc = NULL);
    virtual int isPaymentSet();
    virtual int doPay();
    virtual TapTargetCost * clone() const;
};

//exile as cost
class ExileTargetCost: public ExtraCost{
public:
  ExileTargetCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual ExileTargetCost * clone() const;
};

//bounce cost
class BounceTargetCost: public ExtraCost{
public:
  BounceTargetCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual BounceTargetCost * clone() const;
};

//bounce cost
class Ninja: public ExtraCost{
public:
  Ninja(TargetChooser *_tc = NULL);
  virtual int isPaymentSet();
  virtual int doPay();
  virtual Ninja * clone() const;
};

class CounterCost: public ExtraCost{
public:
  Counter * counter;
  int hasCounters;
  CounterCost(Counter * _counter,TargetChooser *_tc = NULL);
  ~CounterCost();
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual CounterCost * clone() const;
};

#endif
