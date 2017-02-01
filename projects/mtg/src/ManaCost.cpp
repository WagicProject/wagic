#include "PrecompiledHeader.h"
#include "ManaCost.h"
#include "ManaCostHybrid.h"
#include "ExtraCost.h"
#include "TargetChooser.h"
#include "Targetable.h"
#include "Player.h"
#include "WEvent.h"
#include "MTGAbility.h"
#include "AllAbilities.h"
#include "iterator"

SUPPORT_OBJECT_ANALYTICS(ManaCost)

ManaCost * ManaCost::parseManaCost(string s, ManaCost * _manaCost, MTGCardInstance * c)
{
    ManaCost * manaCost;
    GameObserver* g = c?c->getObserver():NULL;
    if (_manaCost)
    {
        manaCost = _manaCost;
    }
    else
    {
        manaCost = NEW ManaCost();
    }
    manaCost->xColor = -1;
    int state = 0;
    size_t start = 0;
    size_t end = 0;
    while (!s.empty() && state != -1)
    {
        switch (state)
        {
        case 0:
            start = s.find_first_of("{");
            if(s.find_first_of("{") != string::npos && start > 0)
            {
                string value = s.substr(start -1,end);
                if(value == "n{")//"restrictio n{m orbid} would read the n{m as {m} millcost
                    return manaCost;
            }
            if (start == string::npos)
            {
                return manaCost;
            }
            else
            {
                state = 1;
            }
            break;
        case 1:
            end = s.find_first_of("}");
            if (end == string::npos)
            {
                state = -1;
            }
            else
            {
                string value = s.substr(start + 1, end - 1 - start);

                if (value == "u")
                {
                    manaCost->add(Constants::MTG_COLOR_BLUE, 1);
                }
                else if (value == "b")
                {
                    manaCost->add(Constants::MTG_COLOR_BLACK, 1);
                }
                else if (value == "w")
                {
                    manaCost->add(Constants::MTG_COLOR_WHITE, 1);
                }
                else if (value == "g")
                {
                    manaCost->add(Constants::MTG_COLOR_GREEN, 1);
                }
                else if (value == "r")
                {
                    manaCost->add(Constants::MTG_COLOR_RED, 1);

                }
                else
                {
                    //Parse target for extraCosts
                    TargetChooserFactory tcf(g);
                    TargetChooser * tc = NULL;
                    size_t target_start = value.find("(");
                    size_t target_end = value.find(")");
                    if (target_start != string::npos && target_end != string::npos)
                    {
                        string target = value.substr(target_start + 1, target_end - 1 - target_start);
                        tc = tcf.createTargetChooser(target, c);
                    }

                    //switch on the first letter. If two costs share their first letter, add an "if" within the switch
                    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
                    switch (value[0])
                    {
                    case 'x':
                        if(value == "x")
                        {
                            manaCost->x();
                        }
                        else
                        {
                            vector<string>colorSplit = parseBetween(value,"x:"," ",false);
                            if(colorSplit.size())
                            {
                                int color = -1;
                                const string ColorStrings[] = { Constants::kManaColorless, Constants::kManaGreen, Constants::kManaBlue, Constants::kManaRed, Constants::kManaBlack, Constants::kManaWhite };
                                for (unsigned int i = 0; i < sizeof(ColorStrings)/sizeof(ColorStrings[0]); ++i)
                                {
                                    if (s.find(ColorStrings[i]) != string::npos)
                                    {
                                        color = i;
                                    }
                                }
                                manaCost->specificX(color);
                            }
                        }
                        break;
                    case 'v':
                        if (value.find("value:") != string::npos) {
                            vector<string> splitParsedVar = parseBetween(value, "value:", " ", false);
                            WParsedInt* res = NEW WParsedInt(splitParsedVar[1], NULL, c);
                            manaCost->add(Constants::MTG_COLOR_ARTIFACT, res->getValue());
                            SAFE_DELETE(res);
                        }
                        break;
                    case 't': //Tap
                        if (value == "t")
                        {
                            manaCost->addExtraCost(NEW TapCost);
                        }
                        else
                        {
                            manaCost->addExtraCost(NEW TapTargetCost(tc));
                        }
                        break;
                    case 's':
                        if (value.find("s2l") != string::npos)
                        { //Send To Library Cost (move from anywhere to Library)
                            manaCost->addExtraCost(NEW ToLibraryCost(tc));
                        }
                        else if (value.find("s2g") != string::npos)
                        { //Send to Graveyard Cost (move from anywhere to Graveyard)
                            manaCost->addExtraCost(NEW ToGraveCost(tc));
                        }
                        else
                        { //Sacrifice
                            manaCost->addExtraCost(NEW SacrificeCost(tc));
                        }
                        break;
                    case 'e': 
                        if (value == "emerge")
                        {
                            if (!tc)
                                tc = tcf.createTargetChooser("creature|mybattlefield", c);
                            manaCost->addExtraCost(NEW Offering(tc,true));
                        }
                        else if(value.substr(0,2) == "e:")
                        {//Energy Cost
                            vector<string>valSplit = parseBetween(value,"e:"," ",false);
                            if (valSplit.size()) {
                                WParsedInt* energytopay = NEW WParsedInt(valSplit[1], NULL, c);
                                manaCost->addExtraCost(NEW EnergyCost(energytopay->getValue()));
                                SAFE_DELETE(energytopay);
                            }
                        }
                        else
                        //Exile
                        manaCost->addExtraCost(NEW ExileTargetCost(tc));
                        break;
                    case 'h': //bounce (move to Hand)
                        manaCost->addExtraCost(NEW BounceTargetCost(tc));
                        break;
                    case 'l':
                        if (value == "l2e")
                        { //Mill to exile yourself as a cost (Library 2 Exile)
                            manaCost->addExtraCost(NEW MillExileCost(tc));
                        }
                        else if (value == "l")
                        { //Life cost
                            manaCost->addExtraCost(NEW LifeCost(tc));
                        }
                        else
                        { //Specific Life cost
                            vector<string>valSplit = parseBetween(value,"l:"," ",false);
                            if (valSplit.size()) {
                                WParsedInt* lifetopay = NEW WParsedInt(valSplit[1], NULL, c);
                                manaCost->addExtraCost(NEW SpecificLifeCost(tc,lifetopay->getValue()));
                                SAFE_DELETE(lifetopay);
                            }
                        }
                        break;
                    case 'd': //DiscardRandom cost
                        if (value.find("delve") != string::npos)
                        {
                            if(!tc)
                                tc = tcf.createTargetChooser("*|mygraveyard", c);
                            manaCost->addExtraCost(NEW Delve(tc));
                        }
                        else if (value == "d")
                        {
                            manaCost->addExtraCost(NEW DiscardRandomCost(tc));
                        }
                        else
                        {
                            manaCost->addExtraCost(NEW DiscardCost(tc));
                        }
                        break;
                    case 'm': //Mill yourself as a cost
                        manaCost->addExtraCost(NEW MillCost(tc));
                        break;
                    case 'n': //return unblocked attacker cost
                        {
                            TargetChooserFactory tcf(g);
                            tc = tcf.createTargetChooser("creature|myBattlefield", c);
                            manaCost->addExtraCost(NEW Ninja(tc));
                            break;
                        }
                    case 'k': //kill offering
                        {
                            TargetChooserFactory tcf(g);
                            if (value == "kgoblin")
                            {
                                tc = tcf.createTargetChooser("creature[goblin]|myBattlefield", c);
                            }
                            else if (value == "kfox")
                            {
                                tc = tcf.createTargetChooser("creature[fox]|myBattlefield", c);
                            }
                            else if (value == "kmoonfolk")
                            {
                                tc = tcf.createTargetChooser("creature[moonfolk]|myBattlefield", c);
                            }
                            else if (value == "krat")
                            {
                                tc = tcf.createTargetChooser("creature[rat]|myBattlefield", c);
                            }
                            else if (value == "ksnake")
                            {
                                tc = tcf.createTargetChooser("creature[snake]|myBattlefield", c);
                            }
                            //TODO iterate subtypes of creatures
                            manaCost->addExtraCost(NEW Offering(tc));
                            break;
                        }
                    case 'p' :
                        {
                            SAFE_DELETE(tc);
                            size_t start = value.find("(");
                            size_t end = value.rfind(")");
                            string manaType = value.substr(start + 1, end - start - 1);
                            manaCost->addExtraCost(NEW LifeorManaCost(NULL,manaType));
                            break;
                        }
                    case 'i' :
                        if(value == "improvise")
                        {
                            if(!tc)
                                tc = tcf.createTargetChooser("artifact[-tapped]|myBattlefield", c);
                            manaCost->addExtraCost(NEW Improvise(tc));
                        }
                        else
                        {
                            SAFE_DELETE(tc);
                            manaCost->add(0,1);
                            manaCost->addExtraCost(NEW SnowCost);
                        }
                        break;
                    case 'q':
                        if(value == "q")
                        {
                            manaCost->addExtraCost(NEW UnTapCost);
                        }
                        else
                        {
                            manaCost->addExtraCost(NEW UnTapTargetCost(tc));
                        }
                        break;
                    case 'c': //Counters or cycle
                        {
                            if (value.find("convoke") != string::npos)
                            {
                                if (!tc)
                                    tc = tcf.createTargetChooser("creature|mybattlefield", c);
                                manaCost->addExtraCost(NEW Convoke(tc));
                            }
                            else if(value == "chosencolor")
                            {
                                if(c)
                                manaCost->add(c->chooseacolor, 1);
                            }
                            else if(value == "cycle")
                            {
                                manaCost->addExtraCost(NEW CycleCost(tc));
                            }
                            else if(value.substr(0,4) == "crew")
                            {//tap crew
                                manaCost->addExtraCost(NEW TapTargetCost(tc,true));
                            }
                            else if(value.find("(") != string::npos)
                            {
                                size_t counter_start = value.find("(");
                                size_t counter_end = value.find(")", counter_start);
                                AbilityFactory abf(g);
                                string counterString = value.substr(counter_start + 1, counter_end - counter_start - 1);
                                Counter * counter = abf.parseCounter(counterString, c);
                                size_t separator = value.find(",", counter_start);
                                size_t separator2 = string::npos;
                                if (separator != string::npos)
                                {
                                    separator2 = value.find(",", counter_end + 1);
                                }
                                SAFE_DELETE(tc);
                                size_t target_start = string::npos;
                                if (separator2 != string::npos)
                                {
                                    target_start = value.find(",", counter_end + 1);
                                }
                                size_t target_end = value.length();
                                if (target_start != string::npos && target_end != string::npos)
                                {
                                    string target = value.substr(target_start + 1, target_end - 1 - target_start);
                                    tc = tcf.createTargetChooser(target, c);
                                }
                                manaCost->addExtraCost(NEW CounterCost(counter, tc));
                                break;
                            }
                            else if(value == "c")
                            {
                                manaCost->add(Constants::MTG_COLOR_WASTE, 1);
                                break;
                            }
                        break;
                    }
                    default: //uncolored cost and hybrid costs and special cost
                    {
                        if(value == "unattach")
                        {
                            manaCost->addExtraCost(NEW UnattachCost(c));
                            break;
                        }
                        int intvalue = atoi(value.c_str());
                        int colors[2];
                        int values[2];
                        if (intvalue < 10 && value.size() > 1)
                        {
                            for (int i = 0; i < 2; i++)
                            {
                                char c = value[i];
                                if (c >= '0' && c <= '9')
                                {
                                    colors[i] = Constants::MTG_COLOR_ARTIFACT;
                                    values[i] = c - '0';
                                }
                                else
                                {
                                    for (int j = 0; j < Constants::NB_Colors; j++)
                                    {
                                        if (c == Constants::MTGColorChars[j])
                                        {
                                            colors[i] = j;
                                            values[i] = 1;
                                        }
                                    }
                                }
                            }
                            if (values[0] > 0 || values[1] > 0)
                                manaCost->addHybrid(colors[0], values[0], colors[1], values[1]);
                        }
                        else
                        {
                            manaCost->add(Constants::MTG_COLOR_ARTIFACT, intvalue);
                        }
                        break;
                    }
                    }
                }
                s = s.substr(end + 1);
                state = 0;
            }
            break;
        default:
            break;
        }
    }
    return manaCost;
}

