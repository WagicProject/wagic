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
        TYPE_SNOW = 8,
        TYPE_BASIC = 9,
        TYPE_WORLD = 10,
        TYPE_EQUIPMENT = 11,
        TYPE_AURA = 12,
        TYPE_PLANESWALKER = 13,
        TYPE_TRIBAL = 14,
        TYPE_PLANE = 15,
        TYPE_SCHEME = 16,
        TYPE_VANGUARD = 17,
        LAST_TYPE = TYPE_VANGUARD,
    };

protected:
    map<string, int> values;
    vector<string> valuesById;
    map<int,int> subtypesToType;
public:
    static Subtypes * subtypesList;
    Subtypes();
    int find(string subtype, bool forceAdd = true);
    string find(unsigned int id);
    bool isSubtypeOfType(string subtype, string type);
    bool isSubtypeOfType(int subtype, int type);
    bool isSuperType(int type);
    bool isType(int type);
    bool isSubType(int type);
    int add(string value, int parentType);
    const vector<string> getValuesById();
};

#endif
