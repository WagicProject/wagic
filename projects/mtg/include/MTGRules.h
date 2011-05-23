/* Default observers/Abilities that are added to the game for a standard Magic Game
 */

#ifndef _MTGRULES_H_
#define _MTGRULES_H_

#include "MTGAbility.h"
#include "Counters.h"
#include "WEvent.h"
#include "CardSelector.h"
#include "ManaCost.h"

class OtherAbilitiesEventReceiver: public MTGAbility
{
public:
    int testDestroy();
    int receiveEvent(WEvent * event);
    OtherAbilitiesEventReceiver(int _id);
    OtherAbilitiesEventReceiver * clone() const;
};

class MTGEventBonus: public MTGAbility
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

    int receiveEvent(WEvent * event);
    void grantAward(string awardName,int amount);
    int testDestroy();
    void Update(float dt);
    void Render();
    MTGEventBonus(int _id);
    virtual MTGEventBonus * clone() const;
};

class MTGPutInPlayRule: public MTGAbility
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGPutInPlayRule(int _id);
    const char * getMenuText()
    {
        return "cast card normally";
    }
    virtual MTGPutInPlayRule * clone() const;
};

class MTGKickerRule: public MTGPutInPlayRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGKickerRule(int _id);
    const char * getMenuText()
    {
        return "pay kicker";
    }
    virtual MTGKickerRule * clone() const;
};

class MTGAlternativeCostRule: public MTGAbility
{
protected:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana, ManaCost *alternateManaCost);
    int reactToClick(MTGCardInstance * card, ManaCost * alternateManaCost, int paymentType = ManaCost::MANA_PAID);
    string alternativeName;
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);

    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGAlternativeCostRule(int _id);
    const char * getMenuText()
    {
        if(alternativeName.size())
            return alternativeName.c_str();
        return "pay alternative cost";
    }
    virtual MTGAlternativeCostRule * clone() const;
};

class MTGBuyBackRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGBuyBackRule(int _id);
    const char * getMenuText()
    {
        return "cast and buy back";
    }
    virtual MTGBuyBackRule * clone() const;
};

class MTGFlashBackRule: public MTGAlternativeCostRule
{
public:

    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGFlashBackRule(int _id);
    const char * getMenuText()
    {
        return "flash back";
    }
    virtual MTGFlashBackRule * clone() const;
};

class MTGRetraceRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGRetraceRule(int _id);
    const char * getMenuText()
    {
        return "retrace";
    }
    virtual MTGRetraceRule * clone() const;
};

class MTGMorphCostRule: public MTGAbility
{
public:
 
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGMorphCostRule(int _id);
    const char * getMenuText()
    {
        return "play morphed";
    }
    virtual MTGMorphCostRule * clone() const;
};

class MTGSuspendRule: public MTGAlternativeCostRule
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int receiveEvent(WEvent *e);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    string suspendmenu;
    virtual ostream& toString(ostream& out) const;
    MTGSuspendRule(int _id);
    const char * getMenuText()
    {
        suspendmenu = "suspend";
        //char buffer[20];
        //sprintf(buffer,"-%i",card->suspendedTime);
        //suspendmenu.append(buffer);
        //TODO:make this work so it shows "Suspend-the amount of turns"
        return suspendmenu.c_str();
    }
    virtual MTGSuspendRule * clone() const;
};

class MTGAttackRule: public MTGAbility, public Limitor
{
public:
 
    virtual bool select(Target*);
    virtual bool greyout(Target*);
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGAttackRule(int _id);
    const char * getMenuText()
    {
        return "Attacker";
    }
    int receiveEvent(WEvent * event);
    virtual MTGAttackRule * clone() const;
};

