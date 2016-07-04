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
    int auras;
    bool wasDealtDamage;
    bool damageToOpponent;
    bool damageToController;
    bool damageToCreature;
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
    bool turningOver;
    bool isMorphed;
    bool isFlipped;
    bool isPhased;
    bool isCascaded;
    int phasedTurn;
    bool graveEffects;
    bool exileEffects;
    bool suspended;
    bool miracle;
	bool isBestowed;
    int chooseacolor;
    string chooseasubtype;
    int coinSide;//1 = tails
        
    int stillInUse();
    int didattacked;
    int didblocked;
    int notblocked;
    int fresh;
    int MaxLevelUp;
	int CountedObjects;
    int kicked;
    int dredge;
    bool isDualWielding;
    bool stillNeeded;
    Player * lastController;
    MTGGameZone * getCurrentZone();
    MTGGameZone * previousZone;
    MTGCardInstance * previous;
    MTGCardInstance * next;
    int doDamageTest;
    bool skipDamageTestOnce;
    int summoningSickness;
    ManaCost reducedCost;
    ManaCost increasedCost;
    ManaCost * getReducedManaCost();
    ManaCost * getIncreasedManaCost();
    bool matchesCastFilter(int castMethod);

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
    int toggleDefenser(MTGCardInstance * opponent);
    int raiseBlockerRankOrder(MTGCardInstance * blocker);

    //Returns rank of the card in blockers if it is a blocker of this (starting at 1), 0 otherwise
    int getDefenserRank(MTGCardInstance * blocker);
    int toggleAttacker();
    MTGCardInstance * banding; // If belongs to a band when attacking
    int canBlock();
    int canBlock(MTGCardInstance * opponent);
    int canAttack();
    int isAttacker();
    Targetable * isAttacking;
    MTGCardInstance * storedCard;
    MTGCardInstance * myPair;
    MTGCardInstance * createSnapShot();
    MTGCardInstance * storedSourceCard;
    MTGCardInstance * isDefenser();
    int initAttackersDefensers();
    MTGCardInstance * getNextOpponent(MTGCardInstance * previous=NULL);
    int nbOpponents();
    int stepPower(CombatStep step);
    int afterDamage();
    int has(int ability);
    int cleanup();

    MTGCard * model;
    MTGCardInstance();
    MTGCardInstance(MTGCard * card, MTGPlayerCards * _belongs_to);
    int regenerate();
    int triggerRegenerate();
    Player * controller();

    virtual ~MTGCardInstance();
    int bury();
    int destroy();

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

    void copy(MTGCardInstance * card);

    void setUntapping();
    int isUntapping();
    int isTapped();
    void untap();
    void tap();
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
    int modifiedbAbi;
    bool StackIsEmptyandSorcerySpeed();
    bool isTargetted();
    int cardistargetted;
    bool isTargetter();
    int cardistargetter;
    int myconvertedcost;
    ManaCost * computeNewCost(MTGCardInstance * card,ManaCost * oldCost, ManaCost * refCost,bool noTrinisphere = false);
    int countTrini;
    vector<MTGCardInstance*>imprintedCards;
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
