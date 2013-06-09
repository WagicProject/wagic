#ifndef _WFILTER_H_
#define _WFILTER_H_
/**
  @file WFilter.h
  Includes classes and functionality related to card filtering.

  @defgroup Filter Card filtering
  @{
*/

class WCardFilter;

/**
  A factory class used to construct a WCardFilter from a string. It does so via
  recursive calls to Leaf(), which handles the structure of the filter, and Terminal(),
  which handles specific filter components.
*/
class WCFilterFactory
{
public:
    WCFilterFactory() {};
    static WCFilterFactory * GetInstance();
    static void Destroy();
    WCardFilter * Construct(string src);
private:
    size_t findNext(string src, size_t start, char open = '(', char close = ')');
    WCardFilter * Leaf(string src);
    WCardFilter * Terminal(string src, string arg);
    static WCFilterFactory * me;
};

/**
  An abstract parent to all filter components. Children provide implementations of 
  isMatch() to filter cards, and filterFee() to adjust card prices based on filtering.
*/
class WCardFilter
{
public:
    WCardFilter() {};
    virtual ~WCardFilter() {};

    /**
      Returns true if the card argument matches a certain condition.
    */
    virtual bool isMatch(MTGCard *)
    {
        return true;
    }
    ;
    /**
      Returns the filter in the same string form used by WCFilterFactory to construct it.
    */
    virtual string getCode() = 0;

    /**
      Returns a price multiplier which is summed using some logic (see the WCFBranch classes). Once a 
      multiplier has been figured, it is added to the card's base price in GameStateShop::purchasePrice(), 
      using the following equation: price + (price * filterFee * ((countAllCards - countMatchFilter)/countAllCards).
    */
    virtual float filterFee()
    {
        return 0.0f;
    }
    ;
};

/**   
  An abstract parent to all filter logical branches. 
*/
class WCFBranch: public WCardFilter
{
public:
    WCFBranch(WCardFilter * a, WCardFilter * b)
    {
        lhs = a;
        rhs = b;
    }
    ;
    ~WCFBranch()
    {
        SAFE_DELETE(lhs);
        SAFE_DELETE(rhs);
    }
    ;
    virtual bool isMatch(MTGCard * c) = 0;
    virtual string getCode() = 0;
    virtual WCardFilter * Right()
    {
        return rhs;
    }
    ;
    virtual WCardFilter * Left()
    {
        return lhs;
    }
    ;
protected:
    WCardFilter *lhs, *rhs;
};

/**
  Handles the logical inclusive OR operator. 
  When returning a filterFee, it returns whichever is higher.
*/
class WCFilterOR: public WCFBranch
{
public:
    WCFilterOR(WCardFilter * a, WCardFilter * b) :
        WCFBranch(a, b)
    {
    }
    ;
    bool isMatch(MTGCard *c);
    string getCode();
    float filterFee();
};

/**
  Handles the logical inclusive AND operator. 
  When returning a filterFee, it returns the sum of the two filters.
*/
class WCFilterAND: public WCFBranch
{
public:
    WCFilterAND(WCardFilter * a, WCardFilter * b) :
        WCFBranch(a, b)
    {
    }
    ;
    bool isMatch(MTGCard *c)
    {
        return (lhs->isMatch(c) && rhs->isMatch(c));
    }
    ;
    string getCode();
    float filterFee();
};


/**
  Logical group operator. 
*/
class WCFilterGROUP: public WCardFilter
{
public:
    WCFilterGROUP(WCardFilter * _k)
    {
        kid = _k;
    }
    ;
    ~WCFilterGROUP()
    {
        SAFE_DELETE(kid);
    }
    ;
    bool isMatch(MTGCard *c)
    {
        return kid->isMatch(c);
    }
    ;
    string getCode();
    float filterFee()
    {
        return kid->filterFee();
    }
    ;
protected:
    WCardFilter * kid;
};


/**
  A logical negation, returns true if the child filter returns false.
*/
class WCFilterNOT: public WCardFilter
{
public:
    WCFilterNOT(WCardFilter * _k)
    {
        kid = _k;
    }
    ;
    ~WCFilterNOT()
    {
        SAFE_DELETE(kid);
    }
    ;
    bool isMatch(MTGCard *c)
    {
        return !kid->isMatch(c);
    }
    ;
    string getCode();
protected:
    WCardFilter * kid;
};

