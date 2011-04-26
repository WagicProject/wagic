#ifndef _CARDPRIMITIVE_H_
#define _CARDPRIMITIVE_H_


#include <string>
#include <vector>
#include <map>
#include <bitset>

#include "ManaCost.h"
#include "ObjectAnalytics.h"

using namespace std;

const uint8_t kColorBitMask_Artifact = 0x01;
const uint8_t kColorBitMask_Green    = 0x02;
const uint8_t kColorBitMask_Blue     = 0x04;
const uint8_t kColorBitMask_Red      = 0x08;
const uint8_t kColorBitMask_Black    = 0x10;
const uint8_t kColorBitMask_White    = 0x20;
const uint8_t kColorBitMask_Land     = 0x40;


class CardPrimitive
#ifdef TRACK_OBJECT_USAGE
    : public InstanceCounter<CardPrimitive>
#endif
{
protected:
    string lcname;
    ManaCost manaCost;

public:
    string text;
    string name;
    int init();

    uint8_t colors;
    std::bitset<Constants::NB_BASIC_ABILITIES> basicAbilities;

    map<string,string> magicTexts;
    string magicText;
    int alias;
    string spellTargetType;
    int power;
    int toughness;
    bool hasRestriction;
    string restriction;
    string otherrestriction;
    int suspendedTime;

    vector<int>types;
    CardPrimitive();
    CardPrimitive(CardPrimitive * source);
    virtual ~CardPrimitive();

    void setColor(int _color, int removeAllOthers = 0);
    void setColor(const string& _color, int removeAllOthers = 0);
    void removeColor(int color);
    int getColor();
    bool hasColor(int inColor);
    int countColors();

    int has(int ability);

    void setText(const string& value);
    const string& getText();

    void addMagicText(string value);
    void addMagicText(string value, string zone);

    void setName(const string& value);
    const string& getName() const;
    const string& getLCName() const;

    void addType(char * type_text);
    void addType(int id);
    void setType(const string& type_text);
    void setSubtype(const string& value);
    int removeType(string value, int removeAll = 0);
    int removeType(int value, int removeAll = 0);
    bool hasSubtype(int _subtype);
    bool hasSubtype(const char * _subtype);
    bool hasSubtype(const string& _subtype);
    bool hasType(int _type);
    bool hasType(const char * type);

    void setManaCost(const string& value);
    ManaCost * getManaCost();
    bool isCreature();
    bool isLand();
    bool isSpell();

    void setPower(int _power);
    int getPower();
    void setToughness(int _toughness);
    int getToughness();
    void setRestrictions(string _restriction);
    void getRestrictions();
    void setOtherRestrictions(string _restriction);
    void getOtherRestrictions();
};


#endif
