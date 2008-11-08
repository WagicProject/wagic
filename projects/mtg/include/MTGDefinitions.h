#ifndef _MTGDEFINITION_H_
#define _MTGDEFINITION_H_


#define TOTAL_NUMBER_OF_CARDS 4000

#define MTG_NB_COLORS 7

#define MTG_COLOR_ARTIFACT 0
#define MTG_COLOR_GREEN 1
#define MTG_COLOR_BLUE 2
#define MTG_COLOR_RED 3
#define MTG_COLOR_BLACK 4
#define MTG_COLOR_WHITE 5
#define MTG_COLOR_LAND 6


static char MTGColorChars[] = {'x','g','u','r','b','w','l'};
static int _r[7] = {75,	20,		20,	200,50,255,128};
static int _g[7] = {30,	140,	30,	15,	50,255,128};
static int _b[7] = {20,	0,		140,15,	50,255,128};


#define MTG_UNCOLORED 0
#define MTG_FOREST 1
#define MTG_ISLAND 2
#define MTG_MOUNTAIN 3
#define MTG_SWAMP 4
#define MTG_PLAIN 5

#define MTG_TYPE_CREATURE 10
#define MTG_TYPE_ARTIFACT 11
#define MTG_TYPE_ENCHANTMENT 12
#define MTG_TYPE_SORCERY 13
#define MTG_TYPE_LAND 14
#define MTG_TYPE_INSTANT 15



#define MTG_PHASE_UNTAP 0
#define MTG_PHASE_UPKEEP 1
#define MTG_PHASE_DRAW 2
#define MTG_PHASE_FIRSTMAIN 3
#define MTG_PHASE_COMBATBEGIN 4
#define MTG_PHASE_COMBATATTACKERS 5
#define MTG_PHASE_COMBATBLOCKERS 6
#define MTG_PHASE_COMBATDAMAGE 7
#define MTG_PHASE_COMBATEND 8
#define MTG_PHASE_SECONDMAIN 9
#define MTG_PHASE_ENDOFTURN 10
#define MTG_PHASE_EOT 10
#define MTG_PHASE_CLEANUP 11


#define TRAMPLE 0
#define FORESTWALK 1
#define ISLANDWALK 2
#define MOUNTAINWALK 3
#define SWAMPWALK 4
#define PLAINSWALK 5
#define FLYING 6
#define FIRSTSTRIKE 7
#define DOUBLESTRIKE 8
#define FEAR 9
#define FLASH 10
#define HASTE 11
#define LIFELINK 12
#define REACH 13
#define SHROUD 14
#define VIGILANCE 15
#define DEFENSER 16
#define DEFENDER 16
#define BANDING 17
#define PROTECTIONGREEN 18
#define PROTECTIONBLUE 19
#define PROTECTIONRED 20
#define PROTECTIONBLACK 21
#define PROTECTIONWHITE 22
#define UNBLOCKABLE 23
#define WITHER 24
#define PERSIST 25
#define RETRACE 26
#define EXALTED 27
#define LEGENDARY 28
#define SHADOW 29
#define REACHSHADOW 29
#define FORESTHOME 30
#define ISLANDHOME 31
#define MOUNTAINHOME 32
#define SWAMPHOME 33
#define PLAINSHOME 34

#define NB_BASIC_ABILITIES 35

static const char * MTGBasicAbilities[] = {
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
"plainshome"
};


#define RARITY_M 'M'
#define RARITY_R 'R'
#define RARITY_U 'U'
#define RARITY_C 'C'
#define RARITY_L 'L'


#define MAIN_FONT 0
#define MAGIC_FONT 1


static const char *MTGPhaseNames[] =
{
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
	"cleanup"
};





#endif
