/* Default observers/Abilities that are added to the game for a standard Magic Game
 */

#ifndef _MTGRULES_H_
#define _MTGRULES_H_

#include "MTGAbility.h"
#include "Counters.h"
#include "WEvent.h"
#include "CardSelector.h"
#include "ManaCost.h"

class PermanentAbility: public MTGAbility
{
public:
    int testDestroy() {return 0;};
    PermanentAbility(GameObserver* observer, int _id);
};

class OtherAbilitiesEventReceiver: public PermanentAbility
{
public:
    int receiveEvent(WEvent * event);
    OtherAbilitiesEventReceiver(GameObserver* observer, int _id);
    OtherAbilitiesEventReceiver * clone() const;
};

class MTGEventBonus: public PermanentAbility
{
public:
    int textAlpha;
    string text;
    int army[2];
    bool army1[2];
    bool army2[2];
    bool army3[2];
    int toys[2];
    bool toybonusgranted[2];
    int chain[2];
    int highestChain[2];
    bool beastbonusgranted[2];
    int beast[2];
    bool zombiebonusgranted[2];
    int zombie[2];
    bool knightbonusgranted[2];
    int knight[2];
    bool insectbonusgranted[2];
    int insect[2];
    bool elementalbonusgranted[2];
    int elemental[2];
    bool vampirebonusgranted[2];
    int vampire[2];
    bool clericbonusgranted[2];
    int cleric[2];
    bool elfbonusgranted[2];
    int elf[2];
    bool Angelbonusgranted[2];
    int Angel[2];
    bool dragonbonusgranted[2];
    int dragon[2];
    bool eldrazibonusgranted[2];
    int eldrazi[2];
    bool werewolfbonusgranted[2];
    int werewolf[2];

    int receiveEvent(WEvent * event);
    void grantAward(string awardName,int amount);
    void Update(float dt);
    void Render();
    MTGEventBonus(GameObserver* observer, int _id);
    virtual MTGEventBonus * clone() const;
};

class MTGPutInPlayRule: public PermanentAbility
{
protected:
    string defaultPlayName;
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGPutInPlayRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        if(defaultPlayName.size())
            return defaultPlayName.c_str();
        return "Cast Card Normally";
    }
    virtual MTGPutInPlayRule * clone() const;
};

class MTGKickerRule: public MTGPutInPlayRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGKickerRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Pay Kicker";
    }
    virtual MTGKickerRule * clone() const;
};

class MTGAlternativeCostRule: public PermanentAbility
{
protected:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana, ManaCost *alternateManaCost);
    int reactToClick(MTGCardInstance * card, ManaCost * alternateManaCost, int paymentType = ManaCost::MANA_PAID, bool overload = false);
    string alternativeName;
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGAlternativeCostRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        if(alternativeName.size())
            return alternativeName.c_str();
        return "Pay Alternative Cost";
    }
    virtual MTGAlternativeCostRule * clone() const;
};

class MTGBuyBackRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGBuyBackRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Cast and Buyback";
    }
    virtual MTGBuyBackRule * clone() const;
};

class MTGFlashBackRule: public MTGAlternativeCostRule
{
public:

    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGFlashBackRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Flashback";
    }
    virtual MTGFlashBackRule * clone() const;
};

class MTGTempFlashBackRule: public MTGAlternativeCostRule
{
public:

    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGTempFlashBackRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Flashback Manacost";
    }
    virtual MTGTempFlashBackRule * clone() const;
};

class MTGRetraceRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGRetraceRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Retrace";
    }
    virtual MTGRetraceRule * clone() const;
};

class MTGMorphCostRule: public PermanentAbility
{
public:
 
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGMorphCostRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Play Morphed";
    }
    virtual MTGMorphCostRule * clone() const;
};

class MTGPayZeroRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    string CustomName;
    virtual ostream& toString(ostream& out) const;
    MTGPayZeroRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        if(CustomName.size())
            return CustomName.c_str();
        return "Pay Zero To Cast";
    }
    virtual MTGPayZeroRule * clone() const;
};

class MTGOverloadRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGOverloadRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Overload";
    }
    virtual MTGOverloadRule * clone() const;
};

class MTGBestowRule : public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGBestowRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Bestow";
    }
    virtual MTGBestowRule * clone() const;
};


class MTGSuspendRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int receiveEvent(WEvent *e);
    int reactToClick(MTGCardInstance * card);
    string suspendmenu;
    virtual ostream& toString(ostream& out) const;
    MTGSuspendRule(GameObserver* observer, int _id);
    const string getMenuText();
    virtual MTGSuspendRule * clone() const;
};

