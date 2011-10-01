#include "PrecompiledHeader.h"

#include "CardDescriptor.h"
#include "Subtypes.h"
#include "Counters.h"

CardDescriptor::CardDescriptor()
    :  MTGCardInstance(), mColorExclusions(0)
{
    init();
    counterName = "";
    counterPower = 0;
    counterToughness = 0;
    counterNB = 0;
    mode = CD_AND;
    powerComparisonMode = COMPARISON_NONE;
    toughnessComparisonMode = COMPARISON_NONE;
    manacostComparisonMode = COMPARISON_NONE;
    counterComparisonMode = COMPARISON_NONE;
    convertedManacost = -1;
    compareName ="";
    nameComparisonMode = COMPARISON_NONE;
    colorComparisonMode = COMPARISON_NONE;
    CDopponentDamaged = 0;
    CDcontrollerDamaged = 0;
}

int CardDescriptor::init()
{
    int result = MTGCardInstance::init();
    attacker = 0;
    defenser = NULL;
    banding = NULL;
    anyCounter = 0;
    //Remove unnecessary pointers
    SAFE_DELETE(counters);
    SAFE_DELETE(previous);
    return result;
}

void CardDescriptor::unsecureSetTapped(int i)
{
    tapped = i;
}

void CardDescriptor::unsecuresetfresh(int k)
{
    fresh = k;
}

void CardDescriptor::setisMultiColored(int w)
{
    isMultiColored = w;
}
    
void CardDescriptor::setNegativeSubtype(string value)
{
    int id = Subtypes::subtypesList->find(value);
    addType(-id);
}

// Very generic function to compare a value to a criterion.
// Should be easily transferable to a more generic class if desired.
bool CardDescriptor::valueInRange(int comparisonMode, int value, int criterion)
{
    switch (comparisonMode)
    {
    case COMPARISON_AT_MOST:
        return (value <= criterion);
    case COMPARISON_AT_LEAST:
        return (value >= criterion);
    case COMPARISON_EQUAL:
        return (value == criterion);
    case COMPARISON_GREATER:
        return (value > criterion);
    case COMPARISON_LESS:
        return (value < criterion);
    case COMPARISON_UNEQUAL:
        return (value != criterion);
    }
    return false;
}


MTGCardInstance * CardDescriptor::match_or(MTGCardInstance * card)
{
    int found = 1;
    for (size_t i = 0; i < types.size(); i++)
    {
        found = 0;
        if (types[i] >= 0)
        {

            if (card->hasSubtype(types[i]) || (Subtypes::subtypesList->find(card->getLCName(), false) == types[i]))
            {
                found = 1;
                break;
            }
        }
        else
        {
            if (!card->hasSubtype(-types[i]) && (Subtypes::subtypesList->find(card->getLCName(), false) != -types[i]))
            {
                found = 1;
                break;
            }
        }
    }
    if (!found)
        return NULL;

    if (colors)
    {
        found = (colors & card->colors);
        if (!found)
            return NULL;
    }

    if (mColorExclusions)
    {
        found = mColorExclusions & card->colors;
        if (found)
            return NULL;
    }

    // Quantified restrictions are always AND-ed:
    if (powerComparisonMode && !valueInRange(powerComparisonMode, card->getPower(), power))
        return NULL;
    if (toughnessComparisonMode && !valueInRange(toughnessComparisonMode, card->getToughness(), toughness))
        return NULL;
    if (manacostComparisonMode && !valueInRange(manacostComparisonMode, card->getManaCost()->getConvertedCost(), convertedManacost))
        return NULL;
    if (nameComparisonMode && compareName != card->name)
        return NULL;
    return card;
}

MTGCardInstance * CardDescriptor::match_and(MTGCardInstance * card)
{
    MTGCardInstance * match = card;
    for (size_t i = 0; i < types.size(); i++)
    {
        if (types[i] >= 0)
        {
            if (!card->hasSubtype(types[i]) && !(Subtypes::subtypesList->find(card->getLCName(), false) == types[i]))
            {
                match = NULL;
            }
        }
        else
        {
            if (card->hasSubtype(-types[i]) || (Subtypes::subtypesList->find(card->getLCName(), false) == -types[i]))
            {
                match = NULL;
            }
        }
    }
    if ((colors & card->colors) != colors)
        match = NULL;

    if (mColorExclusions)
    {
        if ((mColorExclusions & card->colors) == mColorExclusions)
            match = NULL;
    }

    if (powerComparisonMode && !valueInRange(powerComparisonMode, card->getPower(), power))
        match = NULL;
    if (toughnessComparisonMode && !valueInRange(toughnessComparisonMode, card->getToughness(), toughness))
        match = NULL;
    if (manacostComparisonMode && !valueInRange(manacostComparisonMode, card->getManaCost()->getConvertedCost(), convertedManacost))
        match = NULL;
    if(nameComparisonMode && compareName != card->name)
        match = NULL;

    return match;
}

