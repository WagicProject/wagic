#include "PrecompiledHeader.h"

#include "Counters.h"
#include "MTGCardInstance.h"
#include "AllAbilities.h"

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

int Counter::cancelCounter(int power, int toughness)
{
    while(this->target->counters->hasCounter(power,toughness) && this->target->counters->hasCounter(power*-1,toughness*-1))
    {
        GameObserver *g = this->target->getObserver();
        this->removed();
        this->nb--;
        WEvent * t = NEW WEventCounters(NULL,"",power*-1,toughness*-1,false,true);
        dynamic_cast<WEventCounters*>(t)->targetCard = this->target;
        g->receiveEvent(t);
        this->target->counters->removeCounter(power,toughness);
    }
    return 1;
}

int Counter::added()
{
    if (power != 0 || toughness != 0)
    {
        if(target->isSwitchedPT)
        {
            target->switchPT(false);
            target->addcounter(power, toughness);
            target->switchPT(true);
        }
        else
            target->addcounter(power, toughness);
    }
    return 1;
}

int Counter::removed()
{
    if (power != 0 || toughness != 0)
    {
        if(target->isSwitchedPT)
        {
            target->switchPT(false);
            target->removecounter(power, toughness);
            target->switchPT(true);
        }
        else
        target->removecounter(power, toughness);
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

int Counters::addCounter(const char * _name, int _power, int _toughness, bool _noevent)
{
    /*420.5n If a permanent has both a +1/+1 counter and a -1/-1 counter on it, N +1/+1 and N -1/-1 counters are removed from it, where N is the smaller of the number of +1/+1 and -1/-1 counters on it.*/
    GameObserver *g = target->getObserver();
    WEvent * e = NEW WEventCounters(this,_name,_power,_toughness);
    dynamic_cast<WEventCounters*>(e)->targetCard = this->target;
    if (e == g->replacementEffects->replace(e))
    {
        for (int i = 0; i < mCount; i++)
        {
            if (counters[i]->sameAs(_name, _power, _toughness))
            {
                counters[i]->added();
                counters[i]->nb++;
                WEvent * j = NEW WEventCounters(this,_name,_power,_toughness,true,false);
                dynamic_cast<WEventCounters*>(j)->targetCard = this->target;
                g->receiveEvent(j);
                delete(e);
                return mCount;
            }
        }
        Counter * counter = NEW Counter(target, _name, _power, _toughness);
        counters.push_back(counter);
        counter->added();
        if (!_noevent)
        {
            WEvent * w = NEW WEventCounters(this,_name,_power,_toughness,true,false);
            dynamic_cast<WEventCounters*>(w)->targetCard = this->target;
            g->receiveEvent(w);
        }
        mCount++;
        /*the damage test should be handled on game state based effect i think*/
        //this->target->doDamageTest = 1;
        //this->target->afterDamage();
    }
    delete(e);
    return mCount;
}

int Counters::addCounter(int _power, int _toughness)
{
    return addCounter("", _power, _toughness, false);
}

int Counters::addCounter(int _power, int _toughness, bool _noevent)
{
    return addCounter("", _power, _toughness, _noevent);
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
            GameObserver *g = target->getObserver();
            WEvent * e = NEW WEventCounters(this,_name,_power,_toughness,false,true);
            dynamic_cast<WEventCounters*>(e)->targetCard = this->target;
            g->receiveEvent(e);

            // special case: when the last time counter is removed from non-suspended card
            // sacrifice that card
            if (!target->suspended && counters[i]->name == "time" && counters[i]->nb == 0) {
                MTGCardInstance * beforeCard = target;
                target->controller()->game->putInGraveyard(target);
                WEvent * e = NEW WEventCardSacrifice(beforeCard, target);
                g->receiveEvent(e);
            }

            //special case:if a card is suspended and no longer has a time counter when the last is removed, the card is cast.
            if (target->suspended && !target->counters->hasCounter("time",0,0))
            {
                GameObserver * game = target->getObserver();
                MTGAbility *ac = NEW AACastCard(game, game->mLayers->actionLayer()->getMaxId(), target, target, false, false, true, "", "", false, false);
                MayAbility *ma1 = NEW MayAbility(game, game->mLayers->actionLayer()->getMaxId(), ac->clone(), target, true);
                MTGAbility *ga1 = NEW GenericAddToGame(game, game->mLayers->actionLayer()->getMaxId(), target, NULL, ma1->clone());
                SAFE_DELETE(ac);
                SAFE_DELETE(ma1);
                ga1->resolve();
                SAFE_DELETE(ga1);
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