class MTGAttackCostRule: public PermanentAbility
{
public:
    string scost;
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGAttackCostRule(GameObserver* observer, int _id);
    const string getMenuText();
    virtual MTGAttackCostRule * clone() const;
};

class MTGAttackRule: public PermanentAbility, public Limitor
{
public:
 
    virtual bool select(Target*);
    virtual bool greyout(Target*);
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGAttackRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Attacker";
    }
    int receiveEvent(WEvent * event);
    virtual MTGAttackRule * clone() const;
};


class MTGPlaneswalkerAttackRule: public PermanentAbility, public Limitor
{
public:
 
    virtual bool select(Target*);
    virtual bool greyout(Target*);
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    MTGPlaneswalkerAttackRule(GameObserver* observer, int _id);
    const string getMenuText()
    {
        return "Attack Planeswalker";
    }
    virtual MTGPlaneswalkerAttackRule * clone() const;
};
class AAPlaneswalkerAttacked: public InstantAbility
{
public:
    string menuText;
    MTGCardInstance* attacker;
    AAPlaneswalkerAttacked(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target);
    int resolve();
    const string getMenuText();
    AAPlaneswalkerAttacked * clone() const;
    ~AAPlaneswalkerAttacked();
};
/* handles combat trigger send recieve events*/
class MTGCombatTriggersRule: public PermanentAbility
{
public:
    MTGCombatTriggersRule(GameObserver* observer, int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    virtual MTGCombatTriggersRule * clone() const;
};

class MTGBlockCostRule: public PermanentAbility
{
public:
    string scost;
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGBlockCostRule(GameObserver* observer, int _id);
    const string getMenuText();
    virtual MTGBlockCostRule * clone() const;
};

class MTGBlockRule: public PermanentAbility
{
public:
    string blockmenu;
    TargetChooser * tcb;
    MTGAbility * blocker;
    MTGAbility * blockAbility;
    int receiveEvent(WEvent * event);
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    virtual ostream& toString(ostream& out) const;
    MTGBlockRule(GameObserver* observer, int _id);
    const string getMenuText();
    virtual MTGBlockRule * clone() const;
    ~MTGBlockRule();
};
//soulbond rule
class MTGSoulbondRule: public PermanentAbility
{
public:
    vector<MTGCardInstance*>soulbonders;
    TargetChooser * tcb;
    MTGAbility * pairAbility;
    MTGAbility * targetAbility;
    MTGAbility * targetAbility1;
    MTGAbility * mod;
    MTGAbility * activatePairing;
    vector<MTGAbility*>pairing;
    MTGSoulbondRule(GameObserver* observer, int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    virtual MTGSoulbondRule * clone() const;
};
/*dredge*/
class MTGDredgeRule: public PermanentAbility, public ReplacementEffect
{
public:
    //vector<MTGCardInstance*>soulbonders;
    TargetChooser * tcb;
    MTGAbility * dredgeAbility;
    MTGAbility * targetAbility;
    MTGAbility * targetAbilityAdder;
    MTGAbility * targetAbility1;
    MTGAbility * mod;
    //MTGAbility * activateDredge;
    //vector<MTGAbility*>pairing;
    MTGDredgeRule(GameObserver* observer, int _id);
    WEvent * replace(WEvent *e);
    virtual ostream& toString(ostream& out) const;
    virtual MTGDredgeRule * clone() const;
};
/* Persist Rule */
class MTGPersistRule: public PermanentAbility
{
public:
    MTGPersistRule(GameObserver* observer, int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    virtual MTGPersistRule * clone() const;
};
/* vampire Rule */
class MTGVampireRule: public PermanentAbility
{
public:
    MTGVampireRule(GameObserver* observer, int _id);
    map<MTGCardInstance*,vector<MTGCardInstance*> > victims;
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    virtual MTGVampireRule * clone() const;
};
//unearths destruction if leaves play effect
class MTGUnearthRule: public PermanentAbility
{
public:
    MTGUnearthRule(GameObserver* observer, int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    virtual MTGUnearthRule * clone() const;
};
class MTGTokensCleanup: public PermanentAbility
{
public:
    vector<MTGCardInstance *> list;
    MTGTokensCleanup(GameObserver* observer, int _id);
    int receiveEvent(WEvent * event);
    virtual MTGTokensCleanup * clone() const;
};

/*
 * 704.5k If a player controls two or more legendary permanents with the same name, 
 * that player chooses one of them, and the rest are put into their owners’ graveyards. 
 * This is called the “legend rule.” 
 */
class MTGLegendRule: public ListMaintainerAbility
{
public:
    TargetChooser * tcL;
    MTGAbility * Legendrule;
    MTGAbility * LegendruleAbility;
    MTGAbility * LegendruleGeneric;
    MTGLegendRule(GameObserver* observer, int _id);
    int canBeInList(MTGCardInstance * card);
    int added(MTGCardInstance * card);
    int removed(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    virtual MTGLegendRule * clone() const;
};
class MTGPlaneWalkerRule: public ListMaintainerAbility
{
public:
    TargetChooser * tcP;
    MTGAbility * PWrule;
    MTGAbility * PWruleAbility;
    MTGAbility * PWruleGeneric;
    MTGPlaneWalkerRule(GameObserver* observer, int _id);
    int canBeInList(MTGCardInstance * card);
    int added(MTGCardInstance * card);
    int removed(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    virtual MTGPlaneWalkerRule * clone() const;
};
/* LifeLink */
class MTGPlaneswalkerDamage: public PermanentAbility
{
public:
    MTGPlaneswalkerDamage(GameObserver* observer, int _id);

