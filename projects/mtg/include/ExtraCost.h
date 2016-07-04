#ifndef _EXTRACOST_H_
#define _EXTRACOST_H_

#include <vector>
#include "Counters.h"
#include "ObjectAnalytics.h"
#include "ManaCost.h"

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
  ManaCost * costToPay;
  MTGCardInstance * source;
  MTGCardInstance * target;
  std::string mCostRenderString;

  ExtraCost(const std::string& inCostRenderString, TargetChooser *_tc = NULL,ManaCost * _costToPay = NULL);
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

//extraextra
class ExtraManaCost : public ExtraCost
{
public:
  ExtraManaCost(ManaCost * cost = NULL);
  virtual int tryToSetPayment(MTGCardInstance * card);
  virtual int isPaymentSet();
  virtual int canPay();
  virtual int doPay();
  virtual ExtraManaCost * clone() const;
};

class SacrificeCost : public ExtraCost
{
public:
  SacrificeCost(TargetChooser *_tc = NULL);
  virtual int canPay();
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

//Specific life cost 
class SpecificLifeCost : public ExtraCost
{
private:
    int slc;

public:
  SpecificLifeCost(TargetChooser *_tc = NULL, int slc = 0);
  virtual int canPay();
  virtual int doPay();
  virtual SpecificLifeCost * clone() const;
};

//phyrexian mana
class LifeorManaCost : public ExtraCost
{
private:
    string manaType;
    ManaCost manaCost;

public:
    LifeorManaCost(TargetChooser *_tc = NULL, string manaType = "");
    virtual int canPay();
    virtual int doPay();
    virtual LifeorManaCost * clone() const;
    ManaCost * getManaCost();
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

//toGraveyard cost 
class ToGraveCost : public ExtraCost
{
public:
  ToGraveCost(TargetChooser *_tc = NULL);
  virtual int doPay();
  virtual ToGraveCost * clone() const;
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
class UnattachCost : public ExtraCost
{
public:
    UnattachCost(MTGCardInstance * realSource = NULL);
    MTGCardInstance * rSource;
    virtual int isPaymentSet();
    virtual int canPay();
    virtual int doPay();
    virtual UnattachCost * clone() const;
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

//Snow  cost
class SnowCost : public ExtraCost
{
public:
    SnowCost();
    virtual int isPaymentSet();
    virtual int canPay();
    virtual int doPay();
    virtual SnowCost * clone() const;
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
//Convoke
class Convoke : public ExtraCost
{
public:
	ManaCost * getReduction();
	Convoke(TargetChooser *_tc = NULL);
	virtual int canPay();
	virtual int isPaymentSet();
	virtual int doPay();
	virtual Convoke * clone() const;
};
//delve
class Delve : public ExtraCost
{
public:
	Delve(TargetChooser *_tc = NULL);
	virtual int canPay();
	virtual int isPaymentSet();
	virtual int doPay();
	virtual Delve * clone() const;
};
//offering cost
class Offering : public ExtraCost
{
public:
  Offering(TargetChooser *_tc = NULL);
  virtual int canPay();
  virtual int isPaymentSet();
  virtual int doPay();
  virtual Offering * clone() const;
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
