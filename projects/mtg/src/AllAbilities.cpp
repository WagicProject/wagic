#include "PrecompiledHeader.h"
#include "AllAbilities.h"

//Activated Abilities

//Generic Activated Abilities
GenericActivatedAbility::GenericActivatedAbility(string newName,int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int _tap,
        string limit,MTGAbility * sideEffects,string usesBeforeSideEffects, int restrictions, MTGGameZone * dest) :
    ActivatedAbility(_id, card, _cost, restrictions, _tap,limit,sideEffects,usesBeforeSideEffects), NestedAbility(a), activeZone(dest),newName(newName)
{
    counters = 0;
    target = ability->target;
}

int GenericActivatedAbility::resolve()
{
    ManaCost * diff = abilityCost->Diff(cost);
    source->X = diff->hasX();
    source->XX = source->X/2;
    SAFE_DELETE(diff);
    //SAFE_DELETE(abilityCost); this line has been reported as a bug. removing it doesn't seem to break anything, although I didn't get any error in the test suite by leaving it either, so... leaving it for now as a comment, in case.
    ability->target = target; //may have been updated...
    if (ability)
        return ability->resolve();
    return 0;
}

const char * GenericActivatedAbility::getMenuText()
{
if(newName.size())
return newName.c_str();
    if (ability)
        return ability->getMenuText();
    return "Error";
}

int GenericActivatedAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (dynamic_cast<AAMorph*> (ability) && !card->isMorphed && !card->morphed && card->turningOver)
        return 0;
    return ActivatedAbility::isReactingToClick(card, mana);
}

void GenericActivatedAbility::Update(float dt)
{
    ActivatedAbility::Update(dt);
}

int GenericActivatedAbility::testDestroy()
{
    if (!activeZone)
        return ActivatedAbility::testDestroy();
    if (activeZone->hasCard(source))
        return 0;
    return 1;

}

GenericActivatedAbility * GenericActivatedAbility::clone() const
{
    GenericActivatedAbility * a = NEW GenericActivatedAbility(*this);
    a->cost = NEW ManaCost();
    a->cost->copy(cost);
    a->ability = ability->clone();
    return a;
}

GenericActivatedAbility::~GenericActivatedAbility()
{
    SAFE_DELETE(ability);
}

//AA Alter Poison
AAAlterPoison::AAAlterPoison(int _id, MTGCardInstance * _source, Targetable * _target, int poison, ManaCost * _cost, int doTap,
        int who) :
    ActivatedAbilityTP(_id, _source, _target, _cost, doTap, who), poison(poison)
{
}

int AAAlterPoison::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        _target->poisonCount += poison;
    }
    return 0;
}

const char * AAAlterPoison::getMenuText()
{
    return "Poison";
}

AAAlterPoison * AAAlterPoison::clone() const
{
    AAAlterPoison * a = NEW AAAlterPoison(*this);
    a->isClone = 1;
    return a;
}

AAAlterPoison::~AAAlterPoison()
{
}

//Damage Prevent
AADamagePrevent::AADamagePrevent(int _id, MTGCardInstance * _source, Targetable * _target, int preventing, ManaCost * _cost,
        int doTap, int who) :
    ActivatedAbilityTP(_id, _source, _target, _cost, doTap, who), preventing(preventing)
{
    aType = MTGAbility::STANDARD_PREVENT;
}

int AADamagePrevent::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        _target->preventable += preventing;
    }
    return 0;
}

const char * AADamagePrevent::getMenuText()
{
    return "Prevent Damage";
}

AADamagePrevent * AADamagePrevent::clone() const
{
    AADamagePrevent * a = NEW AADamagePrevent(*this);
    a->isClone = 1;
    return a;
}

AADamagePrevent::~AADamagePrevent()
{
}

//AADamager
AADamager::AADamager(int _id, MTGCardInstance * _source, Targetable * _target, string d, ManaCost * _cost, int doTap,
        int who) :
    ActivatedAbilityTP(_id, _source, _target, _cost, doTap, who), d(d)
{
    aType = MTGAbility::DAMAGER;
    }

    int AADamager::resolve()
    {
        Damageable * _target = (Damageable *) getTarget();
        if (_target)
        {
            WParsedInt damage(d, NULL, (MTGCardInstance *)source);
            game->mLayers->stackLayer()->addDamage(source, _target, damage.getValue());
            game->mLayers->stackLayer()->resolve();
            return 1;
        }
        return 0;
    }

    int AADamager::getDamage()
    {
        WParsedInt damage(d, NULL, (MTGCardInstance *)source);
        return damage.getValue();
    }

    const char * AADamager::getMenuText()
    {
        return "Damage";
    }

AADamager * AADamager::clone() const
{
    AADamager * a = NEW AADamager(*this);
    a->isClone = 1;
    return a;
}


//AADepleter
AADepleter::AADepleter(int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who),nbcardsStr(nbcardsStr)
{

}
    int AADepleter::resolve()
    {

        Targetable * _target = getTarget();
        Player * player;
        if (_target)
        {
            WParsedInt numCards(nbcardsStr, NULL, source);
            if (_target->typeAsTarget() == TARGET_CARD)
            {
                player = ((MTGCardInstance *) _target)->controller();
            }
            else
            {
                player = (Player *) _target;
            }
            MTGLibrary * library = player->game->library;
            for (int i = 0; i < numCards.getValue(); i++)
            {
                if (library->nb_cards)
                    player->game->putInZone(library->cards[library->nb_cards - 1], library, player->game->graveyard);
            }
        }
        return 1;
    }

const char * AADepleter::getMenuText()
{
    return "Deplete";
}

AADepleter * AADepleter::clone() const
{
    AADepleter * a = NEW AADepleter(*this);
    a->isClone = 1;
    return a;
}

//AACopier
AACopier::AACopier(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(_id, _source, _cost, 0, 0)
{
    target = _target;
}

int AACopier::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        source->copy(_target);
        return 1;
    }
    return 0;
}

const char * AACopier::getMenuText()
{
    return "Copy";
}

AACopier * AACopier::clone() const
{
    AACopier * a = NEW AACopier(*this);
    a->isClone = 1;
    return a;
}

//phaser
AAPhaseOut::AAPhaseOut(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(_id, _source, _cost, 0, 0)
{
    target = _target;
}

int AAPhaseOut::resolve()
{GameObserver * g = g->GetInstance();
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        _target->isPhased = true;
        
        _target->phasedTurn = game->turn;
        if(_target->view)
            _target->view->alpha = 50;
        _target->initAttackersDefensers();
        return 1;
    }
    return 0;
}

const char * AAPhaseOut::getMenuText()
{
    return "Phase Out";
}

AAPhaseOut * AAPhaseOut::clone() const
{
    AAPhaseOut * a = NEW AAPhaseOut(*this);
    a->isClone = 1;
    return a;
}

//Counters
AACounter::AACounter(int id, MTGCardInstance * source, MTGCardInstance * target,string counterstring, const char * _name, int power, int toughness,
        int nb,int maxNb, ManaCost * cost, int doTap) :
    ActivatedAbility(id, source, cost, 0, doTap),counterstring(counterstring), nb(nb),maxNb(maxNb), power(power), toughness(toughness), name(_name)
{
    this->target = target;
    if (name.find("Level"))
        aType = MTGAbility::STANDARD_LEVELUP;
    menu = "";
}

    int AACounter::resolve()
    {
        if (target)
        {
            MTGCardInstance * _target = (MTGCardInstance *) target;
            AbilityFactory af;
            Counter * checkcounter = af.parseCounter(counterstring, source, NULL);
            nb = checkcounter->nb;
            delete checkcounter;
            if (nb > 0)
            {
                for (int i = 0; i < nb; i++)
                {
                    while (_target->next)
                        _target = _target->next;

                    Counter * targetCounter = NULL;
                    int currentAmount = 0;
                    if (_target->counters && _target->counters->hasCounter(name.c_str(), power, toughness))
                    {
                        targetCounter = _target->counters->hasCounter(name.c_str(), power, toughness);
                        currentAmount = targetCounter->nb;
                    }
                    if(!maxNb || (maxNb && currentAmount < maxNb))
                        _target->counters->addCounter(name.c_str(), power, toughness);
                }
            }
        else
        {
            for (int i = 0; i < -nb; i++)
            {
                while (_target->next)
                    _target = _target->next;
                _target->counters->removeCounter(name.c_str(), power, toughness);
            }
        }
        //specail cases, indestructible creatures which recieve enough counters to kill it are destroyed as a state based effect
        if(_target->toughness <= 0 && _target->has(Constants::INDESTRUCTIBLE) && toughness < 0)
            _target->controller()->game->putInGraveyard(_target);
        return nb;
    }
    return 0;
}

const char* AACounter::getMenuText()
{
    if (menu.size())
    {
        return menu.c_str();
    }
    char buffer[128];

    if (name.size())
    {
        string s = name;
        menu.append(s.c_str());
    }

    if (power != 0 || toughness != 0)
    {
        sprintf(buffer, " %i/%i", power, toughness);
        menu.append(buffer);
    }

    menu.append(" Counter");
    if (nb != 1 && !(nb < -1000))
    {
        sprintf(buffer, ": %i", nb);
        menu.append(buffer);
    }

    sprintf(menuText, "%s", menu.c_str());
    return menuText;
}

AACounter * AACounter::clone() const
{
    AACounter * a = NEW AACounter(*this);
    a->isClone = 1;
    return a;
}

//Counters
AARemoveAllCounter::AARemoveAllCounter(int id, MTGCardInstance * source, MTGCardInstance * target, const char * _name, int power, int toughness,
        int nb,bool all, ManaCost * cost, int doTap) :
    ActivatedAbility(id, source, cost, 0, doTap), nb(nb), power(power), toughness(toughness), name(_name),all(all)
{
    this->target = target;
    menu = "";
}

    int AARemoveAllCounter::resolve()
    {
        if (target)
        {
            MTGCardInstance * _target = (MTGCardInstance *) target;
            if (all )
            {
                for(int amount = 0;amount < _target->counters->mCount;amount++)
                {
                    while(_target->counters->counters[amount]->nb > 0)
                        _target->counters->removeCounter(_target->counters->counters[amount]->name.c_str(),_target->counters->counters[amount]->power,_target->counters->counters[amount]->toughness);

                }
            }
            Counter * targetCounter = NULL;
            if (_target->counters && _target->counters->hasCounter(name.c_str(), power, toughness))
            {
                targetCounter = _target->counters->hasCounter(name.c_str(), power, toughness);
                nb = targetCounter->nb;
            }

            if (nb > 0)
            {
                for (int i = 0; i < nb; i++)
                {
                    while (_target->next)
                        _target = _target->next;
                    _target->counters->removeCounter(name.c_str(), power, toughness);
                }
            }
        return nb;
    }
    return 0;
}

