#include "PrecompiledHeader.h"

#include "TargetsList.h"
#include "Player.h"
#include "MTGCardInstance.h"
#include "Damage.h"
#include "ActionStack.h"

TargetsList::TargetsList()
{
}

TargetsList::TargetsList(Targetable * _targets[], int nbtargets)
{
    for (int i = 0; i < nbtargets; i++)
        targets.push_back(_targets[i]);
}

int TargetsList::addTarget(Targetable * target)
{
    if (!alreadyHasTarget(target))
    {
        TargetChooser * tc = target->getObserver()->getCurrentTargetChooser();
        if(!tc || (tc && tc->maxtargets == 1))
        {
            //because this was originally coded with targets as an array
            //we have to add this conditional to insure that cards with single target effects
            //and abilities that seek the nextcardtarget still work correctly.
            targets.clear();
            targets.push_back(target);
            return 1;

        }
        else
        {
            targets.push_back(target);
            return 1;
        }
    }
    return 0;
}

int TargetsList::alreadyHasTarget(Targetable * target)
{
    for (size_t i = 0; i < targets.size(); i++)
    {
        if (targets[i] == target) return 1;
    }
    return 0;
}

int TargetsList::removeTarget(Targetable * target)
{
    for (size_t i = 0; i < targets.size(); i++)
    {
        if (targets[i] == target)
        {
            targets.erase(targets.begin() + i);
            return 1;
        }
    }
    return 0;
}

int TargetsList::toggleTarget(Targetable * target)
{
    if (alreadyHasTarget(target))
    {

        return removeTarget(target);
    }
    else
    {

        return addTarget(target);
    }
}

size_t TargetsList::iterateTarget(Targetable * previous){
    if (!previous)
        return 0;

    for (size_t i = 0; i < targets.size(); i++) {
        if (targets[i] == previous)
            return i + 1;
    }

    return targets.size() + 1;

}

Targetable * TargetsList::getNextTarget(Targetable * previous)
{
    size_t nextIndex = iterateTarget(previous);

    if (nextIndex < targets.size())
        return targets[nextIndex];

    return NULL;
}

MTGCardInstance * TargetsList::getNextCardTarget(MTGCardInstance * previous)
{
    size_t nextIndex = iterateTarget(previous);
    for (size_t i = nextIndex; i < targets.size(); ++i)
    {
        if (Spell * spell = dynamic_cast<Spell *>(targets[i]))
        {
            if (MTGCardInstance * c = dynamic_cast<MTGCardInstance *>(spell->source))
                return c;
        }
        if (MTGCardInstance * c = dynamic_cast<MTGCardInstance *>(targets[i]))
            return c;
    }

    return NULL;
}

Player * TargetsList::getNextPlayerTarget(Player * previous)
{
    size_t nextIndex = iterateTarget(previous);
    for (size_t i = nextIndex; i < targets.size(); ++i)
    {
        if (Player * p = dynamic_cast<Player *>(targets[i]))
            return p;
    }

    return NULL;
}

Interruptible * TargetsList::getNextInterruptible(Interruptible * previous, int type)
{
    size_t nextIndex = iterateTarget(previous);

    for (size_t i = nextIndex; i < targets.size(); i++)
    {
        if (Interruptible * action = dynamic_cast<Interruptible *>(targets[i]))
        {
            if (action->type == type)
            {
                return action;
            }
        }
    }
    return NULL;
}

Spell * TargetsList::getNextSpellTarget(Spell * previous)
{
    Spell * spell = (Spell *) getNextInterruptible(previous, ACTION_SPELL);
    return spell;
}

//How about DAMAGESTacks ??
Damage * TargetsList::getNextDamageTarget(Damage * previous)
{
    Damage * damage = (Damage *) getNextInterruptible(previous, ACTION_DAMAGE);
    return damage;
}

Damageable * TargetsList::getNextDamageableTarget(Damageable * previous)
{
    size_t nextIndex = iterateTarget(previous);
    for (size_t i = nextIndex; i < targets.size(); i++)
    {

        if (Player * pTarget = dynamic_cast<Player *>(targets[i]))
        {
            return pTarget;
        }
        else if (MTGCardInstance * cTarget = dynamic_cast<MTGCardInstance *>(targets[i]))
        {
            return cTarget;
        }
    }
    return NULL;
}
