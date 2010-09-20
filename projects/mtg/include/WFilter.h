#ifndef _WFILTER_H_
#define _WFILTER_H_

class WCardFilter;

class WCFilterFactory{
public:
  WCFilterFactory() {};
  static WCFilterFactory * GetInstance();
  static void Destroy();
  WCardFilter * Construct(string src);
private:
  size_t findNext(string src, size_t start, char open='(', char close=')');
  WCardFilter * Leaf(string src);
  WCardFilter * Terminal(string src, string arg);
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
  WCFilterSet(int _setid=MTGSets::ALL_SETS) {setid=_setid;};
  WCFilterSet(string arg);
  bool isMatch(MTGCard *c) {return (setid==MTGSets::ALL_SETS || c->setId == setid);};
  string getCode();
  float filterFee() {return 0.2f;};
protected:
  int setid;
};
class WCFilterLetter: public WCardFilter{
public:
  WCFilterLetter(string arg);
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return 4.0f;}; //Alpha searches are expensive!
protected:
  char alpha;
};
class WCFilterColor: public WCardFilter{
public:
  WCFilterColor(int _c) {color = _c;};
  WCFilterColor(string arg);
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return 0.2f;};
protected:
  int color;
};
class WCFilterOnlyColor: public WCFilterColor{
public:
  WCFilterOnlyColor(int _c) : WCFilterColor(_c) {};
  WCFilterOnlyColor(string arg) : WCFilterColor(arg) {};
  bool isMatch(MTGCard * c);
  string getCode();
};
class WCFilterProducesColor: public WCFilterColor{
public:
  WCFilterProducesColor(int _c) : WCFilterColor(_c) {};
  WCFilterProducesColor(string arg) : WCFilterColor(arg) {};
  bool isMatch(MTGCard * c);
  string getCode();
};
class WCFilterNumeric: public WCardFilter{
public:
  WCFilterNumeric(int _num) {number = _num;};
  WCFilterNumeric(string arg);
  bool isMatch(MTGCard * c) = 0;
  string getCode() = 0;
  float filterFee() = 0;
protected:
  int number;
};
class WCFilterCMC: public WCFilterNumeric{
public:
  WCFilterCMC(int amt) : WCFilterNumeric(amt) {};
  WCFilterCMC(string arg) : WCFilterNumeric(arg) {};
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return number/20.0f;};
};
class WCFilterPower: public WCFilterNumeric{
public:
  WCFilterPower(int amt) : WCFilterNumeric(amt) {};
  WCFilterPower(string arg) : WCFilterNumeric(arg) {};
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return 2*number/12.0f;};
};
class WCFilterToughness: public WCFilterNumeric{
public:
  WCFilterToughness(int amt) : WCFilterNumeric(amt) {};
  WCFilterToughness(string arg) : WCFilterNumeric(arg) {};
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return 2*number/12.0f;};
};

class WCFilterType: public WCardFilter{
public:
  WCFilterType(string arg) {type = arg;};
  bool isMatch(MTGCard * c);
  string getCode();
  float filterFee() {return 0.4f;};
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