const char* AARemoveAllCounter::getMenuText()
{
    if (menu.size())
    {
        return menu.c_str();
    }
    char buffer[128];

    if (name.size())
    {
        string s = name;
        menu.append(s.c_str());
    }

    if (power != 0 || toughness != 0)
    {
        sprintf(buffer, " %i/%i", power, toughness);
        menu.append(buffer);
    }

    menu.append(" Counter Removed");
    if (nb != 1)
    {
        sprintf(buffer, ": %i", nb);
        menu.append(buffer);
    }

    sprintf(menuText, "%s", menu.c_str());
    return menuText;
}

AARemoveAllCounter * AARemoveAllCounter::clone() const
{
    AARemoveAllCounter * a = NEW AARemoveAllCounter(*this);
    a->isClone = 1;
    return a;
}

// Fizzler
AAFizzler::AAFizzler(int _id, MTGCardInstance * card, Spell * _target, ManaCost * _cost, int _tap) :
ActivatedAbility(_id, card, _cost, 0, _tap)
{
    target = _target;
}

int AAFizzler::resolve()
{
    Spell * _target = (Spell *) target;
    if(!target && !_target)
    {
        //if we hit this condiational its because Ai was targetting.
        Interruptible * targetCard = game->mLayers->stackLayer()->getAt(game->mLayers->stackLayer()->getActionElementFromCard(source->target));
        if (source->target && source->target->has(Constants::NOFIZZLE))
            return 0;
        _target = (Spell *) targetCard;
        game->mLayers->stackLayer()->Fizzle(_target);
        return 1;
    }
    if (target && _target->source->has(Constants::NOFIZZLE))
        return 0;
    game->mLayers->stackLayer()->Fizzle(_target);
    return 1;
}

const char * AAFizzler::getMenuText()
{
    return "Fizzle";
}

AAFizzler* AAFizzler::clone() const
{
    AAFizzler * a = NEW AAFizzler(*this);
    a->isClone = 1;
    return a;
}
// BanishCard implementations

AABanishCard::AABanishCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _banishmentType) :
    ActivatedAbility(_id, _source, NULL), banishmentType(_banishmentType)
{
    if (_target)
        target = _target;
}

const char * AABanishCard::getMenuText()
{
    return "Send to graveyard";
}

int AABanishCard::resolve()
{
    DebugTrace("This is not implemented!");
    return 0;
}

AABanishCard * AABanishCard::clone() const
{
    AABanishCard * a = NEW AABanishCard(*this);
    a->isClone = 1;
    return a;
}

// Bury

AABuryCard::AABuryCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _banishmentType) :
    AABanishCard(_id, _source, _target, AABanishCard::BURY)
{
}

int AABuryCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        return _target->bury();
    }
    return 0;
}

const char * AABuryCard::getMenuText()
{
    return "Bury";
}

AABuryCard * AABuryCard::clone() const
{
    AABuryCard * a = NEW AABuryCard(*this);
    a->isClone = 1;
    return a;
}

// Destroy

AADestroyCard::AADestroyCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _banishmentType) :
    AABanishCard(_id, _source, _target, AABanishCard::DESTROY)
{
}

int AADestroyCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        return _target->destroy();
    }
    return 0;
}

const char * AADestroyCard::getMenuText()
{
    return "Destroy";
}

AADestroyCard * AADestroyCard::clone() const
{
    AADestroyCard * a = NEW AADestroyCard(*this);
    a->isClone = 1;
    return a;
}

// Sacrifice
AASacrificeCard::AASacrificeCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _banishmentType) :
    AABanishCard(_id, _source, _target, AABanishCard::SACRIFICE)
{
}

int AASacrificeCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        Player * p = _target->controller();
        WEvent * e = NEW WEventCardSacrifice(_target);
        GameObserver * game = GameObserver::GetInstance();
        game->receiveEvent(e);
        p->game->putInGraveyard(_target);
        return 1;
    }
    return 0;
}

const char * AASacrificeCard::getMenuText()
{
    return "Sacrifice";
}

AASacrificeCard * AASacrificeCard::clone() const
{
    AASacrificeCard * a = NEW AASacrificeCard(*this);
    a->isClone = 1;
    return a;
}

// Discard 

AADiscardCard::AADiscardCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _banishmentType) :
    AABanishCard(_id, _source, _target, AABanishCard::DISCARD)
{
}

int AADiscardCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        Player * p = _target->controller();
        WEvent * e = NEW WEventCardDiscard(_target);
        GameObserver * game = GameObserver::GetInstance();
        game->receiveEvent(e);
        p->game->putInGraveyard(_target);
        return 1;
    }
    return 0;
}

const char * AADiscardCard::getMenuText()
{
    return "Discard";
}

AADiscardCard * AADiscardCard::clone() const
{
    AADiscardCard * a = NEW AADiscardCard(*this);
    a->isClone = 1;
    return a;
}

AADrawer::AADrawer(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, string nbcardsStr, int _tap,
        int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who), nbcardsStr(nbcardsStr)
{
    aType = MTGAbility::STANDARD_DRAW;
}

    int AADrawer::resolve()
    {
        Targetable * _target = getTarget();
        Player * player;
        if (_target)
        {
            WParsedInt numCards(nbcardsStr, NULL, source);
            if (_target->typeAsTarget() == TARGET_CARD)
            {
                player = ((MTGCardInstance *) _target)->controller();
            }
            else
            {
                player = (Player *) _target;
            }
            game->mLayers->stackLayer()->addDraw(player, numCards.getValue());
            game->mLayers->stackLayer()->resolve();
        }
        return 1;
    }

    int AADrawer::getNumCards()
    {
        WParsedInt numCards(nbcardsStr, NULL, source);
        return numCards.getValue();
    }

const char * AADrawer::getMenuText()
{
    return "Draw";
}

AADrawer * AADrawer::clone() const
{
    AADrawer * a = NEW AADrawer(*this);
    a->isClone = 1;
    return a;
}

// AAFrozen: Prevent a card from untapping during next untap phase
AAFrozen::AAFrozen(int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost, int doTap) :
ActivatedAbility(id, card, _cost, 0, doTap)
{
    target = _target;
}

int AAFrozen::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next; //This is for cards such as rampant growth
        _target->frozen += 1;
    }
    return 1;
}

const char * AAFrozen::getMenuText()
{
    return "Freeze";
}

AAFrozen * AAFrozen::clone() const
{
    AAFrozen * a = NEW AAFrozen(*this);
    a->isClone = 1;
    return a;
}

// chose a new target for an aura or enchantment and equip it note: VERY basic right now.
AANewTarget::AANewTarget(int id, MTGCardInstance * card, MTGCardInstance * _target,bool retarget, ManaCost * _cost, int doTap) :
ActivatedAbility(id, card, _cost, 0, doTap),retarget(retarget)
{
    target = _target;
}

int AANewTarget::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(retarget)
    {
        _target = source;
        source = (MTGCardInstance *) target;
    }
    if (_target)
    {
        while (_target->next)
            _target = _target->next; 
        _target->controller()->game->putInZone(_target, _target->currentZone,
            _target->owner->game->exile);
        _target = _target->next;

        MTGCardInstance * refreshed = source->controller()->game->putInZone(_target,_target->currentZone,source->controller()->game->battlefield);
        Spell * reUp = NEW Spell(refreshed);
        if(reUp->source->hasSubtype(Subtypes::TYPE_AURA))
        {
            reUp->source->target = source;
            reUp->resolve();
        }
        if(_target->hasSubtype(Subtypes::TYPE_EQUIPMENT))
        {
            reUp->resolve();
            GameObserver * g = g->GetInstance();
            for (int i = 1; i < g->mLayers->actionLayer()->mCount; i++)
            {
                MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->mObjects[i]);
                AEquip * eq = dynamic_cast<AEquip*> (a);
                if (eq && eq->source == reUp->source)
                {
                    ((AEquip*)a)->unequip();
                    ((AEquip*)a)->equip(source);
                }
            }
        }
        delete reUp;
        if(retarget)
        {
            target = source;
            source = _target;
        }

    }
    return 1;
}

const char * AANewTarget::getMenuText()
{
    return "New Target";
}

AANewTarget * AANewTarget::clone() const
{
    AANewTarget * a = NEW AANewTarget(*this);
    a->isClone = 1;
    a->oneShot = 1;
    return a;
}
// morph a card
AAMorph::AAMorph(int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost, int doTap) :
ActivatedAbility(id, card, _cost, restrictions, doTap)
{
    target = _target;
}

int AAMorph::resolve()
{
    MTGCardInstance * Morpher = (MTGCardInstance*)source;
    if(!Morpher->isMorphed && !Morpher->morphed && Morpher->turningOver)
        return 0;
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next; 

        AbilityFactory af;
        _target->morphed = false;
        _target->isMorphed = false;
        _target->turningOver = true;
        af.getAbilities(&currentAbilities, NULL, _target, 0);
        for (size_t i = 0; i < currentAbilities.size(); ++i)
        {
            MTGAbility * a = currentAbilities[i];
            a->source = (MTGCardInstance *) _target;
            if( a && dynamic_cast<AAMorph *> (a))
            {
                a->removeFromGame();
                GameObserver * g = g->GetInstance();
                g->removeObserver(a);
            }
            if (a)
            {
                if (a->oneShot)
                {
                    a->resolve();
                    delete (a);
                }
                else
                {
                    a->addToGame();
                }
            }
        }
        currentAbilities.clear();
        testDestroy();
    }
    return 1;
}

int AAMorph::testDestroy()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(target)
    {
        if(_target->turningOver && !_target->isMorphed && !_target->morphed)
        {
            GameObserver * g = g->GetInstance();
            g->removeObserver(this);
            return 1;
        }
    }
    return 0;
}

const char * AAMorph::getMenuText()
{
    return "Morph";
}

AAMorph * AAMorph::clone() const
{
    AAMorph * a = NEW AAMorph(*this);
    a->isClone = 1;
    a->forceDestroy = 1;
    return a;
}
// AADYNAMIC: dynamic ability builder
AADynamic::AADynamic(int id, MTGCardInstance * card, Damageable * _target,int type,int effect,int who,int amountsource,MTGAbility * storedAbility, ManaCost * _cost, int doTap) :
ActivatedAbility(id, card, _cost, 0, doTap),type(type),effect(effect),who(who),amountsource(amountsource),eachother(eachother),storedAbility(storedAbility)
{
    target = _target;
    sourceamount = 0;
    targetamount = 0;
    eachother = false;
    tosrc = false;
    menu = "";
    OriginalSrc = source;
    storedAbility = storedAbility;
    clonedStored = NULL;
}