MTGCardInstance * CardDescriptor::match(MTGCardInstance * card)
{

    MTGCardInstance * match = card;
    if (mode == CD_AND)
    {
        match = match_and(card);
    }
    else if (mode == CD_OR)
    {
        match = match_or(card);
    }

    //Abilities
    BasicAbilitiesSet set = basicAbilities & card->basicAbilities;
    if (set != basicAbilities)
        return NULL;
    
    BasicAbilitiesSet excludedSet = mAbilityExclusions & card->basicAbilities;
    if (excludedSet.any())
        return NULL;


    if ((tapped == -1 && card->isTapped()) || (tapped == 1 && !card->isTapped()))
    {
        match = NULL;
    }

    if ((fresh == -1 && card->fresh) || (fresh == 1 && !card->fresh))
    {
        match = NULL;
    }

    if ((isMultiColored == -1 && card->isMultiColored) || (isMultiColored == 1 && !card->isMultiColored))
    {
        match = NULL;
    }
        if ((isLeveler == -1 && card->isLeveler) || (isLeveler == 1 && !card->isLeveler))
        {
            match = NULL;
        }
        if ((CDenchanted == -1 && card->enchanted) || (CDenchanted == 1 && !card->enchanted))
        {
            match = NULL;
        }
        if ((CDdamaged == -1 && card->wasDealtDamage) || (CDdamaged == 1 && !card->wasDealtDamage))
        {
            match = NULL;
        }
        Player * p = card->controller()->opponent();//controller()->opponent();
        if ((CDopponentDamaged == -1 && card->damageToOpponent && card->controller() == p) || (CDopponentDamaged == 1 && !card->damageToOpponent && card->controller() == p)
            || (CDopponentDamaged == -1 && card->damageToController && card->controller() == p->opponent()) || (CDopponentDamaged == 1 && !card->damageToController && card->controller() == p->opponent()))
        {
            match = NULL;
        }
        if ((CDcontrollerDamaged == -1 && card->damageToController && card->controller() == p) || (CDcontrollerDamaged == 1 && !card->damageToController && card->controller() == p)
            || (CDcontrollerDamaged == -1 && card->damageToOpponent && card->controller() == p->opponent()) || (CDcontrollerDamaged == 1 && !card->damageToOpponent && card->controller() == p->opponent()))
        {
            match = NULL;
        }
        if ((isToken == -1 && card->isToken) || (isToken == 1 && !card->isToken))
        {
            match = NULL;
        }
    if (attacker == 1)
    {
        if (defenser == &AnyCard)
        {
            if (!card->attacker && !card->defenser)
                match = NULL;
        }
        else
        {
            if (!card->attacker)
                match = NULL;
        }
    }
    else if (attacker == -1)
    {
        if (defenser == &NoCard)
        {
            if (card->attacker || card->defenser)
                match = NULL;
        }
        else
        {
            if (card->attacker)
                match = NULL;
        }
    }
    else
    {
        if (defenser == &NoCard)
        {
            if (card->defenser)
                match = NULL;
        }
        else if (defenser == &AnyCard)
        {
            if (!card->defenser)
                match = NULL;
        }
        else
        {
            // we don't care about the attack/blocker state
        }
    }

    //Counters
    if (anyCounter)
    {
        if (!(card->counters->mCount))
        {
            match = NULL;
        }
        else
        {
            int hasCounter = 0;
            for (int i = 0; i < card->counters->mCount; i++)
            {
                if (card->counters->counters[i]->nb > 0)
                    hasCounter = 1;
            }
            if (!hasCounter)
                match = NULL;
        }
    }
    else
    {
        if (counterComparisonMode)
        {
            Counter * targetCounter = card->counters->hasCounter(counterName.c_str(), counterPower, counterToughness);
            if (targetCounter)
            {
                if (!valueInRange(counterComparisonMode, targetCounter->nb, counterNB))
                    match = NULL;
            }
            else
            {
                if (counterComparisonMode != COMPARISON_LESS && counterComparisonMode != COMPARISON_AT_MOST)
                    match = NULL;
            }
        }
    }

    return match;
}

MTGCardInstance * CardDescriptor::match(MTGGameZone * zone)
{
    return (nextmatch(zone, NULL));
}

MTGCardInstance * CardDescriptor::nextmatch(MTGGameZone * zone, MTGCardInstance * previous)
{
    int found = 0;
    if (NULL == previous)
        found = 1;
    for (int i = 0; i < zone->nb_cards; i++)
    {
        if (found && match(zone->cards[i]))
        {
            return zone->cards[i];
        }
        if (zone->cards[i] == previous)
        {
            found = 1;
        }
    }
    return NULL;
}

void CardDescriptor::SetExclusionColor(int _color, int removeAllOthers)
{
    if (removeAllOthers)
        mColorExclusions = 0;

    mColorExclusions |= ConvertColorToBitMask(_color);
}
