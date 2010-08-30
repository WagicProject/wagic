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
  ExtraCost(TargetChooser *_tc = NULL);
  virtual ~ExtraCost();
  virtual int setPayment(MTGCardInstance * card) = 0;
  virtual int isPaymentSet() = 0;
  virtual int canPay() = 0;
  virtual int doPay() = 0;
  virtual void Render(){};
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
  MTGCardInstance * target;
  SacrificeCost(TargetChooser *_tc = NULL);
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual SacrificeCost * clone() const;
};

//life cost 
class LifeCost: public ExtraCost{
public:
  MTGCardInstance * target;
  LifeCost(TargetChooser *_tc = NULL);
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual LifeCost * clone() const;
};

//tap other cost
class TapTargetCost: public ExtraCost{
public:
  MTGCardInstance * target;
  TapTargetCost(TargetChooser *_tc = NULL);
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual TapTargetCost * clone() const;
};
//exile as cost
class ExileTargetCost: public ExtraCost{
public:
  MTGCardInstance * target;
  ExileTargetCost(TargetChooser *_tc = NULL);
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual ExileTargetCost * clone() const;
};
//bounce cost
class BounceTargetCost: public ExtraCost{
public:
  MTGCardInstance * target;
  BounceTargetCost(TargetChooser *_tc = NULL);
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual BounceTargetCost * clone() const;
};

class CounterCost: public ExtraCost{
public:
  Counter * counter;
  MTGCardInstance * target;
  int hasCounters;
  CounterCost(Counter * _counter,TargetChooser *_tc = NULL);
  ~CounterCost();
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual CounterCost * clone() const;
};

#endif