/* handles combat trigger send recieve events*/
class MTGCombatTriggersRule: public MTGAbility
{
public:
    MTGCombatTriggersRule(int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    int testDestroy();
    virtual MTGCombatTriggersRule * clone() const;
};

class MTGBlockRule: public MTGAbility
{
public:
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    MTGBlockRule(int _id);
    const char * getMenuText()
    {
        return "Blocker";
    }
    virtual MTGBlockRule * clone() const;
};

/* Persist Rule */
class MTGPersistRule: public MTGAbility
{
public:
    MTGPersistRule(int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    int testDestroy();
    virtual MTGPersistRule * clone() const;
};
/* vampire Rule */
class MTGVampireRule: public MTGAbility
{
public:
    MTGVampireRule(int _id);
    map<MTGCardInstance*,vector<MTGCardInstance*> > victems;
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    int testDestroy();
    virtual MTGVampireRule * clone() const;
};
//unearths destruction if leaves play effect
class MTGUnearthRule: public MTGAbility
{
public:
    MTGUnearthRule(int _id);
    int receiveEvent(WEvent * event);
    virtual ostream& toString(ostream& out) const;
    int testDestroy();
    virtual MTGUnearthRule * clone() const;
};
class MTGTokensCleanup: public MTGAbility
{
public:
    vector<MTGCardInstance *> list;
    MTGTokensCleanup(int _id);
    int receiveEvent(WEvent * event);
    int testDestroy();
    virtual MTGTokensCleanup * clone() const;
};

/*
 * Rule 420.5e (Legend Rule)
 * If two or more legendary permanents with the same name are in play, all are put into their
 * owners' graveyards. This is called the "legend rule." If only one of those permanents is
 * legendary, this rule doesn't apply.
 */
class MTGLegendRule: public ListMaintainerAbility
{
public:
    MTGLegendRule(int _id);
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
    MTGPlaneWalkerRule(int _id);
    int canBeInList(MTGCardInstance * card);
    int added(MTGCardInstance * card);
    int removed(MTGCardInstance * card);
    int testDestroy();
    virtual ostream& toString(ostream& out) const;
    virtual MTGPlaneWalkerRule * clone() const;
};

class MTGMomirRule: public MTGAbility
{
private:
    int genRandomCreatureId(int convertedCost);
    static vector<int> pool[20];
    static int initialized;

    int textAlpha;
    string text;
public:

    int alreadyplayed;
    MTGAllCards * collection;
    MTGCardInstance * genCreature(int id);
    int testDestroy();
    void Update(float dt);
    void Render();
    MTGMomirRule(int _id, MTGAllCards * _collection);
    int isReactingToClick(MTGCardInstance * card, ManaCost * mana = NULL);
    int reactToClick(MTGCardInstance * card);
    int reactToClick(MTGCardInstance * card, int id);
    const char * getMenuText()
    {
        return "Momir";
    }
    virtual ostream& toString(ostream& out) const;
    virtual MTGMomirRule * clone() const;
};

//stone hewer gaint avatar mode
class MTGStoneHewerRule: public MTGAbility
{
private:
    int genRandomEquipId(int convertedCost);
    static vector<int> pool[20];
    static int initialized;

    int textAlpha;
    string text;
public:

	int alreadyplayed;
	MTGAllCards * collection;
	MTGCardInstance * genEquip(int id);
	int testDestroy();
	void Update(float dt);
	void Render();
	MTGStoneHewerRule(int _id, MTGAllCards * _collection);
	int receiveEvent(WEvent * event);
	const char * getMenuText()
	{
		return "Stone Hewer";
	}
	virtual ostream& toString(ostream& out) const;
	virtual MTGStoneHewerRule * clone() const;
};
//Hermit Druid avatar mode
class MTGHermitRule: public MTGAbility
{
public:
	int testDestroy();
	MTGHermitRule(int _id);
	int receiveEvent(WEvent * event);
	const char * getMenuText()
	{
		return "Hermit";
	}
	virtual MTGHermitRule * clone() const;
};
//
/* LifeLink */
class MTGLifelinkRule: public MTGAbility
{
public:
    MTGLifelinkRule(int _id);

    int receiveEvent(WEvent * event);

    int testDestroy();

    virtual ostream& toString(ostream& out) const;

    virtual MTGLifelinkRule * clone() const;
};

/* Deathtouch */
class MTGDeathtouchRule: public MTGAbility
{
public:
    MTGDeathtouchRule(int _id);

    int receiveEvent(WEvent * event);

    int testDestroy();
    const char * getMenuText()
    {
        return "Deathtouch";
    }

    virtual MTGDeathtouchRule * clone() const;
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

class HUDDisplay: public MTGAbility
{
private:
    list<HUDString *> events;
    float timestamp;
    float popdelay;
    WFont * f;
    float maxWidth;
    int addEvent(string s);
public:
    int testDestroy();
    int receiveEvent(WEvent * event);
    void Update(float dt);
    void Render();
    HUDDisplay(int _id);
    ~HUDDisplay();
    virtual HUDDisplay * clone() const;
};

#endif