ManaCost::ManaCost()
{
    init();
}

ManaCost::ManaCost(vector<int16_t>& _cost, int nb_elems)
{
    init();
    for (int i = 0; i < nb_elems; i++)
    {
        cost[_cost[i * 2]] = _cost[i * 2 + 1];
    }

}

// pointer copy constructor 

ManaCost::ManaCost(ManaCost * manaCost)
{
    init();
    if ( !manaCost ) 
        return;
    for (int i = 0; i <= Constants::NB_Colors; i++)
    {
        cost[i] = manaCost->getCost(i);
    }
    hybrids = manaCost->hybrids;
    kicker = NEW ManaCost(manaCost->kicker);
    if (kicker)
            kicker->isMulti = manaCost->isMulti;
    Retrace = NEW ManaCost( manaCost->Retrace );
    BuyBack = NEW ManaCost( manaCost->BuyBack );
    alternative = NEW ManaCost( manaCost->alternative );
    FlashBack = NEW ManaCost( manaCost->FlashBack );
    morph = NEW ManaCost( manaCost->morph );
    suspend = NEW ManaCost( manaCost->suspend );
    Bestow = NEW ManaCost(manaCost->Bestow);
    extraCosts = NULL;
    if (manaCost->extraCosts)
    {
        extraCosts = manaCost->extraCosts->clone();
    }
    manaUsedToCast = NULL;
    xColor = manaCost->xColor;
}

