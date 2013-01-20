#ifndef _REPLACEMENT_EFFECTS_H_
#define _REPLACEMENT_EFFECTS_H_

#include <list>
using namespace std;
#include "Damage.h"
#include "WEvent.h"
#include "Counters.h"

class TargetChooser;
class MTGAbility;

class ReplacementEffect
{
public:
    virtual WEvent * replace(WEvent * e)
    {
        return e;
    }
    ;
    virtual ~ReplacementEffect() {}
};

class REDamagePrevention: public ReplacementEffect
{
protected:
    MTGAbility * source;
    TargetChooser * tcSource;
    TargetChooser * tcTarget;
    int damage;
    bool oneShot;
    int typeOfDamage;
public:
    REDamagePrevention(MTGAbility * _source, TargetChooser *_tcSource = NULL, TargetChooser *_tcTarget = NULL, int _damage = -1, bool _oneShot = true, int typeOfDamage = DAMAGE_ALL_TYPES);
    WEvent * replace(WEvent *e);
    ~REDamagePrevention();
};

class RECountersPrevention: public ReplacementEffect
{
protected:
    MTGAbility * source;
    MTGCardInstance * cardSource;
    MTGCardInstance * cardTarget;
    TargetChooser * TargetingCards;
    Counter * counter;
public:
    RECountersPrevention(MTGAbility * _source,MTGCardInstance * cardSource = NULL,MTGCardInstance * cardTarget = NULL,TargetChooser * tc = NULL,Counter * counter = NULL);
    WEvent * replace(WEvent *e);
    ~RECountersPrevention();
};
class REDrawReplacement: public ReplacementEffect
{
protected:
    MTGAbility * source;

public:
    Player * DrawerOfCard;
    MTGAbility * replacementAbility;
    REDrawReplacement(MTGAbility * _source, Player * Drawer = NULL, MTGAbility * replaceWith = NULL);
    WEvent * replace(WEvent *e);
    ~REDrawReplacement();
};
class ReplacementEffects
{
public:
    list<ReplacementEffect *> modifiers;
    ReplacementEffects();
    WEvent * replace(WEvent *e);
    int add(ReplacementEffect * re);
    int remove(ReplacementEffect * re);
    ~ReplacementEffects();
};

#endif
