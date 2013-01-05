#ifndef _EXTRACOST_H_
#define _EXTRACOST_H_

#include <vector>
#include "Counters.h"
#include "ObjectAnalytics.h"

using std::vector;

class TargetChooser;
class MTGCardInstance;
class MTGAbility;

class ExtraCost
#ifdef TRACK_OBJECT_USAGE
    : public InstanceCounter<ExtraCost>
#endif
{
public:
  TargetChooser * tc;
  MTGCardInstance * source;
  MTGCardInstance * target;
  std::string mCostRenderString;

  ExtraCost(const std::string& inCostRenderString, TargetChooser *_tc = NULL);
  virtual ~ExtraCost();
  
  virtual int setPayment(MTGCardInstance * card);
  virtual int isPaymentSet()
  {
      return (target != NULL);
  }

  virtual int canPay() { return 1; }
  virtual int doPay() = 0;
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual ExtraCost* clone() const = 0;
};

class ExtraCosts
{
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

class SacrificeCost : public ExtraCost
{
public:
  SacrificeCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual SacrificeCost * clone() const;
};

//life cost 
class LifeCost : public ExtraCost
{
public:
  LifeCost(TargetChooser *_tc = NULL);
  virtual int canPay();
  virtual int doPay();
  virtual LifeCost * clone() const;
};

//pyrhaixa mana
class LifeorManaCost : public ExtraCost
{
public:
    LifeorManaCost(TargetChooser *_tc = NULL,string manaType = "");
    string manaType;
    virtual int canPay();
    virtual int doPay();
    virtual LifeorManaCost * clone() const;
};

//Discard a random card cost 
class DiscardRandomCost : public ExtraCost
{
public:
  DiscardRandomCost(TargetChooser *_tc = NULL);
  virtual int canPay();
  virtual int doPay();
  virtual DiscardRandomCost * clone() const;
};

//a choosen discard
class DiscardCost : public ExtraCost
{
public:
  DiscardCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual DiscardCost * clone() const;
};

//cycle
class CycleCost : public ExtraCost
{
public:
  CycleCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual CycleCost * clone() const;
};

//tolibrary cost 
class ToLibraryCost : public ExtraCost
{
public:
  ToLibraryCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual ToLibraryCost * clone() const;
};

//Millyourself cost 
class MillCost : public ExtraCost
{
public:
  MillCost(TargetChooser *_tc = NULL);
  virtual int canPay();
  virtual int doPay();
  virtual MillCost * clone() const;
};

//Mill to exile yourself cost 
class MillExileCost : public MillCost
{
public:
	MillExileCost(TargetChooser *_tc = NULL);
  virtual int doPay();
};

//unattach cost
class unattachCost : public ExtraCost
{
public:
    unattachCost(MTGCardInstance * realSource = NULL);
    MTGCardInstance * rSource;
    virtual int isPaymentSet();
    virtual int canPay();
    virtual int doPay();
    virtual unattachCost * clone() const;
};
//tap  cost
class TapCost : public ExtraCost
{
public:
    TapCost();
    virtual int isPaymentSet();
    virtual int canPay();
    virtual int doPay();
    virtual TapCost * clone() const;
};

//untap  cost
class UnTapCost : public ExtraCost
{
public:
    UnTapCost();
    virtual int isPaymentSet();
    virtual int canPay();
    virtual int doPay();
    virtual UnTapCost * clone() const;
};

//tap other cost
class TapTargetCost : public ExtraCost
{
public:
    TapTargetCost(TargetChooser *_tc = NULL);
    virtual int isPaymentSet();
    virtual int doPay();
    virtual TapTargetCost * clone() const;
};
//untap a target as cost
class UnTapTargetCost : public ExtraCost
{
public:
    UnTapTargetCost(TargetChooser *_tc = NULL);
    virtual int isPaymentSet();
    virtual int doPay();
    virtual UnTapTargetCost * clone() const;
};

//exile as cost
class ExileTargetCost : public ExtraCost
{
public:
  ExileTargetCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual ExileTargetCost * clone() const;
};

//bounce cost
class BounceTargetCost : public ExtraCost
{
public:
  BounceTargetCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual BounceTargetCost * clone() const;
};

//bounce cost
class Ninja : public ExtraCost
{
public:
  Ninja(TargetChooser *_tc = NULL);
  virtual int canPay();
  virtual int isPaymentSet();
  virtual int doPay();
  virtual Ninja * clone() const;
};

class CounterCost : public ExtraCost
{
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
