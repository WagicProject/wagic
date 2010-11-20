#ifndef _MTGDEFINITION_H_
#define _MTGDEFINITION_H_

const float DEFAULT_MENU_FONT_SCALE = 1.0f;
const float DEFAULT_MAIN_FONT_SCALE = 1.0f;
const float DEFAULT_TEXT_FONT_SCALE = 1.0f;

#include <string>
using std::string;
class Constants
{
 public:
  // Exception Codes

/* Exception codes */
 const static int PARSER_FAILED_INSTANTIATION = 1000;
 const static int PARSER_KEYWORD_NOT_MATCHED  = 2000;
 const static int PARSER_INVALID_KEYWORD      = 3000;


  // color constants
  static const string kManaColorless;
  static const string kManaGreen;   
  static const string kManaBlue;    
  static const string kManaRed;     
  static const string kManaBlack;   
  static const string kManaWhite;

  // alternative costs constants

  static const string kAlternativeKeyword;
  static const string kBuyBackKeyword;
  static const string kFlashBackKeyword;
  static const string kRetraceKeyword;
  static const string kKickerKeyword;

  // used for deck statistics
  static const int STATS_FOR_TURNS = 8;
  static const int STATS_MAX_MANA_COST = 9; 
  
  enum
  {
    MTG_COLOR_ARTIFACT = 0,
    MTG_COLOR_GREEN = 1,
    MTG_COLOR_BLUE = 2,
    MTG_COLOR_RED = 3,
    MTG_COLOR_BLACK = 4,
    MTG_COLOR_WHITE = 5,
    MTG_COLOR_LAND = 6,

    MTG_NB_COLORS = 7,


    MTG_UNCOLORED = 0,
    MTG_FOREST = 1,
    MTG_ISLAND = 2,
    MTG_MOUNTAIN = 3,
    MTG_SWAMP = 4,
    MTG_PLAIN = 5,


    MTG_TYPE_CREATURE = 10,
    MTG_TYPE_ARTIFACT = 11,
    MTG_TYPE_ENCHANTMENT = 12,
    MTG_TYPE_SORCERY = 13,
    MTG_TYPE_LAND = 14,
    MTG_TYPE_INSTANT = 15,


    MTG_PHASE_BEFORE_BEGIN = 0,
    MTG_PHASE_UNTAP = 1,
    MTG_PHASE_UPKEEP = 2,
    MTG_PHASE_DRAW = 3,
    MTG_PHASE_FIRSTMAIN = 4,
    MTG_PHASE_COMBATBEGIN = 5,
    MTG_PHASE_COMBATATTACKERS = 6,
    MTG_PHASE_COMBATBLOCKERS = 7,
    MTG_PHASE_COMBATDAMAGE = 8,
    MTG_PHASE_COMBATEND = 9,
    MTG_PHASE_SECONDMAIN = 10,
    MTG_PHASE_ENDOFTURN = 11,
    MTG_PHASE_EOT = 11,
    MTG_PHASE_CLEANUP = 12,
    MTG_PHASE_AFTER_EOT = 13,
    NB_MTG_PHASES = 14,

