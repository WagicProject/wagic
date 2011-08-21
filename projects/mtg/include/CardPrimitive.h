/* CardPrimitive objects represent the cards database. 
 * For MTG we have thousands of those, that stay constantly in Ram
 * on low-end devices such as the PSP, adding stuff to this class can have a very high cost
 * As an example, with 16'000 card primitives (the rough number of cards in MTG), adding a simple 16 bytes attribute costs 250kB (2% of the total available ram on the PSP)
 */
#ifndef _CARDPRIMITIVE_H_
#define _CARDPRIMITIVE_H_


#include <string>
#include <vector>
#include <map>
#include <bitset>
#include "config.h"
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


class CastRestrictions {
public:
    string restriction;
    string otherrestriction;

    CastRestrictions * clone() const
    {
        return NEW CastRestrictions(*this);
    };
};

class CardPrimitive
#ifdef TRACK_OBJECT_USAGE
    : public InstanceCounter<CardPrimitive>
#endif
{
private:
    string text;
    vector<string> formattedText;
    CastRestrictions * restrictions;

protected:
    string lcname;
    ManaCost manaCost;

public:
    string name;
    int init();

    uint8_t colors;
    typedef std::bitset<Constants::NB_BASIC_ABILITIES> BasicAbilitiesSet;
    BasicAbilitiesSet basicAbilities;

    map<string,string> magicTexts;
    string magicText;
    int alias;
    string spellTargetType;
    int power;
    int toughness;
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

    static uint8_t ConvertColorToBitMask(int inColor);

    int has(int ability);

    void setText(const string& value);
    const vector<string>& getFormattedText();

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
    const string getRestrictions();
    void setOtherRestrictions(string _restriction);
    const string getOtherRestrictions();
};


#endif