/**
  An empty filter, matches anything. Used when WCFilterFactory is passed an empty string,
  or cannot parse the syntax of something.
*/
class WCFilterNULL: public WCardFilter
{
public:
    WCFilterNULL()
    {
    }
    ;
    string getCode()
    {
        return "NULL";
    }
    ;
    bool isMatch(MTGCard *)
    {
        return true;
    }
    ;
};

/**
  Matches a card against a particular set. 
*/
class WCFilterSet: public WCardFilter
{
public:
    WCFilterSet(int _setid = MTGSets::ALL_SETS)
    {
        setid = _setid;
    }
    ;
    WCFilterSet(string arg);
    bool isMatch(MTGCard *c)
    {
        return (setid == MTGSets::ALL_SETS || c->setId == setid);
    }
    ;
    string getCode();
    float filterFee()
    {
        return 0.2f;
    }
    ;
protected:
    int setid;
};

/**
  Matches a card that contains a particular letter. 
*/
class WCFilterLetter: public WCardFilter
{
public:
    WCFilterLetter(string arg);
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee()
    {
        return 4.0f; //Alpha searches are expensive!
    }
    ; 
protected:
    char alpha;
};

/**
  Matches a card that contains a particular color (inclusively). 
*/
class WCFilterColor: public WCardFilter
{
public:
    WCFilterColor(int _c)
    {
        color = _c;
    }
    ;
    WCFilterColor(string arg);
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee()
    {
        return 0.2f;
    }
    ;
protected:
    int color;
};

/**
  Matches a card that is a particular color (exclusively). 
*/
class WCFilterOnlyColor: public WCFilterColor
{
public:
    WCFilterOnlyColor(int _c) : WCFilterColor(_c) {};
    WCFilterOnlyColor(string arg) : WCFilterColor(arg) {};
    bool isMatch(MTGCard * c);
    string getCode();
};

/**
  Matches a card that produces mana of a particular color (inclusively). 
*/
class WCFilterProducesColor: public WCFilterColor
{
public:
    WCFilterProducesColor(int _c) : WCFilterColor(_c) {};
    WCFilterProducesColor(string arg) : WCFilterColor(arg) {};
    bool isMatch(MTGCard * c);
    string getCode();
};

/**
  The base class to all filters which compare a numeric value. 
*/
class WCFilterNumeric: public WCardFilter
{
public:
    WCFilterNumeric(int _num)
    {
        number = _num;
    }
    ;
    WCFilterNumeric(string arg);
    bool isMatch(MTGCard * c) = 0;
    string getCode() = 0;
    float filterFee() = 0;
protected:
    int number;
};

/**
  Matches a card with a given combined mana cost.
*/
class WCFilterCMC: public WCFilterNumeric
{
public:
    WCFilterCMC(int amt) : WCFilterNumeric(amt) {};
    WCFilterCMC(string arg) : WCFilterNumeric(arg) {};
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee()
    {
        return number / 20.0f;
    }
    ;
};

/**
  Matches a card with a given power.
*/
class WCFilterPower: public WCFilterNumeric
{
public:
    WCFilterPower(int amt) : WCFilterNumeric(amt) {};
    WCFilterPower(string arg) : WCFilterNumeric(arg) {};
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee()
    {
        return 2 * number / 12.0f;
    }
    ;
};

/**
  Matches a card with a given toughness.
*/
class WCFilterToughness: public WCFilterNumeric
{
public:
    WCFilterToughness(int amt) : WCFilterNumeric(amt) {};
    WCFilterToughness(string arg) : WCFilterNumeric(arg) {};
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee()
    {
        return 2 * number / 12.0f;
    }
    ;
};

/**
  Matches a card of a given type (inclusively).
*/
class WCFilterType: public WCardFilter
{
public:
    WCFilterType(string arg)
    {
        type = arg;
    }
    ;
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee()
    {
        return 0.4f;
    }
    ;
protected:
    string type;
};


/**
  Matches a card of a given rarity. If passed 'A' for any rarity, will always return true.
*/
class WCFilterRarity: public WCardFilter
{
public:
    WCFilterRarity(char _r)
    {
        rarity = _r;
    }
    ;
    WCFilterRarity(string arg);
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee();
protected:
    char rarity;
};


/**
  Matches a card of a given basic ability.
*/
class WCFilterAbility: public WCardFilter
{
public:
    WCFilterAbility(int _a)
    {
        ability = _a;
    }
    ;
    WCFilterAbility(string arg);
    bool isMatch(MTGCard * c);
    string getCode();
    float filterFee();
protected:
    int ability;
};

/**@} This comment used by Doxyyen. */
#endif