int AADynamic::resolve()
{
    Damageable * _target = (Damageable *) target;
    Damageable * secondaryTarget = NULL;
    if(amountsource == 2)
        source = (MTGCardInstance * )_target;
    switch(who)
    {
    case DYNAMIC_ABILITY_WHO_EACHOTHER://each other, both take the effect
        eachother = true;
        break;
    case DYNAMIC_ABILITY_WHO_ITSELF:
        source = ((MTGCardInstance *) _target);
        _target = _target;
        break;
    case DYNAMIC_ABILITY_WHO_TARGETCONTROLLER:
        _target = _target;
        secondaryTarget = ((MTGCardInstance *) _target)->controller();
        break;
    case DYNAMIC_ABILITY_WHO_TARGETOPPONENT:
        _target = _target;
        secondaryTarget = ((MTGCardInstance *) _target)->controller()->opponent();
        break;
    case DYNAMIC_ABILITY_WHO_TOSOURCE:
        tosrc = true;
        break;
    case DYNAMIC_ABILITY_WHO_SOURCECONTROLLER:
        _target = _target;
        secondaryTarget = ((MTGCardInstance *) OriginalSrc)->controller();
        break;
    case DYNAMIC_ABILITY_WHO_SOURCEOPPONENT:
        secondaryTarget = OriginalSrc->controller()->opponent();
        break;
    default:
        _target = _target;
        break;
    }
    if(amountsource == DYNAMIC_MYSELF_AMOUNT)
        _target = OriginalSrc->controller();//looking at controller for amount
    if(amountsource == DYNAMIC_MYFOE_AMOUNT)
        _target = OriginalSrc->controller()->opponent();//looking at controllers opponent for amount
    if(!_target)
        return 0;
    while (_target->typeAsTarget() == TARGET_CARD && ((MTGCardInstance *)_target)->next)
        _target = ((MTGCardInstance *)_target)->next;

    //find the amount variables that will be used
    sourceamount = 0;
    targetamount = 0;
    int colored = 0;
    switch(type)
    {
    case DYNAMIC_ABILITY_TYPE_POWER:
        sourceamount = ((MTGCardInstance *) source)->power;
        targetamount = ((MTGCardInstance *) _target)->power;
        if(eachother )
            sourceamount = ((MTGCardInstance *) source)->power;
        break;
    case DYNAMIC_ABILITY_TYPE_TOUGHNESS:
        sourceamount = ((MTGCardInstance *) source)->toughness;
        targetamount = ((MTGCardInstance *) _target)->toughness;
        if(eachother )
            sourceamount = ((MTGCardInstance *) source)->toughness;
        break;
    case DYNAMIC_ABILITY_TYPE_MANACOST:
        if(amountsource == 1)
            sourceamount = ((MTGCardInstance *) source)->getManaCost()->getConvertedCost();
        else
            sourceamount = ((MTGCardInstance *) _target)->getManaCost()->getConvertedCost();
        break;
    case DYNAMIC_ABILITY_TYPE_COLORS:
        for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
        {
            if (amountsource == 1 && ((MTGCardInstance *)source)->hasColor(i))
                ++colored;
            else
                if (amountsource == 2 && ((MTGCardInstance *)_target)->hasColor(i))
                    ++colored;
        }
        sourceamount = colored;
        break;
    case DYNAMIC_ABILITY_TYPE_AGE:
        {
            Counter * targetCounter = NULL;
            if(amountsource == 2)
            {
                if (((MTGCardInstance *)_target)->counters && ((MTGCardInstance *)_target)->counters->hasCounter("age", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)_target)->counters->hasCounter("age", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            else
            {
                if (((MTGCardInstance *)source)->counters && ((MTGCardInstance *)source)->counters->hasCounter("age", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)source)->counters->hasCounter("age", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            break;
        }
    case DYNAMIC_ABILITY_TYPE_CHARGE:
        {
            Counter * targetCounter = NULL;
            if(amountsource == 2)
            {
                if (((MTGCardInstance *)_target)->counters && ((MTGCardInstance *)_target)->counters->hasCounter("charge", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)_target)->counters->hasCounter("charge", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            else
            {
                if (((MTGCardInstance *)source)->counters && ((MTGCardInstance *)source)->counters->hasCounter("charge", 0, 0))
                {
                    targetCounter = ((MTGCardInstance *)source)->counters->hasCounter("charge", 0, 0);
                    sourceamount = targetCounter->nb;
                }
            }
            break;
        }
    case DYNAMIC_ABILITY_TYPE_ONEONECOUNTERS:
        {
            Counter * targetCounter = NULL;
            if(amountsource == 2)
            {
                if (((MTGCardInstance *)_target)->counters && ((MTGCardInstance *)_target)->counters->hasCounter(1, 1))
                {
                    targetCounter = ((MTGCardInstance *)_target)->counters->hasCounter(1,1);
                    sourceamount = targetCounter->nb;
                }
            }
            else
            {
                if (((MTGCardInstance *)source)->counters && ((MTGCardInstance *)source)->counters->hasCounter(1, 1))
                {
                    targetCounter = ((MTGCardInstance *)source)->counters->hasCounter(1,1);
                    sourceamount = targetCounter->nb;
                }
            }
            break;
        }
    case DYNAMIC_ABILITY_TYPE_THATMUCH:
        {
            sourceamount = _target->thatmuch;
            break;
        }
    default:
        break;
    }

    if(secondaryTarget != NULL)
    {
        _target = secondaryTarget;
    }
    if (_target)
    {
        while (_target->typeAsTarget() == TARGET_CARD && ((MTGCardInstance *)_target)->next)
            _target = ((MTGCardInstance *)_target)->next;
        if(sourceamount < 0)
            sourceamount = 0;
        if(targetamount < 0)
            targetamount = 0;
        //set values less then 0 to 0, it was reported that negitive numbers such as a creature who get -3/-3 having the power become
        //negitive, if then used as the amount, would cuase weird side effects on resolves.
        switch(effect)
        {
        case DYNAMIC_ABILITY_EFFECT_STRIKE://deal damage
            if(storedAbility)
                activateStored();
            if(tosrc == false)
            {
                game->mLayers->stackLayer()->addDamage((MTGCardInstance *)source, _target, sourceamount);
                game->mLayers->stackLayer()->resolve();
            }
            else
            {
                game->mLayers->stackLayer()->addDamage((MTGCardInstance *)source, OriginalSrc, sourceamount);
                game->mLayers->stackLayer()->resolve();
            }
            if(eachother )
            {
                game->mLayers->stackLayer()->addDamage((MTGCardInstance *)_target, source, targetamount);
                game->mLayers->stackLayer()->resolve();
            }
            return 1;
            break;
        case DYNAMIC_ABILITY_EFFECT_DRAW://draw cards
            if(storedAbility)
                activateStored();
            game->mLayers->stackLayer()->addDraw((Player *)_target,sourceamount);
            game->mLayers->stackLayer()->resolve();
            return 1;
            break;
        case DYNAMIC_ABILITY_EFFECT_LIFEGAIN://gain life
            if(storedAbility)
                activateStored();
            game->mLayers->stackLayer()->addLife(_target,sourceamount);
            game->mLayers->stackLayer()->resolve();
            return 1;
            break;
        case DYNAMIC_ABILITY_EFFECT_PUMPPOWER://pump power
            {
                if(storedAbility)
                    activateStored();
                if(tosrc == false)
                {
                    PTInstant * a = NEW PTInstant(this->GetId(), source, (MTGCardInstance*)_target,NEW WParsedPT(sourceamount,0));
                    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source,(MTGCardInstance*)_target, a);
                    wrapper->addToGame();
                    return 1;
                }
                else
                {
                    PTInstant * a = NEW PTInstant(this->GetId(), source, OriginalSrc,NEW WParsedPT(sourceamount,0));
                    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source,OriginalSrc, a);
                    wrapper->addToGame();
                    return 1;
                }
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPTOUGHNESS://pump toughness
            {
                if(storedAbility)
                    activateStored();
                if(tosrc == false)
                {
                    PTInstant * a = NEW PTInstant(this->GetId(), source, (MTGCardInstance*)_target,NEW WParsedPT(0,sourceamount));
                    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source,(MTGCardInstance*)_target, a);
                    wrapper->addToGame();
                    return 1;
                }
                else
                {
                    PTInstant * a = NEW PTInstant(this->GetId(), source, OriginalSrc,NEW WParsedPT(0,sourceamount));
                    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source,OriginalSrc, a);
                    wrapper->addToGame();
                    return 1;
                }
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPBOTH://pump both
            {
                if(storedAbility)
                    activateStored();
                if(tosrc == false)
                {
                    PTInstant * a = NEW PTInstant(this->GetId(), source, (MTGCardInstance*)_target,NEW WParsedPT(sourceamount,sourceamount));
                    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source,(MTGCardInstance*)_target, a);
                    wrapper->addToGame();
                    return 1;
                }
                else
                {
                    PTInstant * a = NEW PTInstant(this->GetId(), source, OriginalSrc,NEW WParsedPT(sourceamount,sourceamount));
                    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source,OriginalSrc, a);
                    wrapper->addToGame();
                    return 1;
                }
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_LIFELOSS://lose life
            if(storedAbility)
                activateStored();
                game->mLayers->stackLayer()->addLife(_target,(sourceamount * -1));
                game->mLayers->stackLayer()->resolve();
            
            return 1;
            break;
        case DYNAMIC_ABILITY_EFFECT_DEPLETE://deplete cards
            {
                if(storedAbility)
                    activateStored();
                Player * player = (Player *)_target;
                MTGLibrary * library = player->game->library;
                for (int i = 0; i < sourceamount; i++)
                {
                    if (library->nb_cards)
                        player->game->putInZone(library->cards[library->nb_cards - 1], library, player->game->graveyard);
                }
                return 1;
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_COUNTERSONEONE:
            {
                if(_target->typeAsTarget() != TARGET_CARD)
                    _target = OriginalSrc;
                for(int j = 0;j < sourceamount;j++)
                    ((MTGCardInstance*)_target)->counters->addCounter(1,1);
                break;
            }
        default:
            return 0;
        }
    }

    return 0;
}

int AADynamic::activateStored()
{
    clonedStored = storedAbility->clone();
    clonedStored->target = target;
    if (clonedStored->oneShot)
    {
        clonedStored->resolve();
        delete (clonedStored);
    }
    else
    {
        clonedStored->addToGame();
    }
    return 1;
}

const char * AADynamic::getMenuText()
{
    if (menu.size())
    {
        return menu.c_str();
    }

    switch(type)
    {
    case 0:
        menu.append("Power");
        break;
    case 1:
        menu.append("Tough");
        break;
    case 2:
        menu.append("Mana");
        break;
    case 3:
        menu.append("color");
        break;
    case 4:
        menu.append("Elder");
        break;
    case 5:
        menu.append("Charged");
        break;
    case 6:
        menu.append("Counter");
        break;
    case 7:
        menu.append("That Many ");
        break;
    default:
        break;
    }

    switch(effect)
    {
    case 0:
        menu.append("Strike");
        break;
    case 1:
        menu.append("Draw");
        break;
    case 2:
        menu.append("Life");
        break;
    case 3:
        menu.append("Pump");
        break;
    case 4:
        menu.append("Fortify");
        break;
    case 5:
        menu.append("Buff");
        break;
    case 6:
        menu.append("Drain");
        break;
    case 7:
        menu.append("Deplete!");
        break;
    case 8:
        menu.append("Counters!");
        break;
    default:
        break;
    }
    sprintf(menuText, "%s", menu.c_str());
    return menuText;
}

AADynamic * AADynamic::clone() const
{
    AADynamic * a = NEW AADynamic(*this);
    a->isClone = 1;
    return a;
}

AADynamic::~AADynamic()
{
    if (!isClone)
        SAFE_DELETE(storedAbility);
}

//AALifer
AALifer::AALifer(int _id, MTGCardInstance * card, Targetable * _target, string life_s, ManaCost * _cost, int _tap, int who) :
ActivatedAbilityTP(_id, card, _target, _cost, _tap, who),life_s(life_s)
{
    aType = MTGAbility::LIFER;
}

int AALifer::resolve()
{  
    Damageable * _target = (Damageable *) getTarget();
    if (!_target)
        return 0;

    WParsedInt life(life_s, NULL, source);
    if (_target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
    {
        _target = ((MTGCardInstance *) _target)->controller();
    }
    Player *player = (Player*)_target;
    player->gainOrLoseLife(life.getValue());

    return 1;
}

int AALifer::getLife()
{
    WParsedInt life(life_s, NULL, source);
    return life.getValue();
}

const char * AALifer::getMenuText()
{
    if(getLife() < 0)
        return "Life Loss";
    return "Life";
}

AALifer * AALifer::clone() const
{
    AALifer * a = NEW AALifer(*this);
    a->isClone = 1;
    return a;
}



//Lifeset
AALifeSet::AALifeSet(int _id, MTGCardInstance * _source, Targetable * _target, WParsedInt * life, ManaCost * _cost, int doTap,
        int who) :
    ActivatedAbilityTP(_id, _source, _target, _cost, doTap, who), life(life)
{
}

int AALifeSet::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (!_target)
        return 0;

    Player * p = NULL;
    if (_target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
    {
        p = ((MTGCardInstance *) _target)->controller();
    }
    else
    {
        p = (Player*)_target;
    }

    int lifeDiff = life->getValue() - p->life ;
    p->gainOrLoseLife(lifeDiff);

    return 1;
}

const char * AALifeSet::getMenuText()
{
    return "Set Life";
}

AALifeSet * AALifeSet::clone() const
{
    AALifeSet * a = NEW AALifeSet(*this);
    a->life = NEW WParsedInt(*(a->life));
    a->isClone = 1;
    return a;
}

AALifeSet::~AALifeSet()
{
    SAFE_DELETE(life);
}

//AACloner 
//cloning...this makes a token thats a copy of the target.
AACloner::AACloner(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int who,
        string abilitiesStringList) :
    ActivatedAbility(_id, _source, _cost, 0, 0), who(who)
{
    aType = MTGAbility::CLONING;
    target = _target;
    source = _source;
    if (abilitiesStringList.size() > 0)
    {
        PopulateAbilityIndexVector(awith, abilitiesStringList);
        PopulateColorIndexVector(colors, abilitiesStringList);
    }

}

    int AACloner::resolve()
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if (_target)
        {
            MTGCardInstance * myClone = NULL;
            MTGCard* clone = (_target->isToken ? _target: MTGCollection()->getCardByName(_target->name));
            if (who != 1)
            {
                myClone = NEW MTGCardInstance(clone, source->controller()->game);
            }
            if (who == 1)
            {
                myClone = NEW MTGCardInstance(clone, source->controller()->opponent()->game);
            }
            if (who != 1)
                source->controller()->game->temp->addCard(myClone);
            else
                source->controller()->opponent()->game->temp->addCard(myClone);
            Spell * spell = NEW Spell(myClone);
            spell->resolve();
            spell->source->isToken = 1;
            spell->source->fresh = 1;
            if(_target->isToken)
            {
            spell->source->power = _target->origpower;
            spell->source->toughness = _target->origtoughness;
            spell->source->life = _target->origtoughness;
            }
            list<int>::iterator it;
            for (it = awith.begin(); it != awith.end(); it++)
        {
            spell->source->basicAbilities[*it] = 1;
        }
        for (it = colors.begin(); it != colors.end(); it++)
        {
            spell->source->setColor(*it);
        }
        delete spell;
        return 1;
    }
    return 0;
}

const char * AACloner::getMenuText()
{
    if (who == 1)
        return "Clone For Opponent";
    return "Clone";
}

ostream& AACloner::toString(ostream& out) const
{
    out << "AACloner ::: with : ?" // << abilities
            << " (";
    return ActivatedAbility::toString(out) << ")";
}

AACloner * AACloner::clone() const
{
    AACloner * a = NEW AACloner(*this);
    a->isClone = 1;
    return a;
}
AACloner::~AACloner()
{
}

// Cast/Play Restriction modifier
ACastRestriction::ACastRestriction(int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who) :
   AbilityTP(_id, card, _target, who), restrictionsScope(_restrictionsScope), value(_value), modifyExisting(_modifyExisting),zoneId(_zoneId)
{
    existingRestriction = NULL;
    targetPlayer = NULL;
}

int ACastRestriction::addToGame()
{
    Targetable * _target = getTarget();

    if (_target)
    {
        if (_target->typeAsTarget() == TARGET_CARD)
        {
            targetPlayer = ((MTGCardInstance *) _target)->controller();
        }
        else
        {
            targetPlayer = (Player *) _target;
        }
        if (modifyExisting)
        {
            //For now the only modifying rule is the one for lands, so this is hardcoded here.
            //This means that a modifying rule for anything lands will actually modify the lands rule.
            //In the future, we need a way to "identify" rules that modify an existing restriction, probably by doing a comparison of the TargetChoosers
            existingRestriction = targetPlayer->game->playRestrictions->getMaxPerTurnRestrictionByTargetChooser(restrictionsScope);
            if(existingRestriction && existingRestriction->maxPerTurn != MaxPerTurnRestriction::NO_MAX)
                existingRestriction->maxPerTurn += value->getValue();
        }
        else
        {
            TargetChooser * _tc = restrictionsScope->clone();
            existingRestriction = NEW MaxPerTurnRestriction(_tc, value->getValue(), MTGGameZone::intToZone(zoneId, targetPlayer));
            targetPlayer->game->playRestrictions->addRestriction(existingRestriction);

        }
        AbilityTP::addToGame();
    }
    return 0;
}

int ACastRestriction::destroy()
{
    if (!existingRestriction)
        return 0;

    if (modifyExisting)
    {
        if(existingRestriction->maxPerTurn != MaxPerTurnRestriction::NO_MAX)
            existingRestriction->maxPerTurn -= value->getValue();
    }
    else
    {
         targetPlayer->game->playRestrictions->removeRestriction(existingRestriction);
         SAFE_DELETE(existingRestriction);
    }
    return 1;
}

const char * ACastRestriction::getMenuText()
{
    if (modifyExisting)
        return "Additional Lands"; //hardoced because only the lands rule allows to modify existing rule for now
    return "Cast Restriction";
}

ACastRestriction * ACastRestriction::clone() const
{
    ACastRestriction * a = NEW ACastRestriction(*this);
    a->value = NEW WParsedInt(*(a->value));
    a->restrictionsScope = restrictionsScope->clone();
    a->isClone = 1;
    return a;
}

ACastRestriction::~ACastRestriction()
{
    SAFE_DELETE(value);
    SAFE_DELETE(restrictionsScope);
}


AInstantCastRestrictionUEOT::AInstantCastRestrictionUEOT(int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who) :
    InstantAbilityTP(_id, card, _target, who)
{
    ability = NEW ACastRestriction(_id, card, _target, _restrictionsScope, _value, _modifyExisting,  _zoneId, who);
}

int AInstantCastRestrictionUEOT::resolve()
{
    ACastRestriction * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}
const char * AInstantCastRestrictionUEOT::getMenuText()
{
    return ability->getMenuText();
}

AInstantCastRestrictionUEOT * AInstantCastRestrictionUEOT::clone() const
{
    AInstantCastRestrictionUEOT * a = NEW AInstantCastRestrictionUEOT(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

AInstantCastRestrictionUEOT::~AInstantCastRestrictionUEOT()
{
    SAFE_DELETE(ability);
}


//AAMover
AAMover::AAMover(int _id, MTGCardInstance * _source, MTGCardInstance * _target, string dest, ManaCost * _cost, int doTap) :
    ActivatedAbility(_id, _source, _cost, 0, doTap), destination(dest)
{
    if (_target)
        target = _target;
}

MTGGameZone * AAMover::destinationZone()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    return MTGGameZone::stringToZone(destination, source, _target);
}

int AAMover::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        Player* p = _target->controller();
        if (p)
        {
            GameObserver * g = GameObserver::GetInstance();
            MTGGameZone * fromZone = _target->getCurrentZone();
            MTGGameZone * destZone = destinationZone();

            //inplay is a special zone !
            for (int i = 0; i < 2; i++)
            {
                if (destZone == g->players[i]->game->inPlay && fromZone != g->players[i]->game->inPlay && fromZone
                        != g->players[i]->opponent()->game->inPlay)
                {
                    MTGCardInstance * copy = g->players[i]->game->putInZone(_target, fromZone, g->players[i]->game->temp);
                    Spell * spell = NEW Spell(copy);
                    spell->resolve();
                    delete spell;
                    return 1;
                }
            }
            p->game->putInZone(_target, fromZone, destZone);
            return 1;
        }
    }
    return 0;
}

const char * AAMover::getMenuText()
{
    return "Move";
}

AAMover * AAMover::clone() const
{
    AAMover * a = NEW AAMover(*this);
    a->isClone = 1;
    return a;
}

//Random Discard
AARandomDiscarder::AARandomDiscarder(int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost,
        int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who), nbcardsStr(nbcardsStr)
{
}

int AARandomDiscarder::resolve()
{
    Targetable * _target = getTarget();
    Player * player;
    if (_target)
    {
        if (_target->typeAsTarget() == TARGET_CARD)
        {
            player = ((MTGCardInstance *) _target)->controller();
        }
        else
        {
            player = (Player *) _target;
        }


        WParsedInt numCards(nbcardsStr, NULL, source);
        for (int i = 0; i < numCards.intValue; i++)
        {
            player->game->discardRandom(player->game->hand, source);
        }
    }
            
    return 1;
}

const char * AARandomDiscarder::getMenuText()
{
    return "Discard Random";
}

AARandomDiscarder * AARandomDiscarder::clone() const
{
    AARandomDiscarder * a = NEW AARandomDiscarder(*this);
    a->isClone = 1;
    return a;
}

// Shuffle 
AAShuffle::AAShuffle(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who)
{
}

int AAShuffle::resolve()
{
    Targetable * _target = getTarget();
    Player * player;
    if (_target)
    {
        if (_target->typeAsTarget() == TARGET_CARD)
        {
            player = ((MTGCardInstance *) _target)->controller();
        }
        else
        {
            player = (Player *) _target;
        }
        MTGLibrary * library = player->game->library;
        library->shuffle();
    }
    return 1;
}

const char * AAShuffle::getMenuText()
{
    return "Shuffle";
}

AAShuffle * AAShuffle::clone() const
{
    AAShuffle * a = NEW AAShuffle(*this);
    a->isClone = 1;
    return a;
}

//Tapper
AATapper::AATapper(int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost, int doTap) :
    ActivatedAbility(id, card, _cost, 0, doTap)
{
    target = _target;
    aType = MTGAbility::TAPPER;
}

int AATapper::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next; //This is for cards such as rampant growth
        _target->tap();
    }
    return 1;
}

const char * AATapper::getMenuText()
{
    return "Tap";
}

AATapper * AATapper::clone() const
{
    AATapper * a = NEW AATapper(*this);
    a->isClone = 1;
    return a;
}

//AA Untapper
AAUntapper::AAUntapper(int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost, int doTap) :
    ActivatedAbility(id, card, _cost, 0, doTap)
{
    target = _target;
    aType = MTGAbility::UNTAPPER;
}

int AAUntapper::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next; //This is for cards such as rampant growth
        _target->untap();
    }
    return 1;
}