// Copy Constructor 

ManaCost::ManaCost(const ManaCost& manaCost)
#ifdef TRACK_OBJECT_USAGE
    : InstanceCounter<ManaCost>(manaCost)
#endif
{
    for (int i = 0; i <= Constants::NB_Colors; i++)
    {
        cost.push_back(manaCost.cost[i]);      
    }
 
    hybrids = manaCost.hybrids;

    // make new copies of the pointers for the deep copy
    kicker = NEW ManaCost( manaCost.kicker );
    Retrace = NEW ManaCost( manaCost.Retrace );
    BuyBack = NEW ManaCost( manaCost.BuyBack );
    alternative = NEW ManaCost( manaCost.alternative );
    FlashBack = NEW ManaCost( manaCost.FlashBack );
    morph = NEW ManaCost( manaCost.morph );
    suspend = NEW ManaCost( manaCost.suspend );
    Bestow = NEW ManaCost(manaCost.Bestow);
    extraCosts = NULL;
    if (manaCost.extraCosts)
    {
        extraCosts = manaCost.extraCosts->clone();
    }

    manaUsedToCast = NULL;
    xColor = manaCost.xColor;
}

// operator=
ManaCost & ManaCost::operator= (const ManaCost & manaCost)
{
    if ( this != &manaCost )
    {
        for (int i = 0; i < Constants::NB_Colors; i++)
            cost[i] = manaCost.cost[i];

        hybrids = manaCost.hybrids;
        extraCosts = manaCost.extraCosts;
        kicker = manaCost.kicker;
        Retrace = manaCost.Retrace;
        BuyBack = manaCost.BuyBack;
        alternative = manaCost.alternative;
        FlashBack = manaCost.FlashBack;
        morph = manaCost.morph;
        suspend = manaCost.suspend;
        Bestow = manaCost.Bestow;
        manaUsedToCast = manaCost.manaUsedToCast;
        xColor = manaCost.xColor;
    }
    return *this;
}

