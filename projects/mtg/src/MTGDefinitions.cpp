#include "PrecompiledHeader.h"
#include <string.h>
using std::string;

#include "MTGDefinitions.h"

char Constants::MTGColorChars[] = {'x','g','u','r','b','w','c','l'};
vector <const char*> Constants::MTGColorStrings;

const string Constants::kManaColorless      = "colorless";
const string Constants::kManaGreen          = "green";
const string Constants::kManaBlue           = "blue";
const string Constants::kManaRed            = "red";
const string Constants::kManaBlack          = "black";
const string Constants::kManaWhite          = "white";
const string Constants::kManaWaste          = "waste";

int Constants::_r[7] = {75,20,20,200,50,255,128};
int Constants::_g[7] = {30,140,30,15,50,255,128};
int Constants::_b[7] = {20,0,140,15,50,255,128};


const string Constants::kAlternativeKeyword = "alternative";
const string Constants::kBuyBackKeyword = "buyback";
const string Constants::kFlashBackKeyword = "flashback";
const string Constants::kRetraceKeyword = "retrace";
const string Constants::kKickerKeyword = "kicker";
const string Constants::kMorphKeyword = "facedown";
const string Constants::kBestowKeyword = "bestow";

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
    "poisontoxic", // Card has toxic 1
    "poisontwotoxic", // Card has toxic 2
    "poisonthreetoxic", // Card has toxic 3
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
    "nolegend",
    "canplayfromgraveyard",
    "tokenizer",//parallel lives,
    "mygraveexiler",
    "oppgraveexiler",
    "librarydeath",
    "shufflelibrarydeath",
    "offering",
    "evadebigger",
    "spellmastery",
    "nolifegain",
    "nolifegainopponent",
    "auraward",
    "madness",
    "protectionfromcoloredspells",
    "mygcreatureexiler",
    "oppgcreatureexiler",
    "zerocast",
    "trinisphere",
    "canplayfromexile",
    "libraryeater",
    "devoid",
    "cantchangelife",
    "combattoughness",
    "cantpaylife",
    "cantbesacrified", // The card cannot be sacrified (e.g. "Hithlain Rope").
    "skulk",
    "menace",
    "nosolo",
    "mustblock",
    "dethrone",
    "overload",
    "shackler",
    "flyersonly",
    "tempflashback",
    "legendruleremove",
    "canttransform",
    "asflash",
    "conduited",
    "canblocktapped",
    "oppnomaxhand",
    "cantcrew",
    "hiddenface",//test for hiding card
    "anytypeofmana",
    "necroed",//hide necored
    "cantpwattack",
    "canplayfromlibrarytop",//all
    "canplaylandlibrarytop",//land
    "canplaycreaturelibrarytop",//creature
    "canplayartifactlibrarytop",//artifact
    "canplayinstantsorcerylibrarytop",//instant or sorcery
    "showfromtoplibrary",
    "showopponenttoplibrary",
    "totemarmor",
    "discardtoplaybyopponent",
    "modular",
    "mutate", //it can mutate
    "adventure", //it can be adventure
    "mentor",
    "prowess",
    "nofizzle alternative", //No fizzle if card has been paid with alternative cost.
    "hasotherkicker", //Kicker cost is expressed with "other" keyword (eg. not mana kicker such as life and/or tap a creature)
    "partner", //Has partner ability
    "canbecommander", //Can be a commander (eg. some planeswalkers can)
    "poisonfourtoxic", // Card has toxic 4
    "threeblockers", //It can be blocked just by 3 creatures or more.
    "handdeath", //It goes in hand after death.
    "inplaydeath", //It goes back in play untapped after death.
    "inplaytapdeath", //It goes back in play tapped after death.
    "gainedexiledeath", //It goes to exile after death (use just to give add ability to instants and sorceries which originally have not, e.g. with transforms keyword)
    "gainedhanddeath", //It goes to hand after death (use just to give add ability to instants and sorceries which originally have not, e.g. with transforms keyword)
    "cycling", //It has cycling ability
    "foretell", //It has foretell cost
    "anytypeofmanaability", //It allows to spend mana as it were of any color to activate abilities.
    "boast", //It has boast ability
    "twoboast", //It has boast twice ability (e.g. Birgi, God of Storytelling)
    "replacescry", //It has scry replacement ability
    "hasnokicker", //Kicker cost is not a real kicker cost (eg. cards with Fuse cost)
    "undamageable", //It cannot be damaged by any source
    "lifefaker", //It's a card wich modify the life increasement when a @lifeof triggers occours (e.g. Angel of Vitality)
    "doublefacedeath", //It goes to temp zone after death (e.g. Double face card)
    "gaineddoublefacedeath", //It goes to temp after death (use just to give add ability to instants and sorceries which originally have not, e.g. with transforms keyword)
    "twodngtrg", //It makes rooms abilities trigger twice (e.g. Hama Pashar, Ruin Seeker)
    "nodngopp", //Opponent can't venture (e.g. Keen-Eared Sentry)
    "nodngplr", //Controller can't venture
    "canplayauraequiplibrarytop", //auras and equipment
    "counterdeath", //It gains a 1/1 counter when it returns from graveyard (to use with inplaydeath and inplaytapdeath)
    "dungeoncompleted", //This dungeon has been completed
    "perpetuallifelink", //It gains lifelink perpetually
    "perpetualdeathtouch", //It gains deathtouch perpetually
    "noncombatvigor", //instead of taking non-combat damage the source gains +1/+1 counters (e.g. Stormwild Capridor)
    "nomovetrigger", //no trigger when playing these cards (e.g. fake ability cards such as Davriel Conditions, Davriel Offers, Annihilation Rooms)
    "canloyaltytwice", //Planeswalker can activate its loyalty abilities twice in a turn (e.g. "Urza, Planeswalker").
    "showopponenthand", //opponent plays with his hand revealed.
    "showcontrollerhand", //controller plays with his hand revealed.
    "hasreplicate", //Kicker cost is a replicate cost (e.g. "Vacuumelt")
    "isprey", //Creature has been haunted by some other card.
    "hasdisturb", //Retrace cost is a disturb cost (e.g. "Beloved Beggar")
    "daybound", //Card has daybound (e.g. "Brutal Cathar")
    "nightbound", //Card has nightbound (e.g. "Moonrage Brute")
    "decayed", //Card has decayed.
    "hasstrive", //Kicker cost is a strive cost (e.g. "Aerial Formation")
    "isconspiracy", //The card is a conspiracy (e.g. "Double Stroke")
    "hasaftermath", //Flashback cost is an aftemath cost (e.g. "Claim // Fame")
    "noentertrg", //Creatures entering the battlefield don't cause abilities to trigger (e.g. "Hushbringer").
    "nodietrg", //Creatures dying don't cause abilities to trigger (e.g. "Hushbringer").
    "training", //Has training ability (e.g. "Gryff Rider")
    "energyshroud", //Player can't get energy counters (e.g. "Solemnity").
    "expshroud", //Player can't get experience counters (e.g. "Solemnity").
    "countershroud", //Card can't get any kind of counter (e.g. "Solemnity").
    "nonight", //It can't become night (e.g. "Angel of Eternal Dawn").
    "nodamageremoved", //Damage is not removed from card (e.g. "Patient Zero").
    "backgroundpartner", //Can choose a backgorund partner as commander (e.g. "Faceless One").
    "bottomlibrarydeath", //It goes to bottom of library after death (e.g. "Quintorius, Loremaster").
    "noloyaltydamage", //Damage does not cause loyalty counter to be removed from a Planeswalker (e.g. "Spark Rupture").
    "nodefensedamage", //Damage does not cause defense counter to be removed from a Battle.
    "affinityallcreatures", //Cost 1 less for each creature in all battlefields.
    "affinitycontrollercreatures", //Cost 1 less for each creature in controller battlefield.
    "affinityopponentcreatures", //Cost 1 less for each creature in opponent battlefield.
    "affinityalldeadcreatures", //Cost 1 less for each died creature in this turn.
    "affinityparty", //Cost 1 less for each creature in your party.
    "affinityenchantments", //Cost 1 less for each enchantment in your battlefield.
    "affinitybasiclandtypes", //Cost 1 less for each basic land type in your battlefield.
    "affinitytwobasiclandtypes", //Cost 2 less for each basic land type in your battlefield.
    "affinitygravecreatures", //Cost 1 less for each creature in your graveyard.
    "affinityattackingcreatures", //Cost 1 less for each attacking creature in your battlefield.
    "affinitygraveinstsorc", //Cost 1 less for each instant or sorcery in your graveyard.
    "poisonfivetoxic", // Card has toxic 5
    "poisonsixtoxic", // Card has toxic 6
    "poisonseventoxic", // Card has toxic 7
    "poisoneighttoxic", // Card has toxic 8
    "poisonninetoxic", // Card has toxic 9
    "poisontentoxic", // Card has toxic 10
    "eqpasinst", // Can equip as instant
    "canloyaltyasinst" // Can activate loyalty abilities as instant (e.g. "The Wandering Emperor").
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
        if (Constants::MTGColorStrings[idx] == mtgColor)
            return idx;
    }
    
    return -1;
}

const string Constants::MTGPhaseNames[] =
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