const char * AAUntapper::getMenuText()
{
    return "Untap";
}

AAUntapper * AAUntapper::clone() const
{
    AAUntapper * a = NEW AAUntapper(*this);
    a->isClone = 1;
    return a;
}

AAWhatsMax::AAWhatsMax(int id, MTGCardInstance * card, MTGCardInstance * source, ManaCost * _cost, int doTap, int value) :
    ActivatedAbility(id, card, _cost, 0, doTap), value(value)
{
}

int AAWhatsMax::resolve()
{

    if (source)
    {
        source->MaxLevelUp = value;
        source->isLeveler = 1;
    }
    return 1;
}

AAWhatsMax * AAWhatsMax::clone() const
{
    AAWhatsMax * a = NEW AAWhatsMax(*this);
    a->isClone = 1;
    return a;
}

// Win Game
AAWinGame::AAWinGame(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who)
{
}

int AAWinGame::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        if (_target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        {
            _target = ((MTGCardInstance *) _target)->controller();
        }
        int cantlosers = 0;
        MTGGameZone * z = ((Player *) _target)->opponent()->game->inPlay;
        int nbcards = z->nb_cards;

        for (int i = 0; i < nbcards; i++)
        {
            MTGCardInstance * c = z->cards[i];
            if (c->has(Constants::CANTLOSE))
            {
                cantlosers++;
            }
        }
        MTGGameZone * k = ((Player *) _target)->game->inPlay;
        int onbcards = k->nb_cards;
        for (int m = 0; m < onbcards; ++m)
        {
            MTGCardInstance * e = k->cards[m];
            if (e->has(Constants::CANTWIN))
            {
                cantlosers++;
            }
        }
        if (cantlosers < 1)
        {
            game->gameOver = ((Player *) _target)->opponent();
        }
    }
    return 1;
}

