#include "PrecompiledHeader.h"

#include "ExtraCost.h"
#include "TargetChooser.h"
#include "MTGCardInstance.h"
#include "Translate.h"
#include "Player.h"
#include "Counters.h"
#include "AllAbilities.h"
#include <boost/scoped_ptr.hpp>

SUPPORT_OBJECT_ANALYTICS(ExtraCost)

ExtraCost::ExtraCost(const std::string& inCostRenderString, TargetChooser *_tc, ManaCost * _costToPay)
    : tc(_tc),costToPay(_costToPay), source(NULL), target(NULL), mCostRenderString(inCostRenderString)
{
    if (tc)
        tc->targetter = NULL;
}

ExtraCost::~ExtraCost()
{
    SAFE_DELETE(costToPay);
    SAFE_DELETE(tc);
}

int ExtraCost::setSource(MTGCardInstance * _source)
{
    source = _source;
    if (tc)
    {
        tc->source = _source;
        // this keeps the target chooser from being unable to select a creature with shroud/protections.
        tc->targetter = NULL;
        tc->Owner = source->controller();
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
    if (costToPay && source->controller()->getManaPool()->canAfford(costToPay))
    {
        result = 1;
    }
    return result;
}
//extra added manacost, or add a manacost as the cost of extra
ExtraManaCost * ExtraManaCost::clone() const
{
    ExtraManaCost * ec = NEW ExtraManaCost(*this);
    return ec;
}

ExtraManaCost::ExtraManaCost(ManaCost * costToPay)
    : ExtraCost("Pay The Cost",NULL, costToPay)
{
}

int ExtraManaCost::tryToSetPayment(MTGCardInstance * card)
{
    return 1;
}

int ExtraManaCost::isPaymentSet()
{
    if (!source->controller()->getManaPool()->canAfford(costToPay))
    {
        return 0;
    }
    return 1;
}

int ExtraManaCost::canPay()
{
    if(!source->controller()->getManaPool()->canAfford(costToPay))
    {
        return 0;
    }
    return 1;
}

int ExtraManaCost::doPay()
{
    if (!source->controller()->getManaPool()->canAfford(costToPay))
        return 0;

    source->controller()->getManaPool()->pay(costToPay);
    return 1;
}

//life cost
LifeCost * LifeCost::clone() const
{
    LifeCost * ec = NEW LifeCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

LifeCost::LifeCost(TargetChooser *_tc)
    : ExtraCost("Life", _tc)
{
}

int LifeCost::canPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target->controller()->life <= 0)
    {
        return 0;
    }
    return 1;
}

int LifeCost::doPay()
{
    if (!target)
        return 0;

    MTGCardInstance * _target = (MTGCardInstance *) target;

    _target->controller()->loseLife(1);
    target = NULL;
    if (tc)
        tc->initTargets();
    return 1;
}

//Specific life cost
SpecificLifeCost * SpecificLifeCost::clone() const
{
    SpecificLifeCost * ec = NEW SpecificLifeCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

SpecificLifeCost::SpecificLifeCost(TargetChooser *_tc, int slc)
    : ExtraCost("Life", _tc), slc(slc)
{
}

int SpecificLifeCost::canPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target->controller()->life >= slc)
    {
        return 1;
    }
    return 0;
}

int SpecificLifeCost::doPay()
{
    if (!target)
        return 0;

    MTGCardInstance * _target = (MTGCardInstance *) target;

    _target->controller()->loseLife(slc);
    target = NULL;
    if (tc)
        tc->initTargets();
    return 1;
}

//life or Mana cost
LifeorManaCost * LifeorManaCost::clone() const
{
    LifeorManaCost * ec = NEW LifeorManaCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

ManaCost * LifeorManaCost::getManaCost()
{
    return &manaCost;
}

LifeorManaCost::LifeorManaCost(TargetChooser *_tc, string manaType)
    : ExtraCost("Phyrexian Mana", _tc), manaType(manaType)
{
    string buildType ="{";
    buildType.append(manaType);
    buildType.append("}");
    boost::scoped_ptr<ManaCost> cost(ManaCost::parseManaCost(buildType));
    manaCost.copy(cost.get());
}

int LifeorManaCost::canPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target->controller()->getManaPool()->canAfford(getManaCost()) || _target->controller()->life > 1)
    {
        return 1;
    }
    return 0;
}

int LifeorManaCost::doPay()
{
    if (!target)
        return 0;

    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target->controller()->getManaPool()->canAfford(&manaCost))
    {
        _target->controller()->getManaPool()->pay(&manaCost);
    }
    else
    {
        _target->controller()->loseLife(2);
    }
    target = NULL;
    if (tc)
        tc->initTargets();
    return 1;
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

