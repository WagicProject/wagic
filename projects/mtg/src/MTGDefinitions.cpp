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
"plainwalk",
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
"legendary",
"shadow",
"reachshadow",
"foresthome",
"islandhome",
"moutainhome",
"swamphome",
"plainshome",
"flanking",
"rampage",
"cloud",
"cantattack",
"mustattack",
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
	"End of turn",
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
	"endofturn",
	"cleanup",
	"beforenextturn"
};
