#include "PrecompiledHeader.h"

#include "CardDescriptor.h"
#include "Subtypes.h"
#include "Counters.h"
#include "ExtraCost.h"
#include "WParsedInt.h"

CardDescriptor::CardDescriptor()
    :  MTGCardInstance(), mColorExclusions(0)
{
    init();
    counterName = "";
    counterPower = 0;
    counterToughness = 0;
    counterNB = 0;
    mode = CD_AND;
    foretoldComparisonMode = COMPARISON_NONE;
    kickedComparisonMode = COMPARISON_NONE;
    powerComparisonMode = COMPARISON_NONE;
    toughnessComparisonMode = COMPARISON_NONE;
    manacostComparisonMode = COMPARISON_NONE;
    counterComparisonMode = COMPARISON_NONE;
    convertedManacost = -1;
    numofColorsComparisonMode = COMPARISON_NONE;
    numofColors = -1;
    zposComparisonMode = COMPARISON_NONE;
    zposition = -1;
    hasKickerCost = 0;
    hasConvokeCost = 0;
    hasFlashbackCost = 0;
    hasBackSide = 0;
    hasPartner = 0;
    isPermanent = 0;
    modified = 0;
    toxicity = 0;
    hasXCost = 0;
    compareName = "";
    nameComparisonMode = COMPARISON_NONE;
    colorComparisonMode = COMPARISON_NONE;
    CDopponentDamaged = 0;
    CDcontrollerDamaged = 0;
    CDdamager = 0;
    CDgeared = 0;
    CDdiscarded = 0;
    CDattached = 0;
    CDblocked = 0;
    CDcanProduceC = 0;
    CDcanProduceG = 0;
    CDcanProduceU = 0;
    CDcanProduceR = 0;
    CDcanProduceB = 0;
    CDcanProduceW = 0;
    CDnocolor = 0;
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

void CardDescriptor::unsecureSetKicked(int k)
{
    kicked = k;
}

void CardDescriptor::unsecureSetHasKickerCost(int k)
{
    hasKickerCost = k;
}

void CardDescriptor::unsecureSetHasConvokeCost(int k)
{
    hasConvokeCost = k;
}

void CardDescriptor::unsecureSetHasFlashbackCost(int k)
{
    hasFlashbackCost = k;
}

void CardDescriptor::unsecureSetHasBackSide(int k)
{
    hasBackSide = k;
}

void CardDescriptor::unsecureSetModified(int k)
{
    modified = k;
}

void CardDescriptor::unsecureSetHasToxic(int k)
{
    toxicity = k;
}

void CardDescriptor::unsecureSetHasPartner(int k)
{
    hasPartner = k;
}

void CardDescriptor::unsecureSetIsPermanent(int k)
{
    isPermanent = k;
}

void CardDescriptor::unsecureSetTapped(int i)
{
    tapped = i;
}

void CardDescriptor::unsecuresetfresh(int k)
{
    fresh = k;
}

void CardDescriptor::unsecuresetrecent(int j)
{
    entersBattlefield = j;
}

void CardDescriptor::setisMultiColored(int w)
{
    isMultiColored = w;
}

void CardDescriptor::setNegativeSubtype(string value)
{
    int id = MTGAllCards::findType(value);
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

            if (card->hasSubtype(types[i]) || (MTGAllCards::findType(card->getLCName(), false) == types[i]))
            {
                found = 1;
                break;
            }
        }
        else
        {
            if (!card->hasSubtype(-types[i]) && (MTGAllCards::findType(card->getLCName(), false) != -types[i]))
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
    if (foretoldComparisonMode && !valueInRange(foretoldComparisonMode, card->foretellTurn, foretellTurn))
        return NULL;
    if (kickedComparisonMode && (!valueInRange(kickedComparisonMode, card->kicked, kicked) || card->has(Constants::HASREPLICATE) || card->has(Constants::HASSTRIVE))) //Some kicker costs are not a real kicker (e.g. Fuse cost, Replicate cost, Strive cost).
        return NULL;
    if (powerComparisonMode && !valueInRange(powerComparisonMode, card->getPower(), power))
        return NULL;
    if (toughnessComparisonMode){ // Toughness comparison has a different meaning for planeswalkers and battles.
        if(!card->isCreature() && card->counters && (card->hasType(Subtypes::TYPE_PLANESWALKER) || card->hasType(Subtypes::TYPE_BATTLE))){
            for(size_t i = 0; i < card->counters->counters.size(); ++i){
                if((card->counters->counters[i]->name == "loyalty" && card->hasType(Subtypes::TYPE_PLANESWALKER)) || (card->counters->counters[i]->name == "defense" && card->hasType(Subtypes::TYPE_BATTLE))){
                    if(!valueInRange(toughnessComparisonMode, card->counters->counters[i]->nb, toughness))
                        return NULL;
                }
            }
        } else if(!valueInRange(toughnessComparisonMode, card->getToughness(), toughness))
            return NULL;
    }
    if (numofColorsComparisonMode){
        int totalcolor = 0;
        WParsedInt* value = NEW WParsedInt("mycolnum", NULL, card);
        if(value){
            totalcolor = value->getValue();
            SAFE_DELETE(value);
        }
        if(!valueInRange(numofColorsComparisonMode, totalcolor, numofColors))
            return NULL;
    }
    if (manacostComparisonMode && !valueInRange(manacostComparisonMode, card->myconvertedcost, convertedManacost))
        return NULL;
    if (zposComparisonMode && !valueInRange(zposComparisonMode, card->zpos, zposition))
        return NULL;
    if ((nameComparisonMode == COMPARISON_UNEQUAL && compareName == card->name) || (nameComparisonMode && nameComparisonMode != COMPARISON_UNEQUAL && compareName != card->name))
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
            if (!card->hasSubtype(types[i]) && !(MTGAllCards::findType(card->getLCName(), false) == types[i]))
            {
                match = NULL;
            }
        }
        else
        {
            if (card->hasSubtype(-types[i]) || (MTGAllCards::findType(card->getLCName(), false) == -types[i]))
            {
                match = NULL;
            }
        }
    }
    if ((colors & card->colors) != colors)
        match = NULL;

    if (mColorExclusions)
    {
        // if any of forbidden colors intersect with card colors
        if ((mColorExclusions & card->colors) != 0)
            match = NULL;
    }

    if (foretoldComparisonMode && !valueInRange(foretoldComparisonMode, card->foretellTurn, foretellTurn))
        match = NULL;
    if (kickedComparisonMode && (!valueInRange(kickedComparisonMode, card->kicked, kicked) || card->has(Constants::HASREPLICATE) || card->has(Constants::HASSTRIVE))) //Some kicker costs are not a real kicker (e.g. Fuse cost, Replicate cost, Strive cost).
        match = NULL;
    if (powerComparisonMode && !valueInRange(powerComparisonMode, card->getPower(), power))
        match = NULL;
    if (toughnessComparisonMode){ // Toughness comparison has a different meaning for planeswalkers and battles.
       if(!card->isCreature() && card->counters && (card->hasType(Subtypes::TYPE_PLANESWALKER) || card->hasType(Subtypes::TYPE_BATTLE))){
            for(size_t i = 0; i < card->counters->counters.size(); ++i){
                if((card->counters->counters[i]->name == "loyalty" && card->hasType(Subtypes::TYPE_PLANESWALKER)) || (card->counters->counters[i]->name == "defense" && card->hasType(Subtypes::TYPE_BATTLE))){
                    if(!valueInRange(toughnessComparisonMode, card->counters->counters[i]->nb, toughness))
                        return NULL;
                }
            }
        } else if(!valueInRange(toughnessComparisonMode, card->getToughness(), toughness))
            return NULL;
    }
    if (numofColorsComparisonMode){
        int totalcolor = 0;
        WParsedInt* value = NEW WParsedInt("mycolnum", NULL, card);
        if(value){
            totalcolor = value->getValue();
            SAFE_DELETE(value);
        }
        if(!valueInRange(numofColorsComparisonMode, totalcolor, numofColors))
            return NULL;
    }
    if (manacostComparisonMode && !valueInRange(manacostComparisonMode, card->myconvertedcost, convertedManacost))
        match = NULL;
    if (zposComparisonMode && !valueInRange(zposComparisonMode, card->zpos, zposition))
        match = NULL;
    if ((nameComparisonMode == COMPARISON_UNEQUAL && compareName == card->name) || (nameComparisonMode && nameComparisonMode != COMPARISON_UNEQUAL && compareName != card->name))
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
    else if (mode == CD_NAND)
    {
        match = match_and(card);
        if(!match)
            match = card;
        else
            match = NULL;
    }
    else if (mode == CD_OR)
    {
        match = match_or(card);
    }
    else if (mode == CD_NOR)
    {
        match = match_or(card);
        if(!match)
            match = card;
        else
            match = NULL;
    }

    //Abilities
    BasicAbilitiesSet set = basicAbilities & card->basicAbilities;
    if (set != basicAbilities)
        return NULL;
    
    BasicAbilitiesSet excludedSet = mAbilityExclusions & card->basicAbilities;
    if (excludedSet.any())
        return NULL;

    if (hasConvokeCost != 0){
        bool hasConvoke = false;
        ManaCost * extra = card->getManaCost()->getAlternative(); 
        if(extra && extra->extraCosts){
            for(unsigned int i = 0; i < extra->extraCosts->costs.size() && !hasConvoke; i++){
                if(dynamic_cast<Convoke*> (extra->extraCosts->costs[i]))
                    hasConvoke = true;
            }
        }
        if ((hasConvokeCost == -1 && hasConvoke) || (hasConvokeCost == 1 && !hasConvoke))
        {
            match = NULL;
        }
    }

    if ((modified == -1 && ((card->enchanted) || (card->equipment > 0) || (card->counters->mCount))) || (modified == 1 && !((card->enchanted) || (card->equipment > 0) || (card->counters->mCount))))
    {
        match = NULL;
    }

    if ((hasKickerCost == -1 && ((card->getManaCost()->getKicker() && !card->basicAbilities[Constants::HASNOKICKER]) || (!card->getManaCost()->getKicker() && card->basicAbilities[Constants::HASOTHERKICKER]))) || (hasKickerCost == 1 && !((card->getManaCost()->getKicker() && !card->basicAbilities[Constants::HASNOKICKER]) || (!card->getManaCost()->getKicker() && card->basicAbilities[Constants::HASOTHERKICKER]))))
    {
        match = NULL; //Some kicker costs are not a real kicker (e.g. Fuse cost).
    }

    if ((hasFlashbackCost == -1 && (card->getManaCost()->getFlashback() && !card->has(Constants::HASAFTERMATH))) || (hasFlashbackCost == 1 && (!card->getManaCost()->getFlashback() || (card->getManaCost()->getFlashback() && card->has(Constants::HASAFTERMATH)))))
    {
        match = NULL;
    }

    if ((hasBackSide == -1 && card->backSide != "") || (hasBackSide == 1 && card->backSide == ""))
    {
        match = NULL;
    }

    if ((toxicity == -1 && card->getToxicity() > 0) || (toxicity == 1 && card->getToxicity() == 0))
    {
        match = NULL;
    }

    if ((hasPartner == -1 && card->partner != "") || (hasPartner == 1 && card->partner == ""))
    {
        match = NULL;
    }

    if ((isPermanent == -1 && card->isPermanent()) || (isPermanent == 1 && !card->isPermanent()))
    {
        match = NULL;
    }

    if ((hasXCost == -1 && card->getManaCost()->hasX()) || (hasXCost == 1 && !card->getManaCost()->hasX()))
    {
        match = NULL;
    }

    if ((isFlipped == -1 && card->isFlipped > 0) || (isFlipped == 1 && card->isFlipped == 0))
    {
        match = NULL;
    }

    if ((isCommander == -1 && card->isCommander > 0) || (isCommander == 1 && card->isCommander == 0))
    {
        match = NULL;
    }

    if ((isRingBearer == -1 && card->isRingBearer > 0) || (isRingBearer == 1 && card->isRingBearer == 0))
    {
        match = NULL;
    }

    if ((tapped == -1 && card->isTapped()) || (tapped == 1 && !card->isTapped()))
    {
        match = NULL;
    }

    if ((CDdiscarded == -1 && card->discarded) || (CDdiscarded == 1 && !card->discarded))
    {
        match = NULL;
    }

    if ((fresh == -1 && card->fresh) || (fresh == 1 && !card->fresh))
    {
        match = NULL;
    }

    if ((entersBattlefield == -1 && card->entersBattlefield) || (entersBattlefield == 1 && !card->entersBattlefield))
    {
        match = NULL;
    }
    
    if ((CDgeared == -1 && card->equipment > 0) || (CDgeared == 1 && card->equipment < 1))
    {
        match = NULL;
    }
 
    if ((CDattached == -1 && card->parentCards.size() > 0) || (CDattached == 1 && card->parentCards.size() < 1))
    {
        match = NULL;
    }

    if (CDblocked == -1)
    {
        if(!card->isAttacker())
            match = NULL;
        else
        {
            if(card->isBlocked())
                match = NULL;
        }
    }

    if (CDblocked == 1)
    {
        if(!card->isAttacker())
            match = NULL;
        else
        {
            if(!card->isBlocked())
                match = NULL;
        }
    }

    if (CDcanProduceC == -1)
    {
        int count = card->canproduceMana(Constants::MTG_COLOR_ARTIFACT) + card->canproduceMana(Constants::MTG_COLOR_WASTE);
        if (count)
            match = NULL;
    }

    if (CDcanProduceC == 1)
    {
        int count = card->canproduceMana(Constants::MTG_COLOR_ARTIFACT) + card->canproduceMana(Constants::MTG_COLOR_WASTE);
        if (!count)
            match = NULL;
    }

    if ((CDcanProduceG == -1 && card->canproduceMana(Constants::MTG_COLOR_GREEN) == 1) || (CDcanProduceG == 1 && card->canproduceMana(Constants::MTG_COLOR_GREEN) == 0))
    {
        match = NULL;
    }
    
    if ((CDcanProduceU == -1 && card->canproduceMana(Constants::MTG_COLOR_BLUE) == 1) || (CDcanProduceU == 1 && card->canproduceMana(Constants::MTG_COLOR_BLUE) == 0))
    {
        match = NULL;
    }
    
    if ((CDcanProduceR == -1 && card->canproduceMana(Constants::MTG_COLOR_RED) == 1) || (CDcanProduceR == 1 && card->canproduceMana(Constants::MTG_COLOR_RED) == 0))
    {
        match = NULL;
    }
    
    if ((CDcanProduceB == -1 && card->canproduceMana(Constants::MTG_COLOR_BLACK) == 1) || (CDcanProduceB == 1 && card->canproduceMana(Constants::MTG_COLOR_BLACK) == 0))
    {
        match = NULL;
    }
    
    if ((CDcanProduceW == -1 && card->canproduceMana(Constants::MTG_COLOR_WHITE) == 1) || (CDcanProduceW == 1 && card->canproduceMana(Constants::MTG_COLOR_WHITE) == 0))
    {
        match = NULL;
    }

    if ((CDnocolor == -1 && card->getColor() == 0))
    {
        match = NULL;
    }
    else if(CDnocolor == 1)
    {
        if(!card->has(Constants::DEVOID))
            if(card->getColor()>0)
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

    if ((CDdamaged == -1 && card->wasDealtDamage > 0) || (CDdamaged == 1 && card->wasDealtDamage == 0))
    {
        match = NULL;
    }

    if ((CDdamager == -1 && (card->damageToOpponent > 0 || card->damageToController > 0 || card->damageToCreature > 0)) 
            || (CDdamager == 1 && !(card->damageToOpponent > 0 || card->damageToController > 0 || card->damageToCreature > 0)))
    {
        match = NULL;
    }

    if(CDopponentDamaged == -1 || CDopponentDamaged == 1 || CDcontrollerDamaged == -1 || CDcontrollerDamaged == 1)
    {
        Player * p = card->controller();
        if ((CDopponentDamaged == -1 && card->damageToOpponent > 0 && card->controller() == p)
            || (CDopponentDamaged == 1 && card->damageToOpponent == 0 && card->controller() == p)
            || (CDopponentDamaged == -1 && card->damageToController > 0 && card->controller() == p->opponent())
            || (CDopponentDamaged == 1 && card->damageToController == 0 && card->controller() == p->opponent()))
        {
            match = NULL;
        }
        if ((CDcontrollerDamaged == -1 && card->damageToController > 0 && card->controller() == p)
            || (CDcontrollerDamaged == 1 && card->damageToController == 0 && card->controller() == p)
            || (CDcontrollerDamaged == -1 && card->damageToOpponent > 0 && card->controller() == p->opponent())
            || (CDcontrollerDamaged == 1 && card->damageToOpponent == 0 && card->controller() == p->opponent()))
        {
            match = NULL;
        }
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
    if (anyCounter == -1)
    {
        if (card->counters->mCount)
        {
            match = NULL;
        }
    }
    else if (anyCounter)
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