DiscardRandomCost::DiscardRandomCost(TargetChooser *_tc)
    : ExtraCost("Discard Random", _tc)
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
        source->storedCard = target->createSnapShot();
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
        source->storedCard = target->createSnapShot();
        WEvent * e = NEW WEventCardDiscard(target);
        GameObserver * game = target->owner->getObserver();
        game->receiveEvent(e);
        _target->controller()->game->putInGraveyard(_target);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}

//cycling
CycleCost * CycleCost::clone() const
{
    CycleCost * ec = NEW CycleCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

CycleCost::CycleCost(TargetChooser *_tc) :
ExtraCost("Cycle", _tc)
{
}

int CycleCost::doPay()
{
    MTGCardInstance * _source = (MTGCardInstance *) source;
    if (_source)
    {
        WEvent * e = NEW WEventCardDiscard(target);//cycling sends 2 events one for the discard and one for the specific cycle trigger
        GameObserver * game = _source->owner->getObserver();
        game->receiveEvent(e);
        WEvent * e2 = NEW WEventCardCycle(_source);
        game->receiveEvent(e2);
        _source->controller()->game->putInGraveyard(_source);
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

ToLibraryCost::ToLibraryCost(TargetChooser *_tc)
    : ExtraCost("Put a card on top of Library", _tc)
{
}

int ToLibraryCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        source->storedCard = target->createSnapShot();
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

MillCost::MillCost(TargetChooser *_tc)
    : ExtraCost("Deplete", _tc)
{
}

int MillCost::canPay()
{
    MTGGameZone * z = target?target->controller()->game->library:source->controller()->game->library;
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
        source->storedCard = (MTGCardInstance*)_target->controller()->game->library->cards[_target->controller()->game->library->nb_cards - 1]->createSnapShot();
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


MillExileCost::MillExileCost(TargetChooser *_tc)
    : MillCost(_tc)
{
    // override the base string here
    mCostRenderString = "Deplete To Exile";
}

int MillExileCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    source->storedCard = (MTGCardInstance*)_target->controller()->game->library->cards[_target->controller()->game->library->nb_cards - 1]->createSnapShot();
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
//unattach cost

UnattachCost * UnattachCost::clone() const
{
    UnattachCost * ec = NEW UnattachCost(*this);
    return ec;
}

UnattachCost::UnattachCost(MTGCardInstance * realSource)
    : ExtraCost("Unattach"),rSource(realSource)
{
}

int UnattachCost::isPaymentSet()
{
    if (rSource && !rSource->target)
    {
        return 0;
    }
    return 1;
}

int UnattachCost::canPay()
{
    return isPaymentSet();
}

int UnattachCost::doPay()
{
    MTGCardInstance * _source = (MTGCardInstance *) source;
    if(_source != rSource)
        _source = rSource;//for debugging purposes I let it set what it thinks is the source.
    if (_source)
    {
        GameObserver * game = _source->getObserver();
        for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
        {
            MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
            AEquip * eq = dynamic_cast<AEquip*> (a);
            if (eq && eq->source == _source)
            {
                ((AEquip*)a)->unequip();
            }
        }
        return 1;
    }
    return 0;
}

//Tap cost
TapCost * TapCost::clone() const
{
    TapCost * ec = NEW TapCost(*this);
    return ec;
}

TapCost::TapCost()
    : ExtraCost("Tap")
{
}

int TapCost::isPaymentSet()
{
    if (source && (source->isTapped() || source->hasSummoningSickness()))
    {
        return 0;
    }
    return 1;
}

int TapCost::canPay()
{
    return isPaymentSet();
}

int TapCost::doPay()
{
    MTGCardInstance * _source = (MTGCardInstance *) source;
    if (_source)
    {
        _source->tap();
        return 1;
    }
    return 0;
}
//unTap cost
UnTapCost * UnTapCost::clone() const
{
    UnTapCost * ec = NEW UnTapCost(*this);
    return ec;
}

UnTapCost::UnTapCost() :
ExtraCost("UnTap")
{
}

int UnTapCost::isPaymentSet()
{
/*602.5a A creature's activated ability with the tap symbol ({T}) or the untap symbol ({Q})
 * in its activation cost can't be activated unless the creature has been under its
 * controller's control since the start of his or her most recent turn.
 * Ignore this rule for creatures with haste (see rule 702.10). As of 6/1/2014 Comprehensive Rules
 */
    if (source && (!source->isTapped() || source->hasSummoningSickness()))
    {
        return 0;
    }
    return 1;
}

int UnTapCost::canPay()
{
    return isPaymentSet();
}

int UnTapCost::doPay()
{
    MTGCardInstance * _source = (MTGCardInstance *) source;
    if (_source)
    {
        _source->untap();
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

TapTargetCost::TapTargetCost(TargetChooser *_tc)
    : ExtraCost("Tap Target", _tc)
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
    source->storedCard = target->createSnapShot();
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

//untap other as a cost
UnTapTargetCost * UnTapTargetCost::clone() const
{
    UnTapTargetCost * ec = NEW UnTapTargetCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

UnTapTargetCost::UnTapTargetCost(TargetChooser *_tc)
    : ExtraCost("Untap Target", _tc)
{
}

int UnTapTargetCost::isPaymentSet()
{
    if (target && !target->isTapped())
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

int UnTapTargetCost::doPay()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    source->storedCard = target->createSnapShot();
    if (target)
    {
        _target->untap();
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

ExileTargetCost::ExileTargetCost(TargetChooser *_tc)
    : ExtraCost("Exile Target", _tc)
{
}

int ExileTargetCost::doPay()
{

    if (target)
    {
        source->storedCard = target->createSnapShot();
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

BounceTargetCost::BounceTargetCost(TargetChooser *_tc)
    : ExtraCost("Return Target to Hand", _tc)
{
}

int BounceTargetCost::doPay()
{

    if (target)
    {
        source->storedCard = target->createSnapShot();
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

int Ninja::canPay()
{
    if(source->getObserver()->getCurrentGamePhase() != MTG_PHASE_COMBATBLOCKERS)
        return 0;
    return 1;
}

int Ninja::isPaymentSet()
{
    if (target && ((target->isAttacker() && target->isBlocked()) ||
                   target->isAttacker() < 1 ||
                   target->getObserver()->getCurrentGamePhase() != MTG_PHASE_COMBATBLOCKERS))
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

//Sacrifice target as cost for Offering
Offering * Offering::clone() const
{
    Offering * ec = NEW Offering(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

Offering::Offering(TargetChooser *_tc) :
ExtraCost("Select creature to offer", _tc)
{
}

int Offering::canPay()
{
    if (target && (!source->controller()->getManaPool()->canAfford(source->getManaCost()->Diff(target->getManaCost()))))
    {
        tc->removeTarget(target);
        target = NULL;
        return 0;
    }
    if (target && (source->controller()->getManaPool()->canAfford(source->getManaCost()->Diff(target->getManaCost()))))
        return 1;
    return 0;
}

int Offering::isPaymentSet()
{
    if (target && (!source->controller()->getManaPool()->canAfford(source->getManaCost()->Diff(target->getManaCost()))))
    {
        tc->removeTarget(target);
        target = NULL;
        return 0;
    }
    if (target && (source->controller()->getManaPool()->canAfford(source->getManaCost()->Diff(target->getManaCost()))))
        return 1;
    return 0;
}

int Offering::doPay()
{
    if (target)
    {
        target->controller()->getManaPool()->pay(source->getManaCost()->Diff(target->getManaCost()));
        MTGCardInstance * beforeCard = target;
        source->storedCard = target->createSnapShot();
        target->controller()->game->putInGraveyard(target);
        WEvent * e = NEW WEventCardSacrifice(beforeCard,target);
        GameObserver * game = target->owner->getObserver();
        game->receiveEvent(e);
        target = NULL;
        if (tc)
            tc->initTargets();
        return 1;
    }
    return 0;
}
//------------------------------------------------------------

SacrificeCost * SacrificeCost::clone() const
{
    SacrificeCost * ec = NEW SacrificeCost(*this);
    if (tc)
        ec->tc = tc->clone();
    return ec;
}

SacrificeCost::SacrificeCost(TargetChooser *_tc)
    : ExtraCost("Sacrifice", _tc)
{
}

int SacrificeCost::doPay()
{
    if (target)
    {
        MTGCardInstance * beforeCard = target;
        source->storedCard = target->createSnapShot();
        target->controller()->game->putInGraveyard(target);
        WEvent * e = NEW WEventCardSacrifice(beforeCard,target);
        GameObserver * game = target->owner->getObserver();
        game->receiveEvent(e);
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

    //TODO: counter can be NULL at this point, what do we set ec->counter->nb to if it is?
    if (ec->counter != NULL)
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
    Counter * targetCounter = NULL;
    if(target)
    {
        targetCounter = target->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);

    }
    else
    {
        targetCounter = source->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);
    }
    if (targetCounter && targetCounter->nb >= -counter->nb)
    {
        return 1;
    }
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
        if (tc)
            tc->initTargets();
        target = NULL;
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
        if (tc)
            tc->initTargets();
        target = NULL;
        return 1;
    }
    if (tc)
        tc->initTargets();
    target = NULL;
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
    source->storedCard = NULL;
    for (size_t i = 0; i < costs.size(); i++)
    {
        costs[i]->setSource(_card);
    }
    return 1;
}

int ExtraCosts::reset()
{
    action = NULL;
    source->storedCard = NULL;
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
            if(costs[i]->target)
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
        {
            costs[i]->target->isExtraCostTarget = false;
        }
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