    int receiveEvent(WEvent * event);

    virtual MTGPlaneswalkerDamage * clone() const;
};
class MTGMomirRule: public PermanentAbility
{
private:
    int genRandomCreatureId(int convertedCost);
    vector<int> pool[20];
    int initialized;

    int textAlpha;
    string text;
public:

    int alreadyplayed;
    MTGAllCards * collection;
    MTGCardInstance * genCreature(int id);
    void Update(float dt);
    void Render();
    MTGMomirRule(GameObserver* observer, int _id, MTGAllCards * _collection);
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int reactToClick(MTGCardInstance * card, int id);
    const string getMenuText()
    {
        return "Momir";
    }
    virtual ostream& toString(ostream& out) const;
    virtual MTGMomirRule * clone() const;
};

//stone hewer gaint avatar mode
class MTGStoneHewerRule: public PermanentAbility
{
private:
    int genRandomEquipId(int convertedCost);
    vector<int> pool[20];
    int initialized;
public:
    MTGAllCards * collection;
    MTGCardInstance * genEquip(int id);
    MTGStoneHewerRule(GameObserver* observer, int _id, MTGAllCards * _collection);
    int receiveEvent(WEvent * event);
    const string getMenuText()
    {
        return "Stone Hewer";
    }
    virtual ostream& toString(ostream& out) const;
    virtual MTGStoneHewerRule * clone() const;
};
//Hermit Druid avatar mode
class MTGHermitRule: public PermanentAbility
{
public:
    MTGHermitRule(GameObserver* observer, int _id);
    int receiveEvent(WEvent * event);
    const string getMenuText()
    {
        return "Hermit";
    }
    virtual MTGHermitRule * clone() const;
};
//
/* LifeLink */
class MTGLifelinkRule: public PermanentAbility
{
public:
    MTGLifelinkRule(GameObserver* observer, int _id);

    int receiveEvent(WEvent * event);

    virtual ostream& toString(ostream& out) const;

    virtual MTGLifelinkRule * clone() const;
};

/* Deathtouch */
class MTGDeathtouchRule: public PermanentAbility
{
public:
    MTGDeathtouchRule(GameObserver* observer, int _id);

    int receiveEvent(WEvent * event);

    const string getMenuText()
    {
        return "Deathtouch";
    }

    virtual MTGDeathtouchRule * clone() const;
};

/* handling parentchild */
class ParentChildRule: public PermanentAbility
{
public:
    ParentChildRule(GameObserver* observer, int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    virtual ParentChildRule * clone() const;
};
/* HUD Display */

class HUDString
{
public:
    string value;
    float timestamp;
    int quantity;
    HUDString(string s, float ts) :
        value(s), timestamp(ts)
    {
        quantity = 1;
    }
    ;
};

class HUDDisplay: public PermanentAbility
{
private:
    list<HUDString *> events;
    float timestamp;
    float popdelay;
    WFont * f;
    float maxWidth;
    int addEvent(string s);
public:
    int receiveEvent(WEvent * event);
    void Update(float dt);
    void Render();
    HUDDisplay(GameObserver* observer, int _id);
    ~HUDDisplay();
    virtual HUDDisplay * clone() const;
};

#endif