ManaCost::~ManaCost()
{
    SAFE_DELETE(extraCosts);
    SAFE_DELETE(kicker);
    SAFE_DELETE(alternative);
    SAFE_DELETE(BuyBack);
    SAFE_DELETE(FlashBack);
    SAFE_DELETE(Retrace);
    SAFE_DELETE(morph);
    SAFE_DELETE(suspend);
    SAFE_DELETE(Bestow);
    SAFE_DELETE(manaUsedToCast);

    cost.erase(cost.begin() ,cost.end());
}

void ManaCost::x()
{
    if (cost.size() <= (size_t)Constants::NB_Colors)
    {
        return;
    }

    cost[Constants::NB_Colors] = 1;
}

int ManaCost::hasX()
{
    if (cost.size() <= (size_t)Constants::NB_Colors)
    {
        return 0;
    }
    if (xColor > 0)
        return 0;

    return cost[Constants::NB_Colors];
}

void ManaCost::specificX(int color)
{
    if (cost.size() <= (size_t)Constants::NB_Colors)
    {
        return;
    }
    xColor = color;
    cost[Constants::NB_Colors] = 1;
}

int ManaCost::hasSpecificX()
{
    if (cost.size() <= (size_t)Constants::NB_Colors)
    {
        return 0;
    }
    if(xColor > 0)
        return cost[Constants::NB_Colors];
    return 0;
}

int ManaCost::hasAnotherCost()
{
    if (cost.size() <= (size_t)Constants::NB_Colors)
    {
        DebugTrace("Seems ManaCost was not properly initialized");
        return 0;
    }
    int result = 0;
    if(kicker)
        result = 1;
    //kicker is the only one ai knows for now, later hasAnotherCost() can be used to determine other cost types.
    return result;
}

void ManaCost::init()
{
    int i;
    
    cost.erase(cost.begin() ,cost.end());
    
    for (i = 0; i <= Constants::NB_Colors; i++)
    {
        cost.push_back(0);
    }
    
    extraCosts = NULL;
    kicker = NULL;
    alternative = NULL;
    BuyBack = NULL;
    FlashBack = NULL;
    Retrace = NULL;
    morph = NULL;
    suspend = NULL;
    Bestow = NULL;
    manaUsedToCast = NULL;
    isMulti = false;
    xColor = -1;
}

void ManaCost::resetCosts()
{
    int i;
    
    cost.erase(cost.begin() ,cost.end());

    for (i = 0; i <= Constants::NB_Colors; i++)
    {
        cost.push_back(0);
    }
    
    SAFE_DELETE(extraCosts);
    SAFE_DELETE(kicker);
    SAFE_DELETE(alternative);
    SAFE_DELETE(BuyBack);
    SAFE_DELETE(FlashBack);
    SAFE_DELETE(Retrace);
    SAFE_DELETE(morph);
    SAFE_DELETE(suspend);
    SAFE_DELETE(Bestow);
}

