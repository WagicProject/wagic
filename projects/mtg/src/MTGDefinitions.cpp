#include "PrecompiledHeader.h"
#include <string.h>
using std::string;

#include "MTGDefinitions.h"

char Constants::MTGColorChars[] = {'x','g','u','r','b','w','l'};
vector <const char*> Constants::MTGColorStrings;

const string Constants::kManaColorless      = "colorless";
const string Constants::kManaGreen          = "green";
const string Constants::kManaBlue           = "blue";
const string Constants::kManaRed            = "red";
const string Constants::kManaBlack          = "black";
const string Constants::kManaWhite          = "white";

int Constants::_r[7] = {75,	20,		20,	200,50,255,128};
int Constants::_g[7] = {30,	140,	30,	15,	50,255,128};
int Constants::_b[7] = {20,	0,		140,15,	50,255,128};


const string Constants::kAlternativeKeyword = "alternative";
const string Constants::kBuyBackKeyword = "buyback";
const string Constants::kFlashBackKeyword = "flashback";
const string Constants::kRetraceKeyword = "retrace";
const string Constants::kKickerKeyword = "kicker";
const string Constants::kMorphKeyword = "facedown";

int Constants::NB_Colors = 0; //Store the Max number of colors.

const char* Constants::MTGBasicAbilities[] = {
    "trample",
    "forestwalk",
    "islandwalk",
    "mountainwalk",
    "swampwalk",
    "plainswalk",
    "flying",
    "first strike",
    "double strike",
    "fear",
    "flash",
    "haste",
    "lifelink",
    "reach",
    "shroud",
    "vigilance",
    "defender",
    "banding",
    "protection from green",
    "protection from blue",
    "protection from red",
    "protection from black",
    "protection from white",
    "unblockable",
    "wither",
    "persist",
    "retrace",
    "exalted",
    "nofizzle",
    "shadow",
    "reachshadow",
    "foresthome",
    "islandhome",
    "mountainhome",
    "swamphome",
    "plainshome",
    "cloud",
    "cantattack",
    "mustattack",
    "cantblock",
    "doesnotuntap",
    "opponentshroud",
    "indestructible",
    "intimidate",
    "deathtouch",
    "horsemanship",
    "cantregen",
    "oneblocker",
    "infect",
    "poisontoxic",
    "poisontwotoxic",
    "poisonthreetoxic",
    "phantom",//prevents damage and remove 1 +1/+1 counter
    "wilting",//source takes damage in the form of -1/-1 counters.
    "vigor",//instead of taking damage the source gains +1/+1 counters
    "changeling",//this card is every creature type at all times
    "absorb",//timeshifted sliver ability. if damage would be dealt to card, prevent 1 of that damage.
    "treason",
    "unearth",
    "cantlose",
    "cantlifelose",
    "cantmilllose",
    "snowlandwalk",
    "nonbasiclandwalk",
    "strong",//cant be blocked by creature with less power
    "storm",
    "phasing",
    "split second",
    "weak",//cant block creatures with more power
    "affinityartifacts",
    "affinityplains",
    "affinityforests",
    "affinityislands",
    "affinitymountains",
    "affinityswamps",
    "affinitygreencreatures",
    "cantwin",
    "nomaxhand",
    "leyline",
    "playershroud",
    "controllershroud",
    "sunburst",
    "flanking",
    "exiledeath",
    "legendarylandwalk",
    "desertlandwalk",
    "snowforestlandwalk",
    "snowplainslandwalk",
    "snowmountainlandwalk",
    "snowislandlandwalk",
    "snowswamplandwalk",
    "canattack",
    "hydra",
    "undying",
    "poisonshroud",
    "noactivatedability",
    "notapability",
    "nomanaability",
    "onlymanaability",
    "poisondamager",//deals damage to players as poison counters.
    "soulbond",
    "lure",
    "nolegend"
};

map<string,int> Constants::MTGBasicAbilitiesMap;
int Constants::GetBasicAbilityIndex(string basicAbllity)
{
    if ( Constants::MTGBasicAbilitiesMap.size() == 0 )
    {
        for (int idx = 0; idx < Constants::NB_BASIC_ABILITIES; ++idx)
        {
            string ability = MTGBasicAbilities[idx];
            MTGBasicAbilitiesMap[ability] =  idx;
        }
    }
    if ( Constants::MTGBasicAbilitiesMap.find(basicAbllity) != Constants::MTGBasicAbilitiesMap.end() )
        return Constants::MTGBasicAbilitiesMap[basicAbllity];

    return -1;
}

int Constants::GetColorStringIndex(string mtgColor)
{
    for (int idx = 0; idx < Constants::NB_Colors; ++idx)
    {
        if (Constants::MTGColorStrings[idx])
            return idx;
    }
    
    return -1;
}

const char* Constants::MTGPhaseNames[] =
{
	"---",
	"Untap",
	"Upkeep",
	"Draw",
	"Main phase 1",
	"Combat begins",
	"Attackers",
	"Blockers",
	"Combat damage",
	"Combat ends",
	"Main phase 2",
	"End",
	"Cleanup",
	"---"
};

const char* Constants::MTGPhaseCodeNames[] =
{
	"beginofturn",
	"untap",
	"upkeep",
	"draw",
	"firstmain",
	"combatbegins",
	"attackers",
	"blockers",
	"combatdamage",
	"combatends",
	"secondmain",
	"end",
	"cleanup",
	"beforenextturn"
};
