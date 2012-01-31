#ifndef _COUNTERS_H_
#define _COUNTERS_H_
#include <string>

using std::string;
class MTGCardInstance;

/* One family of counters. Ex : +1/+1 */
class Counter
{
public:
    string name;
    int nb;
    int maxNb;
    int power, toughness;
    MTGCardInstance * target;
    Counter(MTGCardInstance * _target, int _power, int _toughness);
    Counter(MTGCardInstance * _target, const char * _name, int _power = 0, int _toughness = 0);
    int init(MTGCardInstance * _target, const char * _name, int _power, int _toughness);
    bool sameAs(const char * _name, int _power, int _toughness);
    bool cancels(int _power, int _toughness);
    int cancelCounter(int power, int toughness);
    int added();
    int removed();
};

/* Various families of counters attached to an instance of a card */
class Counters
{
public:
    int mCount;
   vector<Counter *>counters;
    MTGCardInstance * target;
    Counters(MTGCardInstance * _target);
    ~Counters();
    int addCounter(const char * _name, int _power = 0, int _toughness = 0);
    int addCounter(int _power, int _toughness);
    int removeCounter(const char * _name, int _power = 0, int _toughness = 0);
    int removeCounter(int _power, int _toughness);
    Counter * hasCounter(const char * _name, int _power = 0, int _toughness = 0);
    Counter * hasCounter(int _power, int _toughness);
    Counter * getNext(Counter * previous = NULL);
    int init();
};

#endif