const char * AAWinGame::getMenuText()
{
    return "Win Game";
}

AAWinGame * AAWinGame::clone() const
{
    AAWinGame * a = NEW AAWinGame(*this);
    a->isClone = 1;
    return a;
}

//Generic Abilities

//May Abilities
MayAbility::MayAbility(int _id, MTGAbility * _ability, MTGCardInstance * _source, bool must) :
    MTGAbility(_id, _source), NestedAbility(_ability), must(must)
{
    triggered = 0;
    mClone = NULL;
}

void MayAbility::Update(float dt)
{
    MTGAbility::Update(dt);
    if (!triggered)
    {
        triggered = 1;
        if (TargetAbility * ta = dynamic_cast<TargetAbility *>(ability))
        {
            if (!ta->tc->validTargetsExist())
                return;
        }
        game->mLayers->actionLayer()->setMenuObject(source, must);
        game->mLayers->stackLayer()->setIsInterrupting(source->controller());
    }
}

const char * MayAbility::getMenuText()
{
    return ability->getMenuText();
}

int MayAbility::testDestroy()
{
    if (!triggered)
        return 0;
    if (game->mLayers->actionLayer()->menuObject)
        return 0;
    if (game->mLayers->actionLayer()->getIndexOf(mClone) != -1)
        return 0;
    return 1;
}

int MayAbility::isReactingToTargetClick(Targetable * card)
{
    if (card == source)
        return 1;
    return 0;
}

int MayAbility::reactToTargetClick(Targetable * object)
{
    mClone = ability->clone();
    mClone->addToGame();
    mClone->forceDestroy = 1;
    return mClone->reactToTargetClick(object);
}

MayAbility * MayAbility::clone() const
{
    MayAbility * a = NEW MayAbility(*this);
    a->ability = ability->clone();
    a->isClone = 1;
    return a;
}

MayAbility::~MayAbility()
{
    SAFE_DELETE(ability);
}

//MultiAbility : triggers several actions for a cost
MultiAbility::MultiAbility(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int _tap) :
    ActivatedAbility(_id, card, _cost, 0, _tap)
{
    if (_target)
        target = _target;
}

int MultiAbility::Add(MTGAbility * ability)
{
    abilities.push_back(ability);
    return 1;
}

int MultiAbility::resolve()
{
    Targetable * Phaseactiontarget = NULL;
    vector<int>::size_type sz = abilities.size();
    for (unsigned int i = 0; i < sz; i++)
    {
        if (abilities[i] == NULL)
            continue;
        Targetable * backup = abilities[i]->target;


        if (target && target != source && abilities[i]->target == abilities[i]->source)
        {
            abilities[i]->target = target;
            Phaseactiontarget = target;
        }
        abilities[i]->resolve();
        abilities[i]->target = backup;
        if(dynamic_cast<APhaseActionGeneric *> (abilities[i]))
        {
            if(Phaseactiontarget != NULL)
                dynamic_cast<APhaseActionGeneric *> (abilities[i])->target = Phaseactiontarget;
        }

    }
    return 1;
}

const char * MultiAbility::getMenuText()
{
    if (abilities.size())
        return abilities[0]->getMenuText();
    return "";
}

MultiAbility * MultiAbility::clone() const
{
    MultiAbility * a = NEW MultiAbility(*this);
    a->isClone = 1;
    return a;
}

MultiAbility::~MultiAbility()
{
    if (!isClone)
    {
        vector<int>::size_type sz = abilities.size();
        for (size_t i = 0; i < sz; i++)
        {
            SAFE_DELETE(abilities[i]);
        }
    }
    abilities.clear();
}

//Generic Target Ability
GenericTargetAbility::GenericTargetAbility(string newName,int _id, MTGCardInstance * _source, TargetChooser * _tc, MTGAbility * a,
        ManaCost * _cost, int _tap, string limit,MTGAbility * sideEffects,string usesBeforeSideEffects, int restrictions, MTGGameZone * dest) :
    TargetAbility(_id, _source, _tc, _cost, restrictions, _tap), limit(limit), activeZone(dest),newName(newName)
{
    ability = a;
    MTGAbility * core = AbilityFactory::getCoreAbility(a);
    if (dynamic_cast<AACopier *> (core))
        tc->other = true; //http://code.google.com/p/wagic/issues/detail?id=209 (avoid inifinite loop)
    counters = 0;
}

const char * GenericTargetAbility::getMenuText()
{
    if (!ability)
        return "Error";
    if (newName.size())
        return newName.c_str();

    MTGAbility * core = AbilityFactory::getCoreAbility(ability);
    if (AAMover * move = dynamic_cast<AAMover *>(core))
    {
        MTGGameZone * dest = move->destinationZone();
        GameObserver * g = GameObserver::GetInstance();
        for (int i = 0; i < 2; i++)
        {
            if (dest == g->players[i]->game->hand && tc->targetsZone(g->players[i]->game->inPlay))
            {
                return "Bounce";
            }
            else if (dest == g->players[i]->game->hand && tc->targetsZone(g->players[i]->game->graveyard))
            {
                return "Reclaim";
            }
            else if (dest == g->players[i]->game->graveyard && tc->targetsZone(g->players[i]->game->inPlay))
            {
                return "Sacrifice";
            }
            else if (dest == g->players[i]->game->library && tc->targetsZone(g->players[i]->game->graveyard))
            {
                return "Recycle";
            }
            else if (dest == g->players[i]->game->battlefield && tc->targetsZone(g->players[i]->game->graveyard))
            {
                return "Reanimate";
            }
            else if ((tc->targetsZone(g->players[i]->game->inPlay)
                    && dest == g->players[i]->game->library)
                    || dest == g->players[i]->game->library)
            {
                return "Put in Library";
            }
            else if (dest == g->players[i]->game->inPlay)
            {
                return "Put in Play";
            }
            else if (dest == g->players[i]->game->graveyard && tc->targetsZone(g->players[i]->game->hand))
            {
                return "Discard";
            }
            else if (dest == g->players[i]->game->exile)
            {
                return "Exile";
            }
            else if (tc->targetsZone(g->players[i]->game->library))
            {
                return "Fetch";
            }
            else if (dest == g->players[i]->game->hand && tc->targetsZone(g->opponent()->game->hand))
            {
                return "Steal";
            }
            else if (dest == g->players[i]->game->graveyard && tc->targetsZone(g->opponent()->game->hand))
            {
                return "Opponent Discards";
            }
        }
    }

    return ability->getMenuText();

}

int GenericTargetAbility::resolve()
{
    counters++;
    return TargetAbility::resolve();
}

int GenericTargetAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    limitPerTurn = 0;
    if(limit.size())
    {
        WParsedInt * value = NEW WParsedInt(limit.c_str(),NULL,source);
        limitPerTurn = value->getValue();
        delete value;
    }
    if (limitPerTurn && counters >= limitPerTurn)
        return 0;
    return TargetAbility::isReactingToClick(card, mana);
}

void GenericTargetAbility::Update(float dt)
{
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_AFTER_EOT)
    {
        counters = 0;
    }
    TargetAbility::Update(dt);
}

int GenericTargetAbility::testDestroy()
{
    if (!activeZone)
        return TargetAbility::testDestroy();
    if (activeZone->hasCard(source))
        return 0;
    return 1;

}

GenericTargetAbility * GenericTargetAbility::clone() const
{
    GenericTargetAbility * a = NEW GenericTargetAbility(*this);
    a->ability = ability->clone();
    a->cost = NEW ManaCost();
    a->cost->copy(cost);
    if (tc)
        a->tc = tc->clone();
    return a;
}

GenericTargetAbility::~GenericTargetAbility()
{
    SAFE_DELETE(ability);
}

//Alter Cost
AAlterCost::AAlterCost(int id, MTGCardInstance * source, MTGCardInstance * target, int amount, int type) :
MTGAbility(id, source, target), amount(amount), type(type)
{
    manaReducer = source;
}

int AAlterCost::addToGame()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(!_target || _target->hasType("land"))
    {
        this->forceDestroy = 1;
        return MTGAbility::addToGame();
    }
    if (amount > 0)
    {
        if(!_target->getIncreasedManaCost()->getConvertedCost())
        {
            ManaCost * increased = NEW ManaCost();
            increased->init();
            _target->getIncreasedManaCost()->copy(increased);
            delete increased;

        }
        _target->getIncreasedManaCost()->add(type,amount);
    }
    else
    {
        if(!_target->getReducedManaCost()->getConvertedCost())
        {
            ManaCost * reduced = NEW ManaCost();
            reduced->init();
            _target->getReducedManaCost()->copy(reduced);
            delete reduced;
        }
        _target->getReducedManaCost()->add(type,abs(amount));
    }
    return MTGAbility::addToGame();
}

