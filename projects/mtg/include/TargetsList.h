#ifndef _TARGETSLIST_H_
#define _TARGETSLIST_H_

#define MAX_TARGETS 20

class Targetable;
class MTGCardInstance;
class Player;
class Damageable;
class Spell;
class Interruptible;
class Damage;

class TargetsList
{
public:
    int cursor;
    TargetsList();
    TargetsList(Targetable * _targets[], int nbtargets);
    Targetable* targets[MAX_TARGETS];
    int alreadyHasTarget(Targetable * target);
    int removeTarget(Targetable * _card);
    int toggleTarget(Targetable * _card);
    virtual int addTarget(Targetable * _target);
    MTGCardInstance * getNextCardTarget(MTGCardInstance * previous = 0);
    Player * getNextPlayerTarget(Player * previous = 0);
    Damageable * getNextDamageableTarget(Damageable * previous = 0);
    Interruptible * getNextInterruptible(Interruptible * previous, int type);
    Spell * getNextSpellTarget(Spell * previous = 0);
    Damage * getNextDamageTarget(Damage * previous = 0);
    Targetable * getNextTarget(Targetable * previous = 0, int type = -1);
    void initTargets()
    {
        cursor = 0;
    }
    ;
};

#endif
