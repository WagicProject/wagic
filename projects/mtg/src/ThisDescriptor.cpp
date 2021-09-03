#include "PrecompiledHeader.h"

#include "ThisDescriptor.h"
#include "Counters.h"
#include "MTGCardInstance.h"
#include "CardDescriptor.h"
#include "AllAbilities.h"

ThisDescriptor::~ThisDescriptor()
{
    //nothing to do for now
}

//Returns the amount by which a value passes the comparison.
int ThisDescriptor::matchValue(int value)
{
    switch (comparisonMode)
    {
    case COMPARISON_AT_MOST:
        return (comparisonCriterion - value + 1);
    case COMPARISON_AT_LEAST:
        return (value - comparisonCriterion + 1);
    case COMPARISON_EQUAL:
        return (comparisonCriterion == value);
    case COMPARISON_GREATER:
        return (value - comparisonCriterion);
    case COMPARISON_LESS:
        return (comparisonCriterion - value);
    case COMPARISON_UNEQUAL:
        return (comparisonCriterion != value);
    }
    return 0;
}

ThisDescriptor * ThisDescriptorFactory::createThisDescriptor(GameObserver* observer, string s)
{
    size_t found;

    string whitespaces(" \t\f\v\n\r");

    found = s.find_last_not_of(whitespaces);
    if (found != string::npos)
        s.erase(found + 1);
    else
        return NULL;

    found = s.find_first_not_of(whitespaces);
    if (found != string::npos)
        s.erase(0, found);
    else
        return NULL;

    //set comparison mode
    //equal, greater, and less must remain above the others, otherwise the others may never be used.
    int mode = 0;
    size_t found2 = string::npos;
    int opLength = 0;

    found = s.find("=");
    if (found != string::npos)
    {
        mode = COMPARISON_EQUAL;
        found2 = found + 1;
        opLength = 1;
    }
    found = s.find(">");
    if (found != string::npos)
    {
        mode = COMPARISON_GREATER;
        found2 = found + 1;
        opLength = 1;
    }
    found = s.find("<");
    if (found != string::npos)
    {
        mode = COMPARISON_LESS;
        found2 = found + 1;
        opLength = 1;
    }
    found = s.find("<=");
    if (found != string::npos)
    {
        mode = COMPARISON_AT_MOST;
        found2 = found + 2;
        opLength = 2;
    }
    found = s.find(">=");
    if (found != string::npos)
    {
        mode = COMPARISON_AT_LEAST;
        found2 = found + 2;
        opLength = 2;
    }
    found = s.find("!=");
    if (found != string::npos)
    {
        mode = COMPARISON_UNEQUAL;
        found2 = found + 2;
        opLength = 2;
    }
    if (!mode) mode = COMPARISON_AT_LEAST;

    //get comparison criterion
    int criterionFound = 0;
    int criterion = 1;
    if ((found2 != string::npos) && (found2 < s.length()))
    {
        criterion = atoi(s.substr(found2).c_str());
        criterionFound = 1;
    }
    if (found2 != string::npos) s.erase(found2 - opLength);

    //counters
    found = s.find("counter{");
    if (found != string::npos)
    {
        size_t start = s.find("{");
        size_t end = s.find("}");
        string counterString = s.substr(start + 1, end - start - 1);
        AbilityFactory abf(observer);
        Counter * counter = abf.parseCounter(counterString, NULL);
        if (counter)
        {
            if (criterionFound) counter->nb = criterion;
            ThisCounter * td = NEW ThisCounter(counter);
            if (td)
            {
                td->comparisonMode = mode;
                return td;
            }
        }
        return NULL;
    }

    //any counter
    found = s.find("counters");
    if (found != string::npos)
    {
        ThisCounterAny * td = NEW ThisCounterAny(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //mutations
    found = s.find("mutations");
    if (found != string::npos)
    {
        ThisMutation * td = NEW ThisMutation(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    //equips and auras
    found = s.find("gear");//still same meaning, better wording to word conflict with MTGAbility equip.
    if (found != string::npos)
    {
        ThisEquip * td = NEW ThisEquip(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    found = s.find("auras");
    if (found != string::npos)
    {
        ThisAuras * td = NEW ThisAuras(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    // opponent damage count
        found = s.find("opponentdamagecount");
    if (found != string::npos)
    {
        ThisOpponentDamageAmount * td = NEW ThisOpponentDamageAmount(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    //untapped status
    found = s.find("untapped");
    if (found != string::npos)
    {
        ThisUntapped * td = NEW ThisUntapped(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    found = s.find("tapped");
    if (found != string::npos)
    {
        ThisTapped * td = NEW ThisTapped(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    //whenever this creature attacks do effect
    found = s.find("attacking");
    if (found != string::npos)
    {
        ThisAttacked * td = NEW ThisAttacked(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    //whenever this creature blocks do effect
    found = s.find("blocking");
    if (found != string::npos)
    {
        ThisBlocked * td = NEW ThisBlocked(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    
    //whenever this creature attacks do effect
    found = s.find("notblocked");
    if (found != string::npos)
    {
        ThisNotBlocked * td = NEW ThisNotBlocked(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //this creature was dealt damage this turn
    found = s.find("damaged");
    if (found != string::npos)
    {
        ThisDamaged * td = NEW ThisDamaged(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }
    
        //this creature has 2 of the same weapons in its children vector
    found = s.find("dualwielding");
    if (found != string::npos)
    {
        ThisDualWield * td = NEW ThisDualWield(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //controller life
    found = s.find("opponentlife");
    if (found != string::npos)
    {
        ThisOpponentlife * td = NEW ThisOpponentlife(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //controller life
    found = s.find("controllerlife");
    if (found != string::npos)
    {
        ThisControllerlife * td = NEW ThisControllerlife(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //controller creature spells - conduit of ruin
    found = s.find("creaturespells");
    if (found != string::npos)
    {
        ThisCreatureSpells * td = NEW ThisCreatureSpells(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //power
    found = s.find("power");
    if (found != string::npos)
    {
        ThisPower * td = NEW ThisPower(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //toughness
    found = s.find("toughness");
    if (found != string::npos)
    {
        ThisToughness * td = NEW ThisToughness(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    // X
    found = s.find("x");
    if (found != string::npos)
    {
        ThisX * td = NEW ThisX(criterion);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    //this Variable
    vector<string>splitVComparison = parseBetween(s,"variable{","}",false);
    if (splitVComparison.size())
    {
        ThisVariable * td = NEW ThisVariable(criterion,splitVComparison[1]);
        if (td)
        {
            td->comparisonMode = mode;
            return td;
        }
        return NULL;
    }

    vector<string>splitTargetComparison = parseBetween(s,"cantargetcard(",")",false);
    if (splitTargetComparison.size())
    {
        TargetChooserFactory tf(observer);
        TargetChooser * tcc = tf.createTargetChooser(splitTargetComparison[1],NULL);
        ThisTargetCompare * td = NEW ThisTargetCompare(tcc);
        if (td)
        {
            return td;
        }
        return NULL;
    }

    return NULL;
}

ThisTargetCompare::ThisTargetCompare(TargetChooser * _tcc)
{
    targetComp = _tcc;
}

int ThisTargetCompare::match(MTGCardInstance * card)
{
    if(targetComp->canTarget(card))
        return 1;
    return 0;
}

ThisTargetCompare::~ThisTargetCompare()
{
    SAFE_DELETE(targetComp);
}

ThisTargetCompare* ThisTargetCompare::clone() const 
{
    ThisTargetCompare * a =  NEW ThisTargetCompare(*this);

    return a;
}

ThisCounter::ThisCounter(Counter * _counter)
{
    counter = _counter;
    comparisonCriterion = counter->nb;
}

ThisCounter::ThisCounter(int power, int toughness, int nb, const char * name)
{
    counter = NEW Counter(NULL, name, power, toughness);
    comparisonCriterion = nb;
}

int ThisCounter::match(MTGCardInstance * card)
{
    Counter * targetCounter = card->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);
    if (targetCounter)
    {
        return matchValue(targetCounter->nb);
    }
    else
    {
        switch (comparisonMode)
        {
        case COMPARISON_LESS:
            return comparisonCriterion;
        case COMPARISON_AT_MOST:
            return comparisonCriterion + 1;
        case COMPARISON_UNEQUAL:
            if (comparisonCriterion)
                return 1;
            else
                return 0;
        case COMPARISON_EQUAL:
            if (comparisonCriterion)
                return 0;
            else
                return 1;
        default:
            return 0;
        }
    }
}

ThisCounter::~ThisCounter()
{
    SAFE_DELETE(counter);
}

ThisCounter* ThisCounter::clone() const 
{
    ThisCounter * a =  NEW ThisCounter(*this);
    a->counter = NEW Counter(NULL, counter->name.c_str(), counter->power, counter->toughness);
    return a;
}

ThisOpponentlife::ThisOpponentlife(int olife)
{
    comparisonCriterion = olife;
}

int ThisOpponentlife::match(MTGCardInstance * card)
{
    return matchValue(card->controller()->opponent()->life);
}

ThisOpponentlife* ThisOpponentlife::clone() const 
{
    return NEW ThisOpponentlife(*this);
}

ThisControllerlife::ThisControllerlife(int life)
{
    comparisonCriterion = life;
}

int ThisControllerlife::match(MTGCardInstance * card)
{
    return matchValue(card->controller()->life);
}

ThisControllerlife* ThisControllerlife::clone() const 
{
    return NEW ThisControllerlife(*this);
}

ThisCreatureSpells::ThisCreatureSpells(int count)
{
    comparisonCriterion = count;
}

int ThisCreatureSpells::match(MTGCardInstance * card)
{
    return matchValue(card->controller()->game->stack->seenThisTurn("creature", Constants::CAST_ALL));
}

ThisCreatureSpells* ThisCreatureSpells::clone() const 
{
    return NEW ThisCreatureSpells(*this);
}

ThisPower::ThisPower(int power)
{
    comparisonCriterion = power;
}

int ThisPower::match(MTGCardInstance * card)
{
    return matchValue(card->power);
}

ThisPower* ThisPower::clone() const 
{
    return NEW ThisPower(*this);
}

ThisMutation::ThisMutation(int mutation)
{
    comparisonCriterion = mutation;
}
int ThisMutation::match(MTGCardInstance * card)
{
    return matchValue(card->mutation);
}

ThisMutation* ThisMutation::clone() const 
{
    return NEW ThisMutation(*this);
}

ThisEquip::ThisEquip(int equipment)
{
    comparisonCriterion = equipment;
}
int ThisEquip::match(MTGCardInstance * card)
{
    return matchValue(card->equipment);
}

ThisEquip* ThisEquip::clone() const 
{
    return NEW ThisEquip(*this);
}

ThisAuras::ThisAuras(int auras)
{
    comparisonCriterion = auras;
}
int ThisAuras::match(MTGCardInstance * card)
{
    return matchValue(card->auras);
}

ThisAuras* ThisAuras::clone() const 
{
    return NEW ThisAuras(*this);
}

ThisOpponentDamageAmount::ThisOpponentDamageAmount(int damagecount)
{
    comparisonCriterion = damagecount;
}
int ThisOpponentDamageAmount::match(MTGCardInstance * card)
{
    return matchValue(card->controller()->opponent()->damageCount);
}

ThisOpponentDamageAmount* ThisOpponentDamageAmount::clone() const 
{
    return NEW ThisOpponentDamageAmount(*this);
}

ThisUntapped::ThisUntapped(int untapped)
{
    comparisonCriterion = untapped;
}
int ThisUntapped::match(MTGCardInstance * card)
{
    return matchValue(!card->isTapped());
}

ThisUntapped* ThisUntapped::clone() const 
{
    return NEW ThisUntapped(*this);
}

ThisTapped::ThisTapped(int tapped)
{
    comparisonCriterion = tapped;
}
int ThisTapped::match(MTGCardInstance * card)
{
    return matchValue(card->isTapped());
}

ThisTapped* ThisTapped::clone() const 
{
    return NEW ThisTapped(*this);
}

ThisAttacked::ThisAttacked(int attack)
{

    comparisonCriterion = attack;
}

int ThisAttacked::match(MTGCardInstance * card)
{

    return matchValue(card->didattacked);
}

ThisAttacked* ThisAttacked::clone() const 
{
    return NEW ThisAttacked(*this);
}

ThisBlocked::ThisBlocked(int block)
{

    comparisonCriterion = block;
}

int ThisBlocked::match(MTGCardInstance * card)
{

    return matchValue(card->didblocked);
}

ThisBlocked* ThisBlocked::clone() const 
{
    return NEW ThisBlocked(*this);
}

ThisNotBlocked::ThisNotBlocked(int unblocked)
{

    comparisonCriterion = unblocked;
}

int ThisNotBlocked::match(MTGCardInstance * card)
{

    return matchValue(card->notblocked);
}

ThisNotBlocked* ThisNotBlocked::clone() const 
{
    return NEW ThisNotBlocked(*this);
}

ThisDamaged::ThisDamaged(int wasDealtDamage)
{

    comparisonCriterion = wasDealtDamage;
}

int ThisDamaged::match(MTGCardInstance * card)
{
int result = 0;
if(card->wasDealtDamage > 0)
result = 1;
    return matchValue(result);
}

ThisDamaged* ThisDamaged::clone() const 
{
    return NEW ThisDamaged(*this);
}

ThisDualWield::ThisDualWield(int dualWield)
{

    comparisonCriterion = dualWield;
}

int ThisDualWield::match(MTGCardInstance * card)
{
int result = 0;
if(card->isDualWielding)
result = 1;
    return matchValue(result);
}

ThisDualWield* ThisDualWield::clone() const 
{
    return NEW ThisDualWield(*this);
}

ThisToughness::ThisToughness(int toughness)
{
    comparisonCriterion = toughness;
}

int ThisToughness::match(MTGCardInstance * card)
{
    return matchValue(card->toughness);
}

ThisToughness* ThisToughness::clone() const 
{
    return NEW ThisToughness(*this);
}

ThisCounterAny::ThisCounterAny(int nb)
{
    comparisonCriterion = nb;
}

int ThisCounterAny::match(MTGCardInstance * card)
{
    int result = 0;
    for (int i = 0; i < card->counters->mCount; i++)
    {
        result += card->counters->counters[i]->nb;
    }
    return matchValue(result);
}

ThisCounterAny * ThisCounterAny::clone() const 
{
    return NEW ThisCounterAny(*this);
}

ThisX::ThisX(int x)
{
    comparisonCriterion = x;
}

int ThisX::match(MTGCardInstance * card)
{
    return matchValue(card->X);
}

ThisX * ThisX::clone() const 
{
    return NEW ThisX(*this);
}

//
ThisVariable::ThisVariable(int comp,string _vWord)
{
    vWord = _vWord;
    comparisonCriterion = comp;
}

int ThisVariable::match(MTGCardInstance * card)
{
    int result = 0;
    WParsedInt * res = NEW WParsedInt(vWord,NULL,card);
    result = res->getValue();
    SAFE_DELETE(res);
    return matchValue(result);
}

ThisVariable * ThisVariable::clone() const 
{
    return NEW ThisVariable(*this);
}
