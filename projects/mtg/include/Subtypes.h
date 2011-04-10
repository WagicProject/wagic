#ifndef _SUBTYPES_H_
#define _SUBTYPES_H_

#include <string>
#include <map>
#include <vector>
using namespace std;

class Subtypes
{
public:
    //A list of commonly used types
    enum
    {
        TYPE_CREATURE = 1,
        TYPE_ENCHANTMENT = 2,
        TYPE_SORCERY = 3,
        TYPE_INSTANT = 4,
        TYPE_LAND = 5,
        TYPE_ARTIFACT = 6,
        TYPE_LEGENDARY = 7,
        TYPE_EQUIPMENT = 8,
        TYPE_AURA = 9,
        TYPE_PLANESWALKER = 10,
        LAST_TYPE = TYPE_PLANESWALKER,
    };

protected:
    map<string, int> values;
    vector<string> valuesById;
public:
    static Subtypes * subtypesList;
    Subtypes();
    int find(string subtype, bool forceAdd = true);
    string find(unsigned int id);
};

#endif
