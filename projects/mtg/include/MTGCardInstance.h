#ifndef _MTG_CARD_INSTANCE_H_
#define _MTG_CARD_INSTANCE_H_

#include "MTGCard.h"
#include "CardPrimitive.h"
#include "MTGGameZones.h"
#include "MTGAbility.h"
#include "WResourceManager.h"
#include "ManaCost.h"
#include "Damage.h"
#include "Targetable.h"


class MTGCardInstance;
class MTGPlayerCards;
class MTGAbility;
class MTGCard;
class ManaCost;
class UntapBlockers;
class CardDescriptor;
class Counters;
struct Pos;

#include <list>
using namespace std;

class MTGCardInstance: public CardPrimitive, public MTGCard, public Damageable
#ifdef TRACK_OBJECT_USAGE
    , public InstanceCounter<MTGCardInstance>
#endif

{
private:
    bool blocked; //Blocked this turn or not?
protected:
    int untapping;
    int nb_damages;
    string sample;
    int tapped;
    int lifeOrig;
    MTGPlayerCards * belongs_to;
    MTGCardInstance * getNextPartner();
    void initMTGCI();
    int addBlocker(MTGCardInstance * c);
    int removeBlocker(MTGCardInstance * c);
    int init();
public:
    vector<MTGCardInstance*>parentCards;
    vector<MTGCardInstance*>childrenCards;
    vector<MTGAbility *>cardsAbilities;

    int setAttacker(int value);
    int setDefenser(MTGCardInstance * c);
    MTGGameZone * currentZone;
    Pos* view;
    int X;
    int castX;
    int setX;
    int alternateCostPaid[ManaCost::MANA_PAID_WITH_BESTOW + 1]; 
    int paymenttype;
    int castMethod; /* Tells if the card reached its current zone by being cast or not (brought into the zone by an effect). non 0 == cast, 0 == not cast */
    int frozen;
    int sunburst;
    int equipment;
    int mutation;
    int damageInflictedAsCommander;
    int numofcastfromcommandzone;
    int auras;
    int wasDealtDamage;
    int combatdamageToOpponent;
    int damageToOpponent;
    int damageToController;
    int damageToCreature;
    bool isProvoked;
    MTGCardInstance * ProvokeTarget;
    MTGCardInstance * Provoker;
    bool mPropertiesChangedSinceLastUpdate;
    int reduxamount;
    int flanked;
    int regenerateTokens;
    int isToken;
    int origpower;
    int basepower;//to keep origpower intact
    int pbonus;
    int origtoughness;
    int basetoughness;//to keep origtoughness intact
    int tbonus;
    int isMultiColored;
    int isLeveler;
    bool enchanted;
    int CDenchanted;
    int CDdamaged;
    bool blinked;
    bool isExtraCostTarget;
    bool morphed;
    bool exerted;
    bool turningOver;
    bool isMorphed;
    int isFlipped;
    bool isDefeated;
    string MeldedFrom;
    bool isPhased;
    bool isCascaded;
    int phasedTurn;
    bool handEffects;
    bool graveEffects;
    bool exileEffects;
    bool commandZoneEffects;
    bool suspended;
    bool miracle;
    bool hasCopiedToken;
    bool isBestowed;
    bool isFacedown;
    int chooseacolor;
    string chooseasubtype;
    string chooseaname;
    int coinSide;//1 = tails
    int lastFlipResult;
    int dieSide;
    int lastRollResult;
    int dieNumFaces;
    int scryedCards;

    int stillInUse();
    int didattacked;
    int didblocked;
    int notblocked;
    int fresh;
    int MaxLevelUp;
    int CountedObjects;
    int CountedObjectsB;
    int kicked;
    int dredge;
    int zpos;
    bool isDualWielding;
    bool stillNeeded;
    Player * discarderOwner;
    Player * lastController;
    Player * previousController;
    MTGGameZone * getCurrentZone();
    MTGGameZone * previousZone;
    MTGCardInstance * tokCard;
    MTGCardInstance * previous;
    MTGCardInstance * next;
    MTGCardInstance * auraParent;
    MTGAbility * TokenAndAbility;
    MTGAbility * GrantedAndAbility;
    int doDamageTest;
    bool skipDamageTestOnce;
    int summoningSickness;
    ManaCost reducedCost;
    ManaCost increasedCost;
    ManaCost producedMana;
    ManaCost * getReducedManaCost();
    ManaCost * getIncreasedManaCost();
    ManaCost * getProducedMana();
    bool matchesCastFilter(int castMethod);
    bool hasTotemArmor();

    // The recommended method to test for summoning Sickness !
    int hasSummoningSickness();
    MTGCardInstance * changeController(Player * newcontroller,bool notZone = false);
    Player * owner;
    Counters * counters;
    const string getDisplayName() const;
    MTGCardInstance * target;
    Player * playerTarget;
    vector<Targetable *> backupTargets;


    //types
    void addType(const string& type_text);
    virtual void addType(int id);
    void setType(const string& type_text);
    void setSubtype(const string &value);
    int removeType(const string &value, int removeAll = 0);
    int removeType(int value, int removeAll = 0);

    //dangerranking is a hint to Ai which creatures are the ones it should be targetting for effects.
    int DangerRanking();
    //Combat
    bool isBlocked() {return blocked;}; //Blocked this turn or not?
    MTGCardInstance * defenser;
    list<MTGCardInstance *>blockers;
    int attacker;
    int willattackplayer;
    int willattackpw;
    int toggleDefenser(MTGCardInstance * opponent);
    int raiseBlockerRankOrder(MTGCardInstance * blocker);

    //Returns rank of the card in blockers if it is a blocker of this (starting at 1), 0 otherwise
    int getDefenserRank(MTGCardInstance * blocker);
    int toggleAttacker(bool pw = false);
    MTGCardInstance * banding; // If belongs to a band when attacking
    int canBlock();
    int canBlock(MTGCardInstance * opponent);
    int canAttack( bool pwcheck = false );
    int isAttacker();
    Targetable * isAttacking;
    MTGCardInstance * storedCard;
    MTGCardInstance * myPair;
    MTGCardInstance * createSnapShot();
    MTGCardInstance * storedSourceCard;
    MTGCardInstance * shackled;
    MTGCardInstance * seized;
    MTGCardInstance * isDefenser();
    int initAttackersDefensers();
    MTGCardInstance * getNextOpponent(MTGCardInstance * previous=NULL);
    int nbOpponents();
    int stepPower(CombatStep step);
    int afterDamage();
    int has(int ability);
    int getToxicity();
    int cleanup();

    MTGCard * model;
    MTGCardInstance();
    MTGCardInstance(MTGCard * card, MTGPlayerCards * _belongs_to);
    int regenerate();
    int triggerRegenerate();
    Player * controller();

    virtual ~MTGCardInstance();
    int totem( bool noregen = false );
    int toGrave( bool forced = false );
    int destroy();
    int destroyNoRegen();

    int addToToughness(int value);
    int setToughness(int value);
    int isSettingBase;

    vector<TargetChooser *>protections;
    int addProtection(TargetChooser * tc);
    int removeProtection(TargetChooser *tc, int erase = 0);
    int protectedAgainst(MTGCardInstance * card);

    vector<TargetChooser *>canttarget;
    int addCantBeTarget(TargetChooser * tc);
    int removeCantBeTarget(TargetChooser *tc, int erase = 0);
    int CantBeTargetby(MTGCardInstance * card);

    vector<TargetChooser *>cantBeBlockedBys;
    vector<TargetChooser *>cantBeBlockerOfs;
    vector<MTGCardInstance *>cantBeBlockerOfCards;
    int addCantBeBlockedBy(TargetChooser * tc);
    int removeCantBeBlockedBy(TargetChooser *tc, int erase = 0);
    int cantBeBlockedBy(MTGCardInstance * card);
    int addCantBeBlockerOf(TargetChooser * tc);
    int removeCantBeBlockerOf(TargetChooser *tc, int erase = 0);
    int cantBeBlockerOf(MTGCardInstance * card);
    int addCantBeBlockerOfCard(MTGCardInstance * card);
    int removeCantBeBlockerOfCard(MTGCardInstance * card, int erase = 0);
    int cantBeBlockerOfCard(MTGCardInstance * card);

    void copy(MTGCardInstance * card, bool nolegend = false);

    void setUntapping();
    void resetUntapping(); // Fix to avoid the untap on frozen card by clicking on them after the untap phase.
    int isUntapping();
    int isTapped();
    void untap();
    void tap(bool sendNoEvent = false);
    void attemptUntap();

    //cda and other func
    void stripPTbonus();
    void minusPTbonus(int p = 0, int t = 0);
    void plusPTbonus(int p = 0, int t = 0);
    void applyPTbonus();
    void addcounter(int p = 0, int t = 0);
    void removecounter(int p = 0, int t = 0);
    void addptbonus(int p = 0, int t = 0);
    void removeptbonus(int p = 0, int t = 0);
    void addbaseP(int p = 0);
    void addbaseT(int t = 0);
    void revertbaseP();
    void revertbaseT();
    int getCurrentPower();
    int getCurrentToughness();
    int LKIpower;
    int LKItoughness;
    int countDuplicateCardNames();
    int countDuplicateCardTypes();
    void cdaPT(int p = 0, int t = 0);
    bool isCDA;
    void switchPT(bool apply = false);
    int swapP;
    int swapT;
    bool isSwitchedPT;
    bool isACopier;
    bool bypassTC;
    bool discarded;
    int copiedID;
    int copiedSetID;
    bool canPlayFromLibrary();
    bool StackIsEmptyandSorcerySpeed();
    int forcedBorderA;
    int forcedBorderB;
    int myconvertedcost;
    ManaCost * computeNewCost(MTGCardInstance * card,ManaCost * oldCost, ManaCost * refCost,bool noTrinisphere = false, bool bestow = false);
    int countTrini;
    bool anymanareplacement;
    vector<MTGCardInstance*> imprintedCards;
    MTGCardInstance* hauntedCard;
    int attackCost;
    int attackCostBackup;
    int attackPlaneswalkerCost;
    int attackPlaneswalkerCostBackup;
    int blockCost;
    int blockCostBackup;
    int imprintG;
    int imprintU;
    int imprintR;
    int imprintB;
    int imprintW;
    int foretellTurn;
    int bushidoPoints;
    int modularPoints;
    int canproduceMana(int color = -1);
    int entersBattlefield;
    string currentimprintName;
    vector<string>imprintedNames;

    MTGCardInstance * revealedLast;//last card revealed by a ability this card owns.
    bool MadnessPlay;
    void eventattacked();
    void eventattackedAlone();
    void eventattackednotblocked();
    void eventattackedblocked(MTGCardInstance * opponent);
    void eventblocked(MTGCardInstance * opponent);

    int isInPlay(GameObserver* game);
    const string& getSample();

    JQuadPtr getIcon();

    ostream& toString(ostream&) const;

    static MTGCardInstance AnyCard;
    static MTGCardInstance NoCard;

    bool parseLine(const string& ss);
    virtual MTGCardInstance* clone();
};


#endif
