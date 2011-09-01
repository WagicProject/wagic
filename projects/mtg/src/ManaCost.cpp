#include "PrecompiledHeader.h"

#include "ManaCost.h"
#include "ManaCostHybrid.h"
#include "ExtraCost.h"
#include "TargetChooser.h"
#include "Targetable.h"
#include "Player.h"
#include "WEvent.h"
#include "MTGAbility.h"

#include "iterator"

SUPPORT_OBJECT_ANALYTICS(ManaCost)

ManaCost * ManaCost::parseManaCost(string s, ManaCost * _manaCost, MTGCardInstance * c)
{
    ManaCost * manaCost;
    if (_manaCost)
    {
        manaCost = _manaCost;
    }
    else
    {
        manaCost = NEW ManaCost();
    }
    int state = 0;
    size_t start = 0;
    size_t end = 0;
    while (!s.empty() && state != -1)
    {
        switch (state)
        {
        case 0:
            start = s.find_first_of("{");
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
                    TargetChooserFactory tcf;
                    TargetChooser * tc = NULL;
                    size_t target_start = value.find("(");
                    size_t target_end = value.find(")");
                    if (target_start != string::npos && target_end != string::npos)
                    {
                        string target = value.substr(target_start + 1, target_end - 1 - target_start);
                        tc = tcf.createTargetChooser(target, c);
                    }

                    //switch on the first letter. If two costs share their first letter, add an "if" within the switch
                    switch (value[0])
                    {
                    case 'x':
                        manaCost->x();
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
                        if (value == "s2l")
                        { //Send To Library Cost (move from anywhere to Library)
                            manaCost->addExtraCost(NEW ToLibraryCost(tc));
                        }
                        else
                        { //Sacrifice
                            manaCost->addExtraCost(NEW SacrificeCost(tc));
                        }
                        break;
                    case 'e': //Exile
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
                        else
                        { //Life cost
                            manaCost->addExtraCost(NEW LifeCost(tc));
                        }
                        break;
                    case 'd': //DiscardRandom cost
                        if (value == "d")
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
                        TargetChooserFactory tcf;
                        tc = tcf.createTargetChooser("creature|myBattlefield", c);
                        manaCost->addExtraCost(NEW Ninja(tc));
                        break;
                    case 'p' :
                        {
                            SAFE_DELETE(tc);
                            size_t start = value.find("(");
                            size_t end = value.rfind(")");
                            string manaType = value.substr(start + 1, end - start - 1);
                            manaCost->addExtraCost(NEW LifeorManaCost(NULL,manaType));
                            break;
                        }
                    case 'q':
                        {
                            manaCost->addExtraCost(NEW UnTapCost);
                            break;
                        }
                    case 'c': //Counters
                    {
                        size_t counter_start = value.find("(");
                        size_t counter_end = value.find(")", counter_start);
                        AbilityFactory abf;
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
                    default: //uncolored cost and hybrid costs
                    {
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
                                    for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
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

ManaCost::ManaCost(int8_t _cost[], int nb_elems)
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
    for (int i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = manaCost->getCost(i);
    }
    hybrids = manaCost->hybrids;

    kicker = NEW ManaCost( manaCost->kicker );
    if(kicker)
    kicker->isMulti = manaCost->isMulti;
    Retrace = NEW ManaCost( manaCost->Retrace );
    BuyBack = NEW ManaCost( manaCost->BuyBack );
    alternative = NEW ManaCost( manaCost->alternative );
    FlashBack = NEW ManaCost( manaCost->FlashBack );
    morph = NEW ManaCost( manaCost->morph );
    suspend = NEW ManaCost( manaCost->suspend );

    extraCosts = manaCost->extraCosts ? manaCost->extraCosts->clone() : NULL;
}

// Copy Constructor 

ManaCost::ManaCost(const ManaCost& manaCost)
#ifdef TRACK_OBJECT_USAGE
    : InstanceCounter<ManaCost>(manaCost)
#endif
{
    for (int i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = manaCost.cost[i];      
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
    
    extraCosts = manaCost.extraCosts ? manaCost.extraCosts->clone() : NULL;
}

// operator=
ManaCost & ManaCost::operator= (const ManaCost & manaCost)
{
    if ( this != &manaCost )
    {
        for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
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
}

void ManaCost::x()
{
    cost[Constants::MTG_NB_COLORS] = 1;
}

int ManaCost::hasX()
{
    return cost[Constants::MTG_NB_COLORS];
}

void ManaCost::init()
{
    int i;
    for (i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = 0;
    }
    extraCosts = NULL;
    kicker = NULL;
    alternative = NULL;
    BuyBack = NULL;
    FlashBack = NULL;
    Retrace = NULL;
    morph = NULL;
    suspend = NULL;
    isMulti = false;
}

void ManaCost::reinit()
{
    int i;
    for (i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = 0;
    }
    SAFE_DELETE(extraCosts);
    SAFE_DELETE(kicker);
    SAFE_DELETE(alternative);
    SAFE_DELETE(BuyBack);
    SAFE_DELETE(FlashBack);
    SAFE_DELETE(Retrace);
    SAFE_DELETE(morph);
    SAFE_DELETE(suspend);
}

void ManaCost::copy(ManaCost * _manaCost)
{
    if (!_manaCost)
        return;
    for (unsigned int i = 0; i <= Constants::MTG_NB_COLORS; i++)
    {
        cost[i] = _manaCost->getCost(i);
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
}

int ManaCost::getCost(int color)
{
    return cost[color];
}

ManaCostHybrid * ManaCost::getHybridCost(unsigned int i)
{
    if (hybrids.size() <= i)
        return NULL;
    return &hybrids[i];
}

int ManaCost::hasColor(int color)
{
    if (cost[color])
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
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        result += cost[i];
    }
    for (size_t i = 0; i < hybrids.size(); i++)
    {
        result += hybrids[i].getConvertedCost();
    }
	if(extraCosts && extraCosts->costs.size())
	{
		for(unsigned int i = 0; i < extraCosts->costs.size();i++)
		{
			ExtraCost * pMana = dynamic_cast<LifeorManaCost*>(extraCosts->costs[i]);
			if(pMana)
				result++;
		}
	}

    return result;
}

int ManaCost::remove(int color, int value)
{
    assert (value >= 0);
    int8_t toRemove = min(cost[color], (int8_t)value);
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
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
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
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        int8_t toRemove = min(cost[i], (int8_t)_cost->getCost(i)); //we don't want to be negative
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
    for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
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
    for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {

        if (cost[i] < 0)
        {
            return 0;
        }
    }
    return 1;

}

void ManaCost::randomDiffHybrids(ManaCost * _cost, int8_t diff[])
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
int ManaCost::tryToPayHybrids(std::vector<ManaCostHybrid>& _hybrids, int _nbhybrids, int8_t diff[])
{
    if (!_nbhybrids)
        return 1;
    int result = 0;
    ManaCostHybrid& h = _hybrids[_nbhybrids - 1];
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

    int8_t diff[(Constants::MTG_NB_COLORS + 1) * 2];
    diff[Constants::MTG_NB_COLORS * 2] = Constants::MTG_NB_COLORS;
    for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
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
        for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
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
        diff[Constants::MTG_NB_COLORS * 2 + 1] = 0;
        for (int i = 0; i < Constants::MTG_NB_COLORS; i++)
        {
            if (diff[i * 2 + 1] > 0)
            {
                diff[Constants::MTG_NB_COLORS * 2 + 1] += diff[i * 2 + 1];
                diff[i * 2 + 1] = 0;
            }
        }
    }

    ManaCost * result = NEW ManaCost(diff, Constants::MTG_NB_COLORS + 1);
    return result;

}

string ManaCost::toString()
{
    ostringstream oss;
    for (int i = 0; i <= Constants::MTG_NB_COLORS; i++)
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

void ManaPool::init()
{
    ManaCost::init();
    WEvent * e = NEW WEventEmptyManaPool(this);
    GameObserver::GetInstance()->receiveEvent(e);
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
        GameObserver::GetInstance()->receiveEvent(e);
    }
    return result;
}

int ManaPool::add(int color, int value, MTGCardInstance * source)
{
    int result = ManaCost::add(color, value);
    for (int i = 0; i < value; ++i)
    {
        WEvent * e = NEW WEventEngageMana(color, source, this);
        GameObserver::GetInstance()->receiveEvent(e);
    }
    return result;
}

int ManaPool::add(ManaCost * _cost, MTGCardInstance * source)
{
    if (!_cost)
        return 0;
    int result = ManaCost::add(_cost);
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        for (int j = 0; j < _cost->getCost(i); j++)
        {
            WEvent * e = NEW WEventEngageMana(i, source, this);
            GameObserver::GetInstance()->receiveEvent(e);
        }
    }
    return result;
}

int ManaPool::pay(ManaCost * _cost)
{
    int current[Constants::MTG_NB_COLORS];
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        current[i] = cost[i];
    }

    int result = ManaCost::pay(_cost);
    for (unsigned int i = 0; i < Constants::MTG_NB_COLORS; i++)
    {
        int value = current[i] - cost[i];
        for (int j = 0; j < value; j++)
        {
            WEvent * e = NEW WEventConsumeMana(i, this);
            GameObserver::GetInstance()->receiveEvent(e);

        }
    }
    return result;
}
