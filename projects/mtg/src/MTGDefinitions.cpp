#include "../include/MTGDefinitions.h"

char Constants::MTGColorChars[] = {'x','g','u','r','b','w','l'};
const char* Constants::MTGColorStrings[] = {"artifact", "green", "blue", "red", "black", "white", "land"};

int Constants::_r[7] = {75,	20,		20,	200,50,255,128};
int Constants::_g[7] = {30,	140,	30,	15,	50,255,128};
int Constants::_b[7] = {20,	0,		140,15,	50,255,128};

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
"cantcreaturecast",
"cantspellcast",
"onlyonecast",
"storm",
"bothcantcast",
"bothnocreature",
"oneboth",
"affinityartifacts",
"affinityplains",
"affinityforests",
"affinityislands",
"affinitymountains",
"affinityswamps",
"affinitygreencreatures",
"cantwin",
"nomaxhand",
};


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
