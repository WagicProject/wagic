#ifndef _WFILTER_H_
#define _WFILTER_H_

class WCardFilter;

class WCFilterFactory{
public:
  WCFilterFactory() {};
  static WCFilterFactory * GetInstance();
  static void Destroy();
  WCardFilter * Construct(string x);
private:
  size_t findNext(string src, size_t start, char open='(', char close=')');
  WCardFilter * Leaf(string src);
  WCardFilter * Terminal(string type, string arg);
  static WCFilterFactory * me;
};

class WCardFilter{
public:
  WCardFilter() {};
  virtual ~WCardFilter() {};
  virtual bool isMatch(MTGCard * c) {return true;};
  virtual string getCode() = 0;
  virtual float filterFee() {return 0.0f;};
};

class WCFBranch: public WCardFilter{
public:
  WCFBranch(WCardFilter * a, WCardFilter * b) {lhs=a;rhs=b;};
  ~WCFBranch() {SAFE_DELETE(lhs); SAFE_DELETE(rhs);};
  virtual bool isMatch(MTGCard * c) = 0;
  virtual string getCode() = 0;
  virtual WCardFilter * Right(){return rhs;};
  virtual WCardFilter * Left(){return lhs;};
protected:
  WCardFilter *lhs, *rhs;
};

class WCFilterOR: public WCFBranch{
public:
  WCFilterOR(WCardFilter * a, WCardFilter * b): WCFBranch(a,b) {};
  bool isMatch(MTGCard *c);
  string getCode();
  float filterFee();
};

class WCFilterAND: public WCFBranch{
public:
  WCFilterAND(WCardFilter * a, WCardFilter * b): WCFBranch(a,b) {};
  bool isMatch(MTGCard *c) {return (lhs->isMatch(c) && rhs->isMatch(c));};
  string getCode();
  float filterFee();
};

class WCFilterGROUP: public WCardFilter{
public:
  WCFilterGROUP(WCardFilter * _k) {kid = _k;};
  ~WCFilterGROUP() {SAFE_DELETE(kid);};
  bool isMatch(MTGCard *c) {return kid->isMatch(c);};
  string getCode();
  float filterFee() {return kid->filterFee();};
protected:
  WCardFilter * kid;
};

class WCFilterNOT: public WCardFilter{
public:
  WCFilterNOT(WCardFilter * _k) {kid = _k;};
  ~WCFilterNOT() {SAFE_DELETE(kid);};
  bool isMatch(MTGCard *c) {return !kid->isMatch(c);};
  string getCode();
protected:
  WCardFilter * kid;
};

class WCFilterNULL: public WCardFilter{
public:
  WCFilterNULL() {};
  string getCode() {return "NULL";};
  bool isMatch(MTGCard *c) {return true;};
};


//Filter terminals:
class WCFilterSet: public WCardFilter{
public:
  WCFilterSet(int _setid=-1) {setid=_setid;};
  WCFilterSet(string arg);
  bool isMatch(MTGCard *c) {return (setid==-1 || c->setId == setid);};
  string getCode();
  float filterFee() {return 0.1f;};
protected:
  int setid;
};
class WCFilterColor: public WCardFilter{
public:
  WCFilterColor(int _c) {color = _c;};
  WCFilterColor(string arg);
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return 0.1f;};
protected:
  int color;
};
class WCFilterType: public WCardFilter{
public:
  WCFilterType(string arg) {type = arg;};
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return 0.2f;};
protected:
  string type;
};
class WCFilterRarity: public WCardFilter{
public:
  WCFilterRarity(char _r) {rarity = _r;};
  WCFilterRarity(string arg);
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee();
protected:
  char rarity;
};
class WCFilterAbility: public WCardFilter{
public:
  WCFilterAbility(int _a) {ability = _a;};
  WCFilterAbility(string arg);
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee();
protected:
  int ability;
};

#endif