void ManaCost::copy(ManaCost * _manaCost)
{
    if (!_manaCost)
        return;

    cost.erase(cost.begin() ,cost.end());

    for (int i = 0; i <= Constants::NB_Colors; i++)
    {
        cost.push_back(_manaCost->getCost(i));
    }

    hybrids = _manaCost->hybrids;

    SAFE_DELETE(extraCosts);

    if (_manaCost->extraCosts)
    {
        extraCosts = _manaCost->extraCosts->clone();
    }

    SAFE_DELETE(kicker);
    if (_manaCost->kicker)
    {
        kicker = NEW ManaCost();
        kicker->copy(_manaCost->kicker);
        kicker->isMulti = _manaCost->kicker->isMulti;
    }
    SAFE_DELETE(alternative);
    if (_manaCost->alternative)
    {
        alternative = NEW ManaCost();
        alternative->copy(_manaCost->alternative);
    }
    SAFE_DELETE(BuyBack);
    if (_manaCost->BuyBack)
    {
        BuyBack = NEW ManaCost();
        BuyBack->copy(_manaCost->BuyBack);
    }
    SAFE_DELETE(FlashBack);
    if (_manaCost->FlashBack)
    {
        FlashBack = NEW ManaCost();
        FlashBack->copy(_manaCost->FlashBack);
    }
    SAFE_DELETE(Retrace);
    if (_manaCost->Retrace)
    {
        Retrace = NEW ManaCost();
        Retrace->copy(_manaCost->Retrace);
    }
    SAFE_DELETE(morph);
    if (_manaCost->morph)
    {
        morph = NEW ManaCost();
        morph->copy(_manaCost->morph);
    }
    SAFE_DELETE(suspend);
    if (_manaCost->suspend)
    {
        suspend = NEW ManaCost();
        suspend->copy(_manaCost->suspend);
    }
    SAFE_DELETE(Bestow);
    if (_manaCost->Bestow)
    {
        Bestow = NEW ManaCost();
        Bestow->copy(_manaCost->Bestow);
    }
    xColor = _manaCost->xColor;
}

void ManaCost::changeCostTo(ManaCost * _manaCost)
{
    if (!_manaCost)
        return;

    cost.erase(cost.begin() ,cost.end());

    for (int i = 0; i <= Constants::NB_Colors; i++)
    {
        cost.push_back(_manaCost->getCost(i));
    }

    hybrids = _manaCost->hybrids;

    SAFE_DELETE(extraCosts);

    if (_manaCost->extraCosts)
    {
        extraCosts = _manaCost->extraCosts->clone();
    }
    
    xColor = _manaCost->xColor;
}

int ManaCost::getCost(int color)
{
    if (cost.size() <= (size_t)color)
    {
        DebugTrace("Seems ManaCost was not properly initialized");
        return 0;
    }
    return cost[color];
}

int ManaCost::getManaSymbols(int color)
{
    int result = cost[color];
    for (size_t i = 0; i < hybrids.size(); ++i)
    {
        result += hybrids[i].getManaSymbols(color);
    }
    if (extraCosts && extraCosts->costs.size())
    {
        for (size_t i = 0; i < extraCosts->costs.size(); ++i)
        {
            LifeorManaCost * phyrexianMana = dynamic_cast<LifeorManaCost*>(extraCosts->costs[i]);
            if (phyrexianMana)
            {
                result += phyrexianMana->getManaCost()->getManaSymbols(color);
            }
        }
    }
    return result;
}

int ManaCost::getManaSymbolsHybridMerged(int color)
{
    int result = cost[color];
    for (size_t i = 0; i < hybrids.size(); ++i)
    {
        result += hybrids[i].getManaSymbolsHybridMerged(color);
    }
    if (extraCosts && extraCosts->costs.size())
    {
        for (size_t i = 0; i < extraCosts->costs.size(); ++i)
        {
            LifeorManaCost * phyrexianMana = dynamic_cast<LifeorManaCost*>(extraCosts->costs[i]);
            if (phyrexianMana)
            {
                result += phyrexianMana->getManaCost()->getManaSymbolsHybridMerged(color);
            }
        }
    }
    return result;
}

int ManaCost::countHybridsNoPhyrexian()
{
    int result = 0;
    for (size_t i = 0; i < hybrids.size(); i++)
        result ++;
    return result;
}

void ManaCost::removeHybrid(ManaCost * _manaCost)
{
    if (!_manaCost)
        return;

    vector<int> colors;
    int match = 0;

    for(int j = 0; j < 7; j++)
    {//populate colors values
        colors.push_back(_manaCost->getCost(j));
    }

    for (size_t i = 0; i < hybrids.size(); i++)
    {
        for(int j = 0; j < 7; j++)
        {
            if(colors[j])
            {
                if(hybrids[i].hasColor(j))
                {
                    hybrids[i].reduceValue(j, colors[j]);
                    colors[j] -= 1;
                    match++;
                }
            }
        }
    }
    return;
}