int AAlterCost::testDestroy()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if(!this->manaReducer->isInPlay())
    {
        if (amount > 0)
        {
            _target->getIncreasedManaCost()->remove(type,amount);
            refreshCost(_target);//special case for 0 cost.
        }
        else
        {
            _target->getReducedManaCost()->remove(type,abs(amount));
            refreshCost(_target);//special case for 0 cost.
        }
        return MTGAbility::testDestroy();
    }
    return 0;
}
void AAlterCost::refreshCost(MTGCardInstance * card)
{
    ManaCost * original = NEW ManaCost();
    original->copy(card->model->data->getManaCost());
    original->add(card->getIncreasedManaCost());
    original->remove(card->getReducedManaCost());
    card->getManaCost()->copy(original);
    delete original;
        return;
    }
void AAlterCost::increaseTheCost(MTGCardInstance * card)
{
    if(card->getIncreasedManaCost()->getConvertedCost())
    {
        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::MTG_NB_COLORS;k++)
        {
            card->getManaCost()->add(k,card->getIncreasedManaCost()->getCost(k));
            if (card->getManaCost()->alternative)
            {
                card->getManaCost()->alternative->add(k,card->getIncreasedManaCost()->getCost(k));
            }
            if (card->getManaCost()->BuyBack)
            {
                card->getManaCost()->BuyBack->add(k,card->getIncreasedManaCost()->getCost(k));
            }
        }
    }
    return;
}

void AAlterCost::decreaseTheCost(MTGCardInstance * card)
{
    if(card->getReducedManaCost()->getConvertedCost())
    {
        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::MTG_NB_COLORS;k++)
        {
            card->getManaCost()->remove(k,card->getReducedManaCost()->getCost(k));
            if (card->getManaCost()->alternative)
            {
                card->getManaCost()->alternative->remove(k,card->getReducedManaCost()->getCost(k));
            }
            if (card->getManaCost()->BuyBack)
            {
                card->getManaCost()->BuyBack->remove(k,card->getReducedManaCost()->getCost(k));
            }
        }
    }
    return;
}

AAlterCost * AAlterCost::clone() const
{
    AAlterCost * a = NEW AAlterCost(*this);
    a->isClone = 1;
    return a;
}

AAlterCost::~AAlterCost()
{
}

// ATransformer
ATransformer::ATransformer(int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, string sabilities,string newpower,bool newpowerfound,string newtoughness,bool newtoughnessfound,vector<string> newAbilitiesList,bool newAbilityFound,bool aForever) :
    MTGAbility(id, source, target),newpower(newpower),newpowerfound(newpowerfound),newtoughness(newtoughness),newtoughnessfound(newtoughnessfound),newAbilitiesList(newAbilitiesList),newAbilityFound(newAbilityFound),aForever(aForever)
{

    PopulateAbilityIndexVector(abilities, sabilities);
    PopulateColorIndexVector(colors, sabilities);
    
    addNewColors = false;
    if(sabilities.find("newcolors") != string::npos)
    addNewColors = true;
    //this subkeyword adds a color without removing the existing colors.
    remove = false;
    if (stypes == "removesubtypes")
        remove = true;
    removeTypes = false;
    if (stypes == "removetypes")
        removeTypes = true;
    if (stypes == "allsubtypes" || stypes == "removesubtypes")
    {
        for (int i = Subtypes::LAST_TYPE + 1;; i++)
        {
            string s = Subtypes::subtypesList->find(i);
            {
                if (s == "")
                    break;
                if (s.find(" ") != string::npos)
                    continue;
                if (s == "Nothing" || s == "Swamp" || s == "Plains" || s == "Mountain" || s == "Forest"
                        || s == "Island" || s == "Shrine" || s == "Basic" || s == "Colony" || s == "Desert"
                        || s == "Dismiss" || s == "Equipment" || s == "Everglades" || s == "Grasslands" || s == "Lair"
                        || s == "Level" || s == "Levelup" || s == "Mine" || s == "Oasis" || s == "World" || s == "Aura" || s == "Land"|| s == "Legendary" || s == "Token" || s == "Planeswalker")
                {//dont add "nothing" or land type to this card.
                }
                else
                {
                    types.push_back(i);
                }
            }
        }
    }
    else
    {
        PopulateSubtypesIndexVector(types, stypes);
    }

    menu = stypes;
}

    int ATransformer::addToGame()
    {
        MTGCardInstance * _target = NULL;
         Interruptible * action = (Interruptible *) target;
        if (action->type == ACTION_SPELL && action->state == NOT_RESOLVED)
        {
            Spell * spell = (Spell *) action;
            _target = spell->source;
            aForever = true;
            //when targeting the stack, set the effect to forever, incase the person does not use it
            //otherwise we will end up with a null pointer on the destroy.
        }
        else
        {
            _target = (MTGCardInstance *) target;
        }
        if (_target)
        {
            while (_target->next)
                _target = _target->next;
            for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
            {
                if (_target->hasColor(j))
                    oldcolors.push_back(j);
            }
            for (int j = Subtypes::LAST_TYPE + 1;; j++)
            {
                string otypes = Subtypes::subtypesList->find(j);
                if (otypes == "")
                    break;
                if (otypes.find(" ") != string::npos)
                    continue;
                if (_target->hasSubtype(j))
                {
                    oldtypes.push_back(j);
                }
            }
            list<int>::iterator it;
            for (it = colors.begin(); it != colors.end(); it++)
            {
                if(!addNewColors)
                    _target->setColor(0, 1);
            }

            for (it = types.begin(); it != types.end(); it++)
            {
                if (removeTypes)
                {
                    //remove the main types from a card, ie: hidden enchantment cycle.
                    _target->removeType(0,1);
                    _target->removeType(1,1);
                    _target->removeType(2,1);
                    _target->removeType(3,1);
                    _target->removeType(4,1);
                    _target->removeType(5,1);
                    _target->removeType(6,1);
                    _target->removeType(7,1);
                }
                if (remove)
                {
                    _target->removeType(*it);
                }
                else
                {
                    if(_target->hasSubtype(*it))
                    {
                        //we generally don't want to give a creature type creature again
                        //all it does is create a sloppy mess of the subtype line on alternative quads
                        //also creates instances where a card gained a type from an ability like this one
                        //then loses the type through another ability, when this effect is destroyed the creature regains
                        //the type, which is wrong.
                        dontremove.push_back(*it);
                    }
                    else
                    {
                        _target->addType(*it);
                    }
                }
            }
            for (it = colors.begin(); it != colors.end(); it++)
            {
                _target->setColor(*it);
            }
            for (it = abilities.begin(); it != abilities.end(); it++)
            {
                _target->basicAbilities[*it]++;
            }
            for (it = oldcolors.begin(); it != oldcolors.end(); it++)
            {
            }
            if(newAbilityFound)
            {
                for (unsigned int k = 0 ; k < newAbilitiesList.size();k++)
                {
                    AbilityFactory af;
                    MTGAbility * aNew = af.parseMagicLine(newAbilitiesList[k], 0, NULL, _target);
                    aNew->isClone = 1;
                    GenericTargetAbility * gta = dynamic_cast<GenericTargetAbility*> (aNew);
                    if (gta)
                    {
                        ((GenericTargetAbility *)aNew)->source = _target;
                        ((GenericTargetAbility *)aNew)->ability->source = _target;
                    }
                    GenericActivatedAbility * gaa = dynamic_cast<GenericActivatedAbility*> (aNew);
                    if (gaa)
                    {
                        ((GenericActivatedAbility *)aNew)->source = _target;
                        ((GenericActivatedAbility *)aNew)->ability->source = _target;
                    }
                    if (MultiAbility * abi = dynamic_cast<MultiAbility*>(aNew))
                    {
                        ((MultiAbility *)aNew)->source = _target;
                        ((MultiAbility *)aNew)->abilities[0]->source = _target;
                    }
                    aNew->target = _target;
                    aNew->source = (MTGCardInstance *) _target;
                    if(aNew->oneShot)
                    {
                        aNew->resolve();
                        delete aNew;
                    }
                    else
                        aNew->addToGame();
                    newAbilities[_target].push_back(aNew);
                }
            }
            if(newpowerfound )
            {
                WParsedInt * val = NEW WParsedInt(newpower,NULL, source);
                oldpower = _target->power;
                _target->power += val->getValue();
                _target->power -= oldpower;
                delete val;
            }
            if(newtoughnessfound )
            {
                WParsedInt * val = NEW WParsedInt(newtoughness,NULL, source);
                oldtoughness = _target->toughness;
                _target->addToToughness(val->getValue());
                _target->addToToughness(-oldtoughness);
                _target->life = _target->toughness;
                delete val;
            }
        }
        return MTGAbility::addToGame();
    }

int ATransformer::destroy()
{
    if(aForever)
        return 0;
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next;
        list<int>::iterator it;
        for (it = types.begin(); it != types.end(); it++)
        {
            if (!remove)
            {
                bool removing = true;
                for(unsigned int k = 0;k < dontremove.size();k++)
                {
                    if(dontremove[k] == *it)
                        removing = false;
                }
                if(removing)
                    _target->removeType(*it);
            }
            //iterators annoy me :/
        }
        for (it = colors.begin(); it != colors.end(); it++)
        {
            _target->removeColor(*it);
        }
        for (it = abilities.begin(); it != abilities.end(); it++)
        {
            _target->basicAbilities[*it]--;
        }
        for (it = oldcolors.begin(); it != oldcolors.end(); it++)
        {
            _target->setColor(*it);
        }
        if (remove)
        {
            for (it = oldtypes.begin(); it != oldtypes.end(); it++)
            {
                if (!_target->hasSubtype(*it))
                    _target->addType(*it);
            }
        }
        if(newpowerfound )
        {
            _target->power = oldpower;
        }
        if(newtoughnessfound )
        {
            _target->toughness = oldtoughness;
        }
        if(newAbilityFound)
        {
            for (unsigned int i = 0;i < newAbilities[_target].size(); i++)
            {
                newAbilities[_target].at(i)->forceDestroy = 1;
            }
            if (newAbilities.find(_target) != newAbilities.end())
            {
                newAbilities.erase(_target);
            }
        }
    }
    return 1;
}

const char * ATransformer::getMenuText()
{
    string s = menu;
    sprintf(menuText, "Becomes %s", s.c_str());
    return menuText;
}

ATransformer * ATransformer::clone() const
{
    ATransformer * a = NEW ATransformer(*this);
    a->isClone = 1;
    return a;
}

ATransformer::~ATransformer()
{
}