    TRAMPLE = 0,
    FORESTWALK = 1,
    ISLANDWALK = 2,
    MOUNTAINWALK = 3,
    SWAMPWALK = 4,
    PLAINSWALK = 5,
    FLYING = 6,
    FIRSTSTRIKE = 7,
    DOUBLESTRIKE = 8,
    FEAR = 9,
    FLASH = 10,
    HASTE = 11,
    LIFELINK = 12,
    REACH = 13,
    SHROUD = 14,
    VIGILANCE = 15,
    DEFENSER = 16,
    DEFENDER = 16,
    BANDING = 17,
    PROTECTIONGREEN = 18,
    PROTECTIONBLUE = 19,
    PROTECTIONRED = 20,
    PROTECTIONBLACK = 21,
    PROTECTIONWHITE = 22,
    UNBLOCKABLE = 23,
    WITHER = 24,
    PERSIST = 25,
    RETRACE = 26,
    EXALTED = 27,
    NOFIZZLE = 28,
    SHADOW = 29,
    REACHSHADOW = 30,
    FORESTHOME = 31,
    ISLANDHOME = 32,
    MOUNTAINHOME = 33,
    SWAMPHOME = 34,
    PLAINSHOME = 35,
    CLOUD = 36,
    CANTATTACK = 37,
    MUSTATTACK = 38,
    CANTBLOCK = 39,
	  DOESNOTUNTAP = 40,
	  OPPONENTSHROUD = 41,
	  INDESTRUCTIBLE = 42,
    INTIMIDATE = 43,
    DEATHTOUCH = 44,
    HORSEMANSHIP = 45,
    CANTREGEN = 46,
    ONEBLOCKER = 47,
	  INFECT = 48,
	  POISONTOXIC = 49,
	  POISONTWOTOXIC = 50,
	  POISONTHREETOXIC = 51,
	  PHANTOM = 52,
	  WILTING = 53,
	  VIGOR = 54,
	  CHANGELING = 55,
	  ABSORB = 56,//this need to be coded for players too "If a source would deal damage"
	  TREASON = 57,
	  UNEARTH = 58,
	  CANTLOSE = 59,
    CANTLIFELOSE = 60,
    CANTMILLLOSE = 61,
    CANTCASTCREATURE = 62,
	  CANTCAST = 63,
		CANTCASTTWO = 64,
	  STORM = 65,
	  BOTHCANTCAST = 66,
	  BOTHNOCREATURE = 67,
	  ONLYONEBOTH = 68,
    AFFINITYARTIFACTS = 69,
	  AFFINITYPLAINS = 70,
		AFFINITYFOREST = 71,
		AFFINITYISLAND = 72,
		AFFINITYMOUNTAIN = 73,
		AFFINITYSWAMP = 74,
		AFFINITYGREENCREATURES = 75,
	  CANTWIN = 76,
	  NOMAXHAND = 77,
    LEYLINE = 78,
    PLAYERSHROUD = 79,
    CONTROLLERSHROUD = 80,
	SUNBURST = 81,
	FLANKING = 82,
	EXILEDEATH = 83,

    NB_BASIC_ABILITIES = 84,


    RARITY_S = 'S',   //Special Rarity
    RARITY_M = 'M',   //Mythics
    RARITY_R = 'R',   //Rares
    RARITY_U = 'U',   //Uncommons
    RARITY_C = 'C',   //Commons
    RARITY_L = 'L',   //Lands
    RARITY_T = 'T',   //Tokens

    ECON_NORMAL = 0, //Options default to 0.
    ECON_HARD = 1,
    ECON_LUCK = 2,
    ECON_EASY = 3,

    //Price for singles
    PRICE_1M = 3000,
    PRICE_1R = 500,
    PRICE_1S = 200,
    PRICE_1U = 100,
    PRICE_1C = 20,
    PRICE_1L = 5,

    //Price in booster
    PRICE_BOOSTER = 700,
    PRICE_MIXED_BOOSTER = 800,
    CHANCE_CUSTOM_PACK = 15,
    CHANCE_PURE_OVERRIDE = 50,
    CHANCE_MIXED_OVERRIDE = 25,

    GRADE_SUPPORTED = 0,
    GRADE_BORDERLINE = 1,
    GRADE_UNOFFICIAL = 2,
    GRADE_CRAPPY = 3,
    GRADE_UNSUPPORTED = 4,
    GRADE_DANGEROUS = 5,

	ASKIP_NONE=0,
	ASKIP_SAFE=1,
	ASKIP_FULL=2,
  };

  static char MTGColorChars[];
  static const char* MTGColorStrings[];
  static int _r[], _g[], _b[];

  static map<string,int> MTGBasicAbilitiesMap;
  static const char* MTGBasicAbilities[];
  static const char* MTGPhaseNames[];
  static const char* MTGPhaseCodeNames[];

  static int GetBasicAbilityIndex(string mtgAbility);
  static int GetColorStringIndex(string mtgColor);
   
};

#endif