int ManaCost::parseManaSymbol(char symbol)
{
    switch (symbol)
    {
    case 'g':
        return Constants::MTG_COLOR_GREEN;
    case 'u':
        return Constants::MTG_COLOR_BLUE;
    case 'r':
        return Constants::MTG_COLOR_RED;
    case 'b':
        return Constants::MTG_COLOR_BLACK;
    case 'w':
        return Constants::MTG_COLOR_WHITE;
    }
    DebugTrace( "Failed to parse mana symbol" );
    return -1;
}

ManaCostHybrid * ManaCost::getHybridCost(unsigned int i)
{
    if (hybrids.size() <= i)
        return NULL;
    return &hybrids[i];
}

ExtraCost * ManaCost::getExtraCost(unsigned int i)
{
    if(extraCosts && extraCosts->costs.size())
    {
        if (extraCosts->costs.size() <= i)
            return NULL;
        return extraCosts->costs[i];
    }
    return NULL;
}

int ManaCost::hasColor(int color)
{
    if (getCost(color))
        return 1;
    for (size_t i = 0; i < hybrids.size(); i++)
    {
        if (hybrids[i].hasColor(color))
            return 1;
    }
    return 0;
}

int ManaCost::isNull()
{
    if (getConvertedCost())
        return 0;
    if (extraCosts)
        return 0;
    return 1;
}

int ManaCost::getConvertedCost()
{
    int result = 0;
    for (int i = 0; i < Constants::NB_Colors; i++)
    {
        result += cost[i];
    }
    for (size_t i = 0; i < hybrids.size(); i++)
    {
        result += hybrids[i].getConvertedCost();
    }
    if (extraCosts && extraCosts->costs.size())
    {
        for (unsigned int i = 0; i < extraCosts->costs.size(); i++)
        {
            ExtraCost * pMana = dynamic_cast<LifeorManaCost*>(extraCosts->costs[i]);
            if (pMana)
                result++;
            //snow cost???
            ExtraCost * sMana = dynamic_cast<SnowCost*>(extraCosts->costs[i]);
            if (sMana)
                result++;
        }
    }

    return result;
}

int ManaCost::remove(int color, int value)
{
    assert (value >= 0);
    int16_t toRemove = min(cost[color], (int16_t)value);
    cost[color] -= toRemove;
    return 1;
}

int ManaCost::add(int color, int value)
{
    if (value < 0)
        value = 0;
    cost[color] += value;
    return 1;
}

int ManaCost::add(ManaCost * _cost)
{
    if (!_cost)
        return 0;
    for ( int i = 0; i < Constants::NB_Colors; i++)
    {
        cost[i] += _cost->getCost(i);
    }

    std::copy(_cost->hybrids.begin(), _cost->hybrids.end(), std::back_inserter(hybrids));

    return 1;
}

int ManaCost::remove(ManaCost * _cost)
{
    if (!_cost)
        return 0;
    for ( int i = 0; i < Constants::NB_Colors; i++)
    {
        int16_t toRemove = min(cost[i], (int16_t)_cost->getCost(i)); //we don't want to be negative
        cost[i] -= toRemove;
        assert(cost[i] >= 0);
    }
    return 1;
}

int ManaCost::removeAll(int color)
{
    cost[color] = 0;
    return 1;
}

int ManaCost::addHybrid(int c1, int v1, int c2, int v2)
{
    hybrids.push_back(ManaCostHybrid(c1, v1, c2, v2));
    return hybrids.size();
}

int ManaCost::addExtraCost(ExtraCost * _cost)
{
    if (!extraCosts)
        extraCosts = NEW ExtraCosts();
    extraCosts->costs.push_back(_cost);
    return 1;
}

int ManaCost::addExtraCosts(ExtraCosts *_ecost)
{
    if(!_ecost)
    {
        extraCosts = NULL;
        return 1;
    }
    if (!extraCosts)
        extraCosts = NEW ExtraCosts();
    for(size_t i = 0; i < _ecost->costs.size(); i++)
        extraCosts->costs.push_back(_ecost->costs[i]->clone());
    return 1;
}

int ManaCost::isExtraPaymentSet()
{
    if (!extraCosts)
        return 1;
    return extraCosts->isPaymentSet();
}

int ManaCost::canPayExtra()
{
    if (!extraCosts)
        return 1;
    return extraCosts->canPay();
}

