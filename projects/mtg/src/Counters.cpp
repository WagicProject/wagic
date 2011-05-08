#include "PrecompiledHeader.h"

#include "Counters.h"
#include "MTGCardInstance.h"

Counter::Counter(MTGCardInstance * _target, int _power, int _toughness)
{
    init(_target, "", _power, _toughness);
}

Counter::Counter(MTGCardInstance * _target, const char * _name, int _power, int _toughness)
{
    init(_target, _name, _power, _toughness);
}

int Counter::init(MTGCardInstance * _target, const char * _name, int _power, int _toughness)
{
    target = _target;
    name = _name;
    power = _power;
    toughness = _toughness;
    nb = 1;
    return 1;
}

bool Counter::sameAs(const char * _name, int _power, int _toughness)
{
    if (power == 0 && toughness == 0)
        return (name.compare(_name) == 0);
    return (power == _power && toughness == _toughness);
}

bool Counter::cancels(int _power, int _toughness)
{
    if (power == 0 && toughness == 0)
        return false;
    return (power == -_power && toughness == -_toughness);
}

int Counter::added()
{
    if (power != 0 || toughness != 0)
    {
        target->power += power;
        target->addToToughness(toughness);
    }
    return 1;
}

int Counter::removed()
{
    if (power != 0 || toughness != 0)
    {
        target->power -= power;
        target->addToToughness(-toughness);
    }
    return 1;
}

Counters::Counters(MTGCardInstance * _target) :
    target(_target)
{
    mCount = 0;
}
Counters::~Counters()
{
    for (int i = 0; i < mCount; i++)
    {
        SAFE_DELETE(counters[i]);
    }
}

int Counters::addCounter(const char * _name, int _power, int _toughness)
{
/*420.5n If a permanent has both a +1/+1 counter and a -1/-1 counter on it, N +1/+1 and N -1/-1 counters are removed from it, where N is the smaller of the number of +1/+1 and -1/-1 counters on it.*/
    for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->cancels(_power, _toughness) && !counters[i]->name.size() && counters[i]->nb > 0)
        {
            counters[i]->removed();
            counters[i]->nb--;
            return mCount;
        }
    }
    for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->sameAs(_name, _power, _toughness))
        {
            counters[i]->added();
            counters[i]->nb++;
            return mCount;
        }
    }
    Counter * counter = NEW Counter(target, _name, _power, _toughness);
    counters[mCount] = counter;
    counter->added();
    mCount++;
    return mCount;
}

int Counters::addCounter(int _power, int _toughness)
{
    return addCounter("", _power, _toughness);
}

int Counters::init()
{
    for (int i = mCount - 1; i >= 0; i--)
    {
        while (counters[i]->nb >= 1)
        {
            counters[i]->removed();
            counters[i]->nb--;
        }
    }
    return 1;
}

int Counters::removeCounter(const char * _name, int _power, int _toughness)
{
    for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->sameAs(_name, _power, _toughness))
        {
            if (counters[i]->nb < 1)
                return 0;
            counters[i]->removed();
            counters[i]->nb--;
            //special case:if a card is suspended and no longer has a time counter when the last is removed, the card is cast.
            if (target->suspended && !target->counters->hasCounter("time",0,0))
            {
                GameObserver * game = game->GetInstance();
                MTGCardInstance * copy = target->controller()->game->putInZone(target, target->currentZone, target->controller()->game->stack);
                
                game->mLayers->stackLayer()->addSpell(copy, game->targetChooser, NULL,1, 0);
                game->targetChooser = NULL;
            }
            return mCount;
        }
    }
    return 0;
}

int Counters::removeCounter(int _power, int _toughness)
{
    return removeCounter("", _power, _toughness);
}

Counter * Counters::hasCounter(const char * _name, int _power, int _toughness)
{
    for (int i = 0; i < mCount; i++)
    {
        if (counters[i]->sameAs(_name, _power, _toughness))
        {
            if (counters[i]->nb > 0)
                return counters[i];
        }
    }
    return NULL;
}

Counter * Counters::hasCounter(int _power, int _toughness)
{
    return hasCounter("", _power, _toughness);
}

Counter * Counters::getNext(Counter * previous)
{
    int found = 0;
    for (int i = 0; i < mCount; i++)
    {
        if (found && counters[i]->nb > 0)
            return counters[i];
        if (counters[i] == previous)
            found = 1;
    }
    return NULL;
}
