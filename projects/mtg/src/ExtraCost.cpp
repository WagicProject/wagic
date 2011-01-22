#include "PrecompiledHeader.h"

#include "ExtraCost.h"
#include "TargetChooser.h"
#include "MTGCardInstance.h"
#include "Translate.h"
#include "Player.h"
#include "Counters.h"

ExtraCost::ExtraCost(const std::string& inCostRenderString, TargetChooser *_tc) :
    tc(_tc), source(NULL), target(NULL), mCostRenderString(inCostRenderString)
{
    if (tc)
        tc->targetter = NULL;
}

ExtraCost::~ExtraCost()
{
    SAFE_DELETE(tc);
}

int ExtraCost::setSource(MTGCardInstance * _source)
{
    source = _source;
    if (tc)
    {
        tc->source = _source;
        // "extra cost is not targetting, protections do not apply" this is not cryptic at all :) make an ability you will understand it then. this keeps the target chooser from being unable to select a creature with shroud/protections.
        tc->targetter = NULL;
    }
    else
    {
        target = _source;
    }
    return 1;
}

void ExtraCost::Render()
{
    if (!mCostRenderString.empty())
    {
        WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
        mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
        mFont->SetColor(ARGB(255,255,255,255));
        mFont->DrawString(mCostRenderString, 20, 20, JGETEXT_LEFT);
    }
}

int ExtraCost::setPayment(MTGCardInstance * card)
{
    int result = 0;
    if (tc)
    {
        result = tc->addTarget(card);
        if (result)
        {
            target = card;
        }
    }
    return result;
}