int ManaCost::doPayExtra()
{
    if (!extraCosts)
        return 0;
    return extraCosts->doPay(); //TODO reset ?
}

int ManaCost::setExtraCostsAction(MTGAbility * action, MTGCardInstance * card)
{
    if (extraCosts)
        extraCosts->setAction(action, card);
    return 1;
}

int ManaCost::pay(ManaCost * _cost)
{
    int result = MANA_PAID;
    ManaCost * toPay = NEW ManaCost();
    toPay->copy(_cost);
    ManaCost * diff = Diff(toPay);
    for (int i = 0; i < Constants::NB_Colors; i++)
    {
        cost[i] = diff->getCost(i);
    }
    delete diff;
    delete toPay;
    return result;
    //TODO return 0 if can't afford the cost!
}

//return 1 if _cost can be paid with current data, 0 otherwise
int ManaCost::canAfford(ManaCost * _cost)
{
    ManaCost * diff = Diff(_cost);
    int positive = diff->isPositive();
    delete diff;
    if (positive)
    {
        return 1;
    }
    return 0;
}

int ManaCost::isPositive()
{
    for (int i = 0; i < Constants::NB_Colors; i++)
    {

        if (cost[i] < 0)
        {
            return 0;
        }
    }
    return 1;

}

void ManaCost::randomDiffHybrids(ManaCost * _cost, std::vector<int16_t>& diff)
{
    for (size_t i = 0; i < _cost->hybrids.size(); i++)
    {
        ManaCostHybrid& h = _cost->hybrids[i];
        diff[h.color1 * 2 + 1] -= h.value1;
    }
}

/**
    starting from the end of the array (diff) 
*/
int ManaCost::tryToPayHybrids(const std::vector<ManaCostHybrid>& _hybrids, int _nbhybrids, std::vector<int16_t>& diff)
{
    if (!_nbhybrids)
        return 1;
    int result = 0;
    const ManaCostHybrid& h = _hybrids[_nbhybrids - 1];
    if (diff[h.color1 * 2 + 1] >= h.value1)
    {
        diff[h.color1 * 2 + 1] -= h.value1;
        result = tryToPayHybrids(_hybrids, _nbhybrids - 1, diff);
        if (result)
            return 1;
        diff[h.color1 * 2 + 1] += h.value1;
    }
    if (diff[h.color2 * 2 + 1] >= h.value2)
    {
        diff[h.color2 * 2 + 1] -= h.value2;
        result = tryToPayHybrids(_hybrids, _nbhybrids - 1, diff);
        if (result)
            return 1;
        diff[h.color2 * 2 + 1] += h.value2;
    }
    return 0;
}

//compute the difference between two mana costs
ManaCost * ManaCost::Diff(ManaCost * _cost)
{
    if (!_cost) 
        return NEW ManaCost(*this); //diff with null is equivalent to diff with 0

    vector<int16_t> diff;
    diff.resize((Constants::NB_Colors + 1) * 2);
    diff[Constants::NB_Colors * 2] = Constants::NB_Colors;
    for (int i = 0; i < Constants::NB_Colors; i++)
    {
        diff[i * 2] = i;
        diff[i * 2 + 1] = cost[i] - _cost->getCost(i);
    }
    int hybridResult = tryToPayHybrids(_cost->hybrids, _cost->hybrids.size(), diff);
    if (!hybridResult)
        randomDiffHybrids(_cost, diff);

    //Colorless mana, special case
    int colorless_idx = Constants::MTG_COLOR_ARTIFACT * 2 + 1;
    if (diff[colorless_idx] < 0)
    {
        for (int i = 0; i < Constants::NB_Colors; i++)
        {
            if (diff[i * 2 + 1] > 0)
            {
                if (diff[i * 2 + 1] + diff[colorless_idx] > 0)
                {
                    diff[i * 2 + 1] += diff[colorless_idx];
                    diff[colorless_idx] = 0;
                    break;
                }
                else
                {
                    diff[colorless_idx] += diff[i * 2 + 1];
                    diff[i * 2 + 1] = 0;
                }
            }
        }
    }

    //Cost X
    if (_cost->hasX())
    {
        diff[Constants::NB_Colors * 2 + 1] = 0;
        for (int i = 0; i < Constants::NB_Colors; i++)
        {
            if (diff[i * 2 + 1] > 0)
            {
                diff[Constants::NB_Colors * 2 + 1] += diff[i * 2 + 1];
                diff[i * 2 + 1] = 0;
            }
        }
    }
    //cost x where x is specific.
    if (_cost->hasSpecificX())
    {
        diff[Constants::NB_Colors * 2 + 1] = 0;
        if (diff[_cost->xColor * 2 + 1] > 0)
        {
            diff[Constants::NB_Colors * 2 + 1] += diff[_cost->xColor * 2 + 1];
            diff[_cost->xColor * 2 + 1] = 0;
        }
    }

    ManaCost * result = NEW ManaCost(diff, Constants::NB_Colors + 1);
    return result;

}