//ATransformerInstant
ATransformerInstant::ATransformerInstant(int id, MTGCardInstance * source, MTGCardInstance * target, string types, string abilities,string newpower,bool newpowerfound,string newtoughness,bool newtoughnessfound,vector<string>newAbilitiesList,bool newAbilityFound,bool aForever) :
    InstantAbility(id, source, target),newpower(newpower),newpowerfound(newpowerfound),newtoughness(newtoughness),newtoughnessfound(newtoughnessfound),newAbilitiesList(newAbilitiesList),newAbilityFound(newAbilityFound),aForever(aForever)
{
    ability = NEW ATransformer(id, source, target, types, abilities,newpower,newpowerfound,newtoughness,newtoughnessfound,newAbilitiesList,newAbilityFound,aForever);
    aType = MTGAbility::STANDARD_BECOMES;
}

int ATransformerInstant::resolve()
{
    ATransformer * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}
const char * ATransformerInstant::getMenuText()
{
    return ability->getMenuText();
}

ATransformerInstant * ATransformerInstant::clone() const
{
    ATransformerInstant * a = NEW ATransformerInstant(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

ATransformerInstant::~ATransformerInstant()
{
    SAFE_DELETE(ability);
}
//P/t ueot
PTInstant::PTInstant(int id, MTGCardInstance * source, MTGCardInstance * target, WParsedPT * wppt,string s,bool nonstatic) :
InstantAbility(id, source, target), wppt(wppt),s(s),nonstatic(nonstatic)
{
    ability = NEW APowerToughnessModifier(id, source, target, wppt,s,nonstatic);
    aType = MTGAbility::STANDARD_PUMP;
}

int PTInstant::resolve()
{
    APowerToughnessModifier * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}
const char * PTInstant::getMenuText()
{
    return ability->getMenuText();
}

PTInstant * PTInstant::clone() const
{
    PTInstant * a = NEW PTInstant(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

PTInstant::~PTInstant()
{
    SAFE_DELETE(ability);
}

// ASwapPTUEOT
ASwapPTUEOT::ASwapPTUEOT(int id, MTGCardInstance * source, MTGCardInstance * target) :
    InstantAbility(id, source, target)
{
    ability = NEW ASwapPT(id, source, target);
}

int ASwapPTUEOT::resolve()
{
    ASwapPT * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}

const char * ASwapPTUEOT::getMenuText()
{
    return ability->getMenuText();
}

ASwapPTUEOT * ASwapPTUEOT::clone() const
{
    ASwapPTUEOT * a = NEW ASwapPTUEOT(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

ASwapPTUEOT::~ASwapPTUEOT()
{
    SAFE_DELETE(ability);
}

//APreventDamageTypes
APreventDamageTypes::APreventDamageTypes(int id, MTGCardInstance * source, string to, string from, int type) :
    MTGAbility(id, source), to(to), from(from), type(type)
{
    re = NULL;
}

int APreventDamageTypes::addToGame()
{
    if (re)
    {
        DebugTrace("FATAL:re shouldn't be already set in APreventDamageTypes\n");
        return 0;
    }
    TargetChooserFactory tcf;
    TargetChooser *toTc = tcf.createTargetChooser(to, source, this);
    if (toTc)
        toTc->targetter = NULL;
    TargetChooser *fromTc = tcf.createTargetChooser(from, source, this);
    if (fromTc)
        fromTc->targetter = NULL;
    if (type != 1 && type != 2)
    {//not adding this creates a memory leak.
        re = NEW REDamagePrevention(this, fromTc, toTc, -1, false, DAMAGE_COMBAT);
    }
    else if (type == 1)
    {
        re = NEW REDamagePrevention(this, fromTc, toTc, -1, false, DAMAGE_ALL_TYPES);
    }
    else if (type == 2)
    {
        re = NEW REDamagePrevention(this, fromTc, toTc, -1, false, DAMAGE_OTHER);
    }
    game->replacementEffects->add(re);
    return MTGAbility::addToGame();
}

int APreventDamageTypes::destroy()
{
    game->replacementEffects->remove(re);
    SAFE_DELETE(re);
    return 1;
}

APreventDamageTypes * APreventDamageTypes::clone() const
{
    APreventDamageTypes * a = NEW APreventDamageTypes(*this);
    a->isClone = 1;
    return a;
}

APreventDamageTypes::~APreventDamageTypes()
{
    SAFE_DELETE(re);
}

//APreventDamageTypesUEOT
APreventDamageTypesUEOT::APreventDamageTypesUEOT(int id, MTGCardInstance * source, string to, string from, int type) :
    InstantAbility(id, source)
{
    ability = NEW APreventDamageTypes(id, source, to, from, type);
}

int APreventDamageTypesUEOT::resolve()
{
    APreventDamageTypes * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}

int APreventDamageTypesUEOT::destroy()
{
    for (size_t i = 0; i < clones.size(); ++i)
    {
        clones[i]->forceDestroy = 0;
    }
    clones.clear();
    return 1;
}

const char * APreventDamageTypesUEOT::getMenuText()
{
    return ability->getMenuText();
}

APreventDamageTypesUEOT * APreventDamageTypesUEOT::clone() const
{
    APreventDamageTypesUEOT * a = NEW APreventDamageTypesUEOT(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

APreventDamageTypesUEOT::~APreventDamageTypesUEOT()
{
    SAFE_DELETE(ability);
}

//AVanishing creature also fading
AVanishing::AVanishing(int _id, MTGCardInstance * card, ManaCost * _cost, int _tap, int restrictions, int amount, string counterName) :
MTGAbility(_id, source, target),amount(amount),counterName(counterName)
{
    target = card;
    source = card;
    next = 0;
    for(int i = 0;i< amount;i++)
        source->counters->addCounter(counterName.c_str(),0,0);
}

void AVanishing::Update(float dt)
{
    if (newPhase != currentPhase && source->controller() == game->currentPlayer)
    {
        if(newPhase == Constants::MTG_PHASE_UPKEEP)
        {
            source->counters->removeCounter(counterName.c_str(),0,0);
            Counter * targetCounter = NULL;
            timeLeft = 0;

            if (source->counters && source->counters->hasCounter(counterName.c_str(), 0, 0))
            {
                targetCounter = source->counters->hasCounter(counterName.c_str(), 0, 0);
                timeLeft = targetCounter->nb;
            }
            else
            {
                timeLeft = 0;
                if(counterName.find("fade") != string::npos && next == 0)
                {
                    next = 1;
                }
                else
                {
                    next = 0;
                }
                if (newPhase == Constants::MTG_PHASE_UPKEEP && timeLeft <= 0 && next == 0)
                {
                    WEvent * e = NEW WEventCardSacrifice(source);
                    GameObserver * game = GameObserver::GetInstance();
                    game->receiveEvent(e);
                    source->controller()->game->putInGraveyard(source);
                }
            }
        }
    }
    MTGAbility::Update(dt);
}

int AVanishing::resolve()
{

    return 1;
}

const char * AVanishing::getMenuText()
{
if(counterName.find("fade") != string::npos)
return "Fading";
    return "Vanishing";
}

AVanishing * AVanishing::clone() const
{
    AVanishing * a = NEW AVanishing(*this);
    a->isClone = 1;
    return a;
}

AVanishing::~AVanishing()
{
}

//AUpkeep
AUpkeep::AUpkeep(int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int _tap, int restrictions, int _phase,
        int _once,bool Cumulative) :
    ActivatedAbility(_id, card, _cost, restrictions, _tap), NestedAbility(a), phase(_phase), once(_once),Cumulative(Cumulative)
{
    paidThisTurn = 0;
    aType = MTGAbility::UPCOST;
}

    int AUpkeep::receiveEvent(WEvent * event)
    {
        if (WEventPhaseChange* pe = dynamic_cast<WEventPhaseChange*>(event))
        {
            if (Constants::MTG_PHASE_DRAW == pe->to->id)
            {
                if (source->controller() == game->currentPlayer && once < 2 && paidThisTurn < 1)
                {
                    ability->resolve();
                }
            }
        }
        return 1;
    }
    
void AUpkeep::Update(float dt)
{
    // once: 0 means always go off, 1 means go off only once, 2 means go off only once and already has.
    if (newPhase != currentPhase && source->controller() == game->currentPlayer && once < 2)
    {
        if (newPhase == Constants::MTG_PHASE_UNTAP)
        {
            paidThisTurn = 0;
        }
        else if(newPhase == Constants::MTG_PHASE_UPKEEP && Cumulative )
        {
            source->counters->addCounter("age",0,0);
                Counter * targetCounter = NULL;
                currentage = 0;

                if (source->counters && source->counters->hasCounter("age", 0, 0))
                {
                    targetCounter = source->counters->hasCounter("age", 0, 0);
                    currentage = targetCounter->nb - 1;
                }
            if(currentage)
                paidThisTurn -= currentage;
        }
        if (newPhase == phase + 1 && once)
            once = 2;
    }
    ActivatedAbility::Update(dt);
}

int AUpkeep::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (currentPhase != phase || paidThisTurn > 0 || once >= 2)
        return 0;
    return ActivatedAbility::isReactingToClick(card, mana);
}

int AUpkeep::resolve()
{
    paidThisTurn += 1;
    return 1;
}

const char * AUpkeep::getMenuText()
{
    return "Upkeep";
}

ostream& AUpkeep::toString(ostream& out) const
{
    out << "AUpkeep ::: paidThisTurn : " << paidThisTurn << " (";
    return ActivatedAbility::toString(out) << ")";
}

AUpkeep * AUpkeep::clone() const
{
    AUpkeep * a = NEW AUpkeep(*this);
    a->isClone = 1;
    return a;
}

AUpkeep::~AUpkeep()
{
    if (!isClone)
        SAFE_DELETE(ability);
}

//A Phase based Action
APhaseAction::APhaseAction(int _id, MTGCardInstance * card, MTGCardInstance * target, string sAbility, int _tap, int restrictions, int _phase,bool forcedestroy,bool next,bool myturn,bool opponentturn) :
MTGAbility(_id, card),sAbility(sAbility), phase(_phase),forcedestroy(forcedestroy),next(next),myturn(myturn),opponentturn(opponentturn)
{
    abilityId = _id;
    abilityOwner = card->controller();
    psMenuText = "";
    AbilityFactory af;
    ability = af.parseMagicLine(sAbility, abilityId, NULL, NULL);
    if(ability)
        psMenuText = ability->getMenuText();
    else
        psMenuText = sAbility.c_str();
    delete (ability);
}

void APhaseAction::Update(float dt)
{
    if (newPhase != currentPhase)
    {
        if((myturn && game->currentPlayer == source->controller())|| 
            (opponentturn && game->currentPlayer != source->controller())/*||*/
            /*(myturn && opponentturn)*/)
        {
            if(newPhase == phase && next )
            {
                MTGCardInstance * _target = NULL;
                if(target)
                _target = (MTGCardInstance *) target;
                if (_target)
                {
                    while (_target->next)
                        _target = _target->next;
                }
                if(!sAbility.size() || !target || !_target->isInPlay())
                {
                //im aware that adding the isinplay check restricts this ability to having targets
                //which are in play..however after reviewing all the coded cards which use this
                //none of them targeted this effect at something that is not inplay.
                //the reason for this abilities that use this are generally combat abilities, and
                //without this check, the ability contenues on forever, when used in a trigger which
                //sets its ability to oneshot=0.
                    this->forceDestroy = 1;
                    return;
                }
                AbilityFactory af;
                MTGAbility * ability = af.parseMagicLine(sAbility, abilityId, NULL, _target);

                MTGAbility * a = ability->clone();
                a->target = _target;
                a->resolve();
                delete (a);
                delete (ability);
                if(this->oneShot)
                {
                    this->forceDestroy = 1;
                }
            }
            else if(newPhase == phase && next == false)
                next = true;
        }
    }
    MTGAbility::Update(dt);
}

int APhaseAction::resolve()
{
    return 0;
}

const char * APhaseAction::getMenuText()
{
    if(psMenuText.size())
    return psMenuText.c_str();
    else
    return "Phase Based Action";
}

APhaseAction * APhaseAction::clone() const
{
    APhaseAction * a = NEW APhaseAction(*this);
    if(forcedestroy == false)
        a->forceDestroy = -1;// we want this ability to stay alive until it resolves.
    a->isClone = 1;
    return a;
}

APhaseAction::~APhaseAction()
{

}

// the main ability
APhaseActionGeneric::APhaseActionGeneric(int _id, MTGCardInstance * card, MTGCardInstance * target, string sAbility, int _tap, int restrictions, int _phase,bool forcedestroy,bool next,bool myturn,bool opponentturn) :
    InstantAbility(_id, source, target)
{
    MTGCardInstance * _target = target;
    ability = NEW APhaseAction(_id, card,_target, sAbility,_tap, restrictions, _phase,forcedestroy,next,myturn,opponentturn);
}

int APhaseActionGeneric::resolve()
{
        APhaseAction * a = ability->clone();
        a->target = target;
        a->addToGame();
        return 1;
}

const char * APhaseActionGeneric::getMenuText()
{
    return ability->getMenuText();
}

APhaseActionGeneric * APhaseActionGeneric::clone() const
{
    APhaseActionGeneric * a = NEW APhaseActionGeneric(*this);
    a->ability = this->ability->clone();
    a->oneShot = 1;
    a->isClone = 1;
    return a;
}

APhaseActionGeneric::~APhaseActionGeneric()
{
    SAFE_DELETE(ability);
}

//a blink
ABlink::ABlink(int _id, MTGCardInstance * card, MTGCardInstance * _target,bool blinkueot,bool blinkForSource,bool blinkhand,MTGAbility * stored) :
MTGAbility(_id, card),blinkueot(blinkueot),blinkForSource(blinkForSource),blinkhand(blinkhand),stored(stored)
{
    target = _target;
    Blinked = NULL;
    resolved = false;
}

void ABlink::Update(float dt)
{
    if(resolved == false)
    {
        resolved = true;
        resolveBlink();
    }
    GameObserver * game = game->GetInstance();
    if ((blinkueot && currentPhase == Constants::MTG_PHASE_ENDOFTURN)||(blinkForSource && !source->isInPlay()))
    {
        if(Blinked == NULL)
            MTGAbility::Update(dt);
        MTGCardInstance * _target = Blinked;
        MTGCardInstance * Blinker = NULL;
        if(!blinkhand)
            Blinker = _target->controller()->game->putInZone(_target, _target->currentZone,
            _target->owner->game->battlefield);
        if(blinkhand)
        {
            _target->controller()->game->putInZone(_target, _target->currentZone,
                _target->owner->game->hand);
            return;
        }
        Spell * spell = NEW Spell(Blinker);
        spell->source->counters->init();
        if(spell->source->hasSubtype(Subtypes::TYPE_AURA) && !blinkhand)
        {
            TargetChooserFactory tcf;
            TargetChooser * tc = tcf.createTargetChooser(spell->source->spellTargetType,spell->source);
            if(!tc->validTargetsExist())
            {
                spell->source->owner->game->putInExile(spell->source);
                delete spell;
                delete tc;
                this->forceDestroy = 1;
                return;
            }

            MTGGameZone * inplay = spell->source->owner->game->inPlay;
            spell->source->target = NULL;
            for(int i = WRand()%inplay->nb_cards;;i = WRand()%inplay->nb_cards)
            {
                if(tc->canTarget(inplay->cards[i]) && spell->source->target == NULL)
                {
                    spell->source->target = inplay->cards[i];
                    spell->getNextCardTarget();
                    spell->resolve();

                    delete spell;
                    delete tc;
                    this->forceDestroy = 1;
                    return;
                }
                if(!tc->validTargetsExist())
                return;
            }
        }
        spell->source->power = spell->source->origpower;
        spell->source->toughness = spell->source->origtoughness;
        if(!spell->source->hasSubtype(Subtypes::TYPE_AURA))
        {
            spell->resolve();
            if(stored)
            {
                MTGAbility * clonedStored = stored->clone();
                clonedStored->target = spell->source;
                if (clonedStored->oneShot)
                {
                    clonedStored->resolve();
                    delete (clonedStored);
                }
                else
                {
                    clonedStored->addToGame();
                }
            }
        }
        delete spell;
        this->forceDestroy = 1;
        Blinker = NULL;
        return;
    }
    MTGAbility::Update(dt);
}

void ABlink::resolveBlink()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        if(blinkhand && !_target->controller()->game->isInZone(_target,_target->controller()->game->hand))
        {
            this->forceDestroy = 1;
            return;
        }
        else if(!blinkhand && !_target->controller()->game->isInZone(_target,_target->controller()->game->battlefield))
        {
            this->forceDestroy = 1;
            return;
        }
        _target->controller()->game->putInZone(_target, _target->currentZone,
            _target->owner->game->exile);
        if(_target->isToken)
        {
            //if our target is a token, we're done as soon as its sent to exile.
            this->forceDestroy = 1;
            return;
        }
        _target = _target->next;
        Blinked = _target;
        if(!blinkueot && !blinkForSource)
        {
            MTGCardInstance * Blinker = NULL;
            if(!blinkhand)
                Blinker = _target->controller()->game->putInZone(_target, _target->currentZone,
                _target->owner->game->battlefield);
            if(blinkhand)
            {
                _target->controller()->game->putInZone(_target, _target->currentZone,
                    _target->owner->game->hand);
                return;
            }
            Spell * spell = NEW Spell(Blinker);
            spell->source->counters->init();
            if(spell->source->hasSubtype(Subtypes::TYPE_AURA) && !blinkhand)
            {
                TargetChooserFactory tcf;
                TargetChooser * tc = tcf.createTargetChooser(spell->source->spellTargetType,spell->source);
                if(!tc->validTargetsExist())
                {
                    spell->source->owner->game->putInExile(spell->source);
                    delete spell;
                    delete tc;
                    this->forceDestroy = 1;
                    return;
                }

                MTGGameZone * inplay = spell->source->owner->game->inPlay;
                spell->source->target = NULL;
                for(int i = WRand()%inplay->nb_cards;;i = WRand()%inplay->nb_cards)
                {
                    if(tc->canTarget(inplay->cards[i]) && spell->source->target == NULL)
                    {
                        spell->source->target = inplay->cards[i];
                        spell->getNextCardTarget();
                        spell->resolve();
                        delete spell;
                        delete tc;
                        this->forceDestroy = 1;
                        return;
                    }
                }
            }
            spell->source->power = spell->source->origpower;
            spell->source->toughness = spell->source->origtoughness;
            spell->resolve();
            if(stored)
            {
                MTGAbility * clonedStored = stored->clone();
                clonedStored->target = spell->source;
                if (clonedStored->oneShot)
                {
                    clonedStored->resolve();
                    delete (clonedStored);
                }
                else
                {
                    clonedStored->addToGame();
                }
            }
            delete tc;
            delete spell;
            this->forceDestroy = 1;
            if(stored)
            delete(stored);
            Blinked = NULL;
        }
    }
}

int ABlink::resolve()
{
    return 0;
}
const char * ABlink::getMenuText()
{
    return "Blink";
}

ABlink * ABlink::clone() const
{
    ABlink * a = NEW ABlink(*this);
    a->isClone = 1;
    a->forceDestroy = -1;
    return a;
};
ABlink::~ABlink()
{
    if (!isClone)
        SAFE_DELETE(stored);
}

ABlinkGeneric::ABlinkGeneric(int _id, MTGCardInstance * card, MTGCardInstance * _target,bool blinkueot,bool blinkForSource,bool blinkhand,MTGAbility * stored) :
    InstantAbility(_id, source, _target)
{
    ability = NEW ABlink(_id,card,_target,blinkueot,blinkForSource,blinkhand,stored);
}

int ABlinkGeneric::resolve()
{
        ABlink * a = ability->clone();
        a->target = target;
        a->addToGame();
        return 1;
}

const char * ABlinkGeneric::getMenuText()
{
    return "Blink";
}

ABlinkGeneric * ABlinkGeneric::clone() const
{
    ABlinkGeneric * a = NEW ABlinkGeneric(*this);
    a->ability = this->ability->clone();
    a->oneShot = 1;
    a->isClone = 1;
    return a;
}

ABlinkGeneric::~ABlinkGeneric()
{
    SAFE_DELETE(ability);
}

// utility functions

// Given a delimited string of abilities, add the ones to the list that are "Basic"  MTG abilities
void PopulateAbilityIndexVector(list<int>& abilities, const string& abilityStringList, char delimiter)
{
    vector<string> abilitiesList = split(abilityStringList, delimiter);
    for (vector<string>::iterator iter = abilitiesList.begin(); iter != abilitiesList.end(); ++iter)
    {
        int abilityIndex = Constants::GetBasicAbilityIndex(*iter);

        if (abilityIndex != -1)
            abilities.push_back(abilityIndex);
    }
}

void PopulateColorIndexVector(list<int>& colors, const string& colorStringList, char delimiter)
{
    vector<string> abilitiesList = split(colorStringList, delimiter);
    for (vector<string>::iterator iter = abilitiesList.begin(); iter != abilitiesList.end(); ++iter)
    {
        for (int colorIndex = Constants::MTG_COLOR_ARTIFACT; colorIndex < Constants::MTG_NB_COLORS; ++colorIndex)
        {
            // if the text is not a basic ability but contains a valid color add it to the color vector
            if ((Constants::GetBasicAbilityIndex(*iter) == -1)
                    && ((*iter).find(Constants::MTGColorStrings[colorIndex]) != string::npos))
                colors.push_back(colorIndex);
        }
    }
}

void PopulateSubtypesIndexVector(list<int>& types, const string& subTypesStringList, char delimiter)
{
    vector<string> subTypesList = split(subTypesStringList, delimiter);
    for (vector<string>::iterator it = subTypesList.begin(); it != subTypesList.end(); ++it)
    {
        string subtype = *it;
        size_t id = Subtypes::subtypesList->find(subtype);
        if (id != string::npos)
            types.push_back(id);
    }
}
