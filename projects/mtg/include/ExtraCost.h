#ifndef _EXTRACOST_H_
#define _EXTRACOST_H_

#include <vector>
using std::vector;

class TargetChooser;
class MTGCardInstance;
class MTGAbility;

class ExtraCost{
public:
  TargetChooser * tc;
  MTGCardInstance * source;
  ExtraCost(TargetChooser *_tc = NULL);
  ~ExtraCost();
  virtual int setPayment(MTGCardInstance * card) = 0;
  virtual int isPaymentSet() = 0;
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
  virtual int doPay();
  virtual void Render();
  virtual int setSource(MTGCardInstance * _source);
  virtual SacrificeCost * clone() const;
};

#endif