//life cost
LifeCost * LifeCost::clone() const
{
    LifeCost * ec = NEW LifeCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

LifeCost::LifeCost(TargetChooser *_tc) :
    ExtraCost("Life", _tc)
{
}

int LifeCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        _target->controller()->thatmuch = 1;
        WEvent * lifed = NULL;
        lifed = NEW WEventLife(_target->controller(),-1,1);
        GameObserver * game = GameObserver::GetInstance();
        game->receiveEvent(lifed);
        _target->controller()->life -= 1;
        _target->controller()->lifeLostThisTurn += 1;
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//discard a card at random as a cost
//DiscardRandom cost
DiscardRandomCost * DiscardRandomCost::clone() const
{
    DiscardRandomCost * ec = NEW DiscardRandomCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

DiscardRandomCost::DiscardRandomCost(TargetChooser *_tc) :
    ExtraCost("Discard Random", _tc)
{
}

int DiscardRandomCost::canPay()
{
    MTGGameZone * z = target->controller()->game->hand;
    int nbcards = z->nb_cards;
    if (nbcards < 1)
        return 0;
    if (nbcards == 1 && z->hasCard(source))
        return 0;
    return 1;
}

int DiscardRandomCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        _target->controller()->game->discardRandom(_target->controller()->game->hand, source);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}
//a choosen discard

DiscardCost * DiscardCost::clone() const
{
    DiscardCost * ec = NEW DiscardCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

DiscardCost::DiscardCost(TargetChooser *_tc) :
    ExtraCost("Choose card to Discard", _tc)
{
}

int DiscardCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        WEvent * e = NEW WEventCardDiscard(target);
        GameObserver * game = GameObserver::GetInstance();
        game->receiveEvent(e);
        _target->controller()->game->putInGraveyard(_target);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}
//to library cost

ToLibraryCost * ToLibraryCost::clone() const
{
    ToLibraryCost * ec = NEW ToLibraryCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

ToLibraryCost::ToLibraryCost(TargetChooser *_tc) :
    ExtraCost("Put a card on top of Library", _tc)
{
}

int ToLibraryCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        _target->controller()->game->putInLibrary(target);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//Mill yourself as a cost
MillCost * MillCost::clone() const
{
    MillCost * ec = NEW MillCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

MillCost::MillCost(TargetChooser *_tc) :
    ExtraCost("Deplete", _tc)
{
}

int MillCost::canPay()
{
    MTGGameZone * z = target->controller()->game->library;
    int nbcards = z->nb_cards;
    if (nbcards < 1)
        return 0;
    return 1;
}

int MillCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        _target->controller()->game->putInZone(
                        _target->controller()->game->library->cards[_target->controller()->game->library->nb_cards - 1],
                        _target->controller()->game->library, _target->controller()->game->graveyard);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

MillExileCost::MillExileCost(TargetChooser *_tc) :
    MillCost(_tc)
{
    // override the base string here
    mCostRenderString = "Deplete To Exile";
}

int MillExileCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        _target->controller()->game->putInZone(
                        _target->controller()->game->library->cards[_target->controller()->game->library->nb_cards - 1],
                        _target->controller()->game->library, _target->controller()->game->exile);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//Tap target cost
TapTargetCost * TapTargetCost::clone() const
{
    TapTargetCost * ec = NEW TapTargetCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

TapTargetCost::TapTargetCost(TargetChooser *_tc) :
    ExtraCost("Tap Target", _tc)
{
}

int TapTargetCost::isPaymentSet()
{
    if (target && target->isTapped())
    {
        tc->removeTarget(target);
        target->isExtraCostTarget = false;
        target = NULL;
        return 0;
    }
    if (target)
        return 1;
    return 0;
}

int TapTargetCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        _target->tap();
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//exile as cost
ExileTargetCost * ExileTargetCost::clone() const
{
    ExileTargetCost * ec = NEW ExileTargetCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

ExileTargetCost::ExileTargetCost(TargetChooser *_tc) :
    ExtraCost("Exile Target", _tc)
{
}

int ExileTargetCost::doPay()
{

    if (target)
    {
        target->controller()->game->putInExile(target);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//Bounce as cost
BounceTargetCost * BounceTargetCost::clone() const
{
    BounceTargetCost * ec = NEW BounceTargetCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

BounceTargetCost::BounceTargetCost(TargetChooser *_tc) :
    ExtraCost("Return Target to Hand", _tc)
{
}

int BounceTargetCost::doPay()
{

    if (target)
    {
        target->controller()->game->putInHand(target);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//Bounce as cost for ninja
Ninja * Ninja::clone() const
{
    Ninja * ec = NEW Ninja(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

Ninja::Ninja(TargetChooser *_tc) :
    ExtraCost("Select unblocked attacker", _tc)
{
}

int Ninja::isPaymentSet()
{
    GameObserver * g = GameObserver::GetInstance();
    int currentPhase = g->getCurrentGamePhase();
    if (target && ((target->isAttacker() && target->blocked) || target->isAttacker() < 1 || currentPhase
                    != Constants::MTG_PHASE_COMBATBLOCKERS))
    {
        tc->removeTarget(target);
        target = NULL;
        return 0;
    }
    if (target)
        return 1;
    return 0;
}

int Ninja::doPay()
{

    if (target)
    {
        target->controller()->game->putInHand(target);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//endbouncetargetcostforninja
//------------------------------------------------------------

SacrificeCost * SacrificeCost::clone() const
{
    SacrificeCost * ec = NEW SacrificeCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

SacrificeCost::SacrificeCost(TargetChooser *_tc) :
    ExtraCost("Sacrifice", _tc)
{
}

int SacrificeCost::doPay()
{
    if (target)
    {
        WEvent * e = NEW WEventCardSacrifice(target);
        GameObserver * game = GameObserver::GetInstance();
        game->receiveEvent(e);
        target->controller()->game->putInGraveyard(target);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//Counter costs

CounterCost * CounterCost::clone() const
{
    CounterCost * ec = NEW CounterCost(*this);
    if (tc)
        ec->tc = tc->clone();
    if (counter)
        ec->counter = NEW Counter(counter->target, counter->name.c_str(), counter->power, counter->toughness);
        ec->counter->nb = counter->nb;
    return ec;
}

CounterCost::CounterCost(Counter * _counter, TargetChooser *_tc) :
    ExtraCost("Counters", _tc)
{
    counter = _counter;
    hasCounters = 0;
}

int CounterCost::setPayment(MTGCardInstance *card)
{
    if (tc)
    {
        int result = tc->addTarget(card);
        if (result)
        {
            if (counter->nb >= 0)
                return 1; //add counters always possible
            target = card;
            Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);
            if (targetCounter && targetCounter->nb >= -counter->nb)
            {
                hasCounters = 1;
                return result;
            }
        }
    }
    return 0;
}

int CounterCost::isPaymentSet()
{
    if (!target)
        return 0;
    if (counter->nb >= 0)
        return 1; //add counters always possible
    Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);
    if (targetCounter && targetCounter->nb >= -counter->nb)
    {
        hasCounters = 1;
    }
    if (target && hasCounters)
        return 1;
    return 0;
}

int CounterCost::canPay()
{
    // if target needs to be chosen, then move on.
    if (tc)
        return 1;
    if (counter->nb >= 0)
        return 1; //add counters always possible
    // otherwise, move on only if target has enough counters
    Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);
    if (targetCounter && targetCounter->nb >= -counter->nb)
        return 1;
    return 0;
}

int CounterCost::doPay()
{
    if (!target)
        return 0;

    if (counter->nb >= 0)
    { //Add counters as a cost
        for (int i = 0; i < counter->nb; i++)
        {
            target->counters->addCounter(counter->name.c_str(), counter->power, counter->toughness);
        }
        return 1;
    }

    //remove counters as a cost
    if (hasCounters)
    {
        for (int i = 0; i < -counter->nb; i++)
        {
            target->counters->removeCounter(counter->name.c_str(), counter->power, counter->toughness);
        }
        hasCounters = 0;
        return 1;
    }
    return 0;
}

CounterCost::~CounterCost()
{
    SAFE_DELETE(counter);
}

//
//Container
//
ExtraCosts::ExtraCosts()
{
    action = NULL;
    source = NULL;
}

ExtraCosts * ExtraCosts::clone() const
{
    ExtraCosts * ec = NEW ExtraCosts(*this);
    ec->costs.clear();
    for (size_t i = 0; i < costs.size(); i++)
    {
        ec->costs.push_back(costs[i]->clone());
    }
    return ec;
}

void ExtraCosts::Render()
{
    //TODO cool window and stuff...
    for (size_t i = 0; i < costs.size(); i++)
    {
        costs[i]->Render();
    }
}

int ExtraCosts::setAction(MTGAbility * _action, MTGCardInstance * _card)
{
    action = _action;
    source = _card;
    for (size_t i = 0; i < costs.size(); i++)
    {
        costs[i]->setSource(_card);
    }
    return 1;
}

int ExtraCosts::reset()
{
    action = NULL;
    source = NULL;
    //TODO set all payments to "unset"
    return 1;
}

int ExtraCosts::tryToSetPayment(MTGCardInstance * card)
{
    for (size_t i = 0; i < costs.size(); i++)
    {
        if(!costs[i]->isPaymentSet())
        {
            for(size_t k = 0; k < costs.size(); k++)
            {
                if(card == costs[k]->target)
                    return 0;
            }
            if (int result = costs[i]->setPayment(card))
            {
                card->isExtraCostTarget = true;
                return result;
            }
        }
    }
    return 0;
}

int ExtraCosts::isPaymentSet()
{
    for (size_t i = 0; i < costs.size(); i++)
    {
        if (!costs[i]->isPaymentSet())
            return 0;
    }
    return 1;
}

int ExtraCosts::canPay()
{
    for (size_t i = 0; i < costs.size(); i++)
    {
        if (!costs[i]->canPay())
        {
            costs[i]->target->isExtraCostTarget = false;
            return 0;
        }
    }
    return 1;
}

int ExtraCosts::doPay()
{
    int result = 0;
    for (size_t i = 0; i < costs.size(); i++)
    {
    if(costs[i]->target)
        costs[i]->target->isExtraCostTarget = false;
        result += costs[i]->doPay();
    }
    return result;
}

ExtraCosts::~ExtraCosts()
{
    for (size_t i = 0; i < costs.size(); i++)
    {
        SAFE_DELETE(costs[i]);
    }
}

void ExtraCosts::Dump()
{
    DebugTrace("=====\nDumping ExtraCosts=====\n");
    DebugTrace("NbElements: " << costs.size());
}