string ManaCost::toString()
{
    ostringstream oss;
    for (int i = 0; i <= Constants::NB_Colors; i++)
    {
        if (cost[i])
        {
            if ( i == Constants::MTG_COLOR_ARTIFACT)
                oss << "{" << getCost(i) << "}";
            else
                for (int colorCount = 0; colorCount < getCost(i); colorCount++ )
                    oss << "{" << Constants::MTGColorChars[i] << "}";
        }
    }

    for (size_t i = 0; i < hybrids.size(); i++)
    {
        oss << hybrids[i];
    }
    return oss.str();
}

#ifdef WIN32
void ManaCost::Dump()
{
    //if(this->getConvertedCost())//uncomment when this is far too loud and clutters your output making other traces pointless.
    //{
    DebugTrace( "\n===ManaCost===" );
    DebugTrace( this->toString() );
    DebugTrace( "\n=============" );
    //}
}

#endif

ostream& operator<<(ostream& out, ManaCost& m)
{
    return out << m.toString();
}

ostream& operator<<(ostream& out, ManaCost* m)
{
    return out << m->toString();
}

ostream& operator<<(ostream& out, ManaCost m)
{
    return out << m.toString();
}

void ManaPool::Empty()
{
    SAFE_DELETE(extraCosts);
    SAFE_DELETE(kicker);
    SAFE_DELETE(alternative);
    SAFE_DELETE(BuyBack);
    SAFE_DELETE(FlashBack);
    SAFE_DELETE(Retrace);
    SAFE_DELETE(morph);
    SAFE_DELETE(suspend);
    SAFE_DELETE(Bestow);
    SAFE_DELETE(manaUsedToCast);
    init();
    WEvent * e = NEW WEventEmptyManaPool(this);
    player->getObserver()->receiveEvent(e);
}

ManaPool::ManaPool(Player * player) :
    ManaCost(), player(player)
{
}

ManaPool::ManaPool(ManaCost * _manaCost, Player * player) :
    ManaCost(_manaCost), player(player)
{
}

int ManaPool::remove(int color, int value)
{
    int result = ManaCost::remove(color, value);
    for (int i = 0; i < value; ++i)
    {
        WEvent * e = NEW WEventConsumeMana(color, this);
        player->getObserver()->receiveEvent(e);
    }
    return result;
}

int ManaPool::add(int color, int value, MTGCardInstance * source, bool extra)
{
    if (color == Constants::MTG_COLOR_ARTIFACT)
        color = Constants::MTG_COLOR_WASTE;
    int result = ManaCost::add(color, value);
    for (int i = 0; i < value; ++i)
    {
        WEvent * e = NEW WEvent;

        if(extra)
            e = NEW WEventEngageManaExtra(color, source, this);
        else
            e = NEW WEventEngageMana(color, source, this);

        player->getObserver()->receiveEvent(e);
    }
    return result;
}

int ManaPool::add(ManaCost * _cost, MTGCardInstance * source)
{
    if (!_cost)
        return 0;
    //while colorless is still exactly the same, there are now cards that require 
    //true colorless mana, ei:eldrazi. so whenever we add mana, we now replace it with the
    //new type. keeping the old type intact for payment methods {1}{c} ....
    int replaceArtifact = _cost->getCost(Constants::MTG_COLOR_ARTIFACT);
    if (replaceArtifact)
    {
        _cost->add(Constants::MTG_COLOR_WASTE, replaceArtifact);
        _cost->remove(Constants::MTG_COLOR_ARTIFACT, replaceArtifact);
    }
    int result = ManaCost::add(_cost);
    for (int i = 0; i < Constants::NB_Colors; i++)
    {
        for (int j = 0; j < _cost->getCost(i); j++)
        {
            WEvent * e = NEW WEventEngageMana(i, source, this);
            player->getObserver()->receiveEvent(e);
        }
    }
    return result;
}

int ManaPool::pay(ManaCost * _cost)
{
    vector<int> current;
    for (int i = 0; i < Constants::NB_Colors; i++)
    {
        current.push_back(cost[i]);
    }

    int result = ManaCost::pay(_cost);
    for (int i = 0; i < Constants::NB_Colors; i++)
    {
        int value = current[i] - cost[i];
        for (int j = 0; j < value; j++)
        {
            WEvent * e = NEW WEventConsumeMana(i, this);
            player->getObserver()->receiveEvent(e);

        }
    }
    current.erase(current.begin(),current.end());
    return result;
}
