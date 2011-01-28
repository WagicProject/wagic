#ifndef _REPLACEMENT_EFFECTS_H_
#define _REPLACEMENT_EFFECTS_H_

#include <list>
using namespace std;
#include "Damage.h"
#include "WEvent.h"

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

class ReplacementEffects
{
protected:
    list<ReplacementEffect *> modifiers;
public:
    ReplacementEffects();
    WEvent * replace(WEvent *e);
    int add(ReplacementEffect * re);
    int remove(ReplacementEffect * re);
    ~ReplacementEffects();
};

#endif
