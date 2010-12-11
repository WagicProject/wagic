#include "PrecompiledHeader.h"
#include "AllAbilities.h"

//Activated Abilities

//Generic Activated Abilities
GenericActivatedAbility::GenericActivatedAbility(int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int _tap,
        int limit, int restrictions, MTGGameZone * dest) :
    ActivatedAbility(_id, card, _cost, restrictions, _tap), NestedAbility(a), limitPerTurn(limit), activeZone(dest)
{
    counters = 0;
    target = ability->target;
}

int GenericActivatedAbility::resolve()
{
    counters++;
    ManaCost * diff = abilityCost->Diff(cost);
    source->X = diff->hasX();
    SAFE_DELETE(diff);
    //SAFE_DELETE(abilityCost); this line has been reported as a bug. removing it doesn't seem to break anything, although I didn't get any error in the test suite by leaving it either, so... leaving it for now as a comment, in case.
    ability->target = target; //may have been updated...
    if (ability)
        return ability->resolve();
    return 0;
}

const char * GenericActivatedAbility::getMenuText()
{
    if (ability)
        return ability->getMenuText();
    return "Error";
}

int GenericActivatedAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (limitPerTurn && counters >= limitPerTurn)
        return 0;
    return ActivatedAbility::isReactingToClick(card, mana);
}

void GenericActivatedAbility::Update(float dt)
{
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_AFTER_EOT)
    {
        counters = 0;
    }
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
AADamager::AADamager(int _id, MTGCardInstance * _source, Targetable * _target, WParsedInt * damage, ManaCost * _cost, int doTap,
        int who) :
    ActivatedAbilityTP(_id, _source, _target, _cost, doTap, who), damage(damage)
{
    aType = MTGAbility::DAMAGER;
}

int AADamager::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        game->mLayers->stackLayer()->addDamage(source, _target, damage->getValue());
        game->mLayers->stackLayer()->resolve();
        return 1;
    }
    return 0;
}

const char * AADamager::getMenuText()
{
    return "Damage";
}

AADamager * AADamager::clone() const
{
    AADamager * a = NEW AADamager(*this);
    a->damage = NEW WParsedInt(*(a->damage));
    a->isClone = 1;
    return a;
}

AADamager::~AADamager()
{
    SAFE_DELETE(damage);
}

//AADepleter
AADepleter::AADepleter(int _id, MTGCardInstance * card, Targetable * _target, int nbcards, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who), nbcards(nbcards)
{
}
int AADepleter::resolve()
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
        for (int i = 0; i < nbcards; i++)
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

//Counters
AACounter::AACounter(int id, MTGCardInstance * source, MTGCardInstance * target, const char * _name, int power, int toughness,
        int nb, ManaCost * cost, int doTap) :
    ActivatedAbility(id, source, cost, 0, doTap), nb(nb), power(power), toughness(toughness), name(_name)
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
        if (nb > 0)
        {
            for (int i = 0; i < nb; i++)
            {
                while (_target->next)
                    _target = _target->next;
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
    if (nb != 1)
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

// Fizzler
AAFizzler::AAFizzler(int _id, MTGCardInstance * card, Spell * _target, ManaCost * _cost, int _tap) :
    ActivatedAbility(_id, card, _cost, 0, _tap)
{
    target = _target;
}

int AAFizzler::resolve()
{
    Spell * _target = (Spell *) target;
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

AADrawer::AADrawer(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, WParsedInt * _nbcards, int _tap,
        int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who), nbcards(_nbcards)
{
    aType = MTGAbility::STANDARD_DRAW;
    nbcardAmount = nbcards->getValue();
}

int AADrawer::resolve()
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
        game->mLayers->stackLayer()->addDraw(player, nbcards->getValue());
        game->mLayers->stackLayer()->resolve();
    }
    return 1;
}

const char * AADrawer::getMenuText()
{
    return "Draw";
}

AADrawer * AADrawer::clone() const
{
    AADrawer * a = NEW AADrawer(*this);
    a->nbcards = NEW WParsedInt(*(a->nbcards));
    a->isClone = 1;
    return a;
}

AADrawer::~AADrawer()
{
    SAFE_DELETE(nbcards);
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

//AALifer
AALifer::AALifer(int _id, MTGCardInstance * card, Targetable * _target, WParsedInt * life, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who), life(life)
{
    aType = MTGAbility::LIFER;
}

int AALifer::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        if (_target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        {
            _target = ((MTGCardInstance *) _target)->controller();
        }
        _target->life += life->getValue();
    }
    return 1;
}

const char * AALifer::getMenuText()
{
    return "Life";
}

AALifer * AALifer::clone() const
{
    AALifer * a = NEW AALifer(*this);
    a->life = NEW WParsedInt(*(a->life));
    a->isClone = 1;
    return a;
}

AALifer::~AALifer()
{
    SAFE_DELETE(life);
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
    if (_target)
    {
        if (_target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        {
            _target = ((MTGCardInstance *) _target)->controller();
        }
        _target->life = life->getValue();
    }
    return 0;
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
        MTGCard* clone = (_target->isToken ? _target : GameApp::collection->getCardById(_target->getId()));

        if (who != 1)
            myClone = NEW MTGCardInstance(clone, source->controller()->game);
        if (who == 1)
            myClone = NEW MTGCardInstance(clone, source->controller()->opponent()->game);
        if (who != 1)
            source->controller()->game->temp->addCard(myClone);
        else
            source->controller()->opponent()->game->temp->addCard(myClone);
        Spell * spell = NEW Spell(myClone);
        spell->resolve();
        spell->source->isToken = 1;
        spell->source->fresh = 1;
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

// More Land - allow more lands to be played on a turn
AAMoreLandPlz::AAMoreLandPlz(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, WParsedInt * _additional,
        int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who), additional(_additional)
{
}

int AAMoreLandPlz::resolve()
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
        player->landsPlayerCanStillPlay += additional->getValue();
    }
    return 1;
}

const char * AAMoreLandPlz::getMenuText()
{
    return "Additional Lands";
}

AAMoreLandPlz * AAMoreLandPlz::clone() const
{
    AAMoreLandPlz * a = NEW AAMoreLandPlz(*this);
    a->additional = NEW WParsedInt(*(a->additional));
    a->isClone = 1;
    return a;
}

AAMoreLandPlz::~AAMoreLandPlz()
{
    SAFE_DELETE(additional);
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

// No Creatures
AANoCreatures::AANoCreatures(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who)
{
}

int AANoCreatures::resolve()
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
        player->nocreatureinstant = true;
    }
    return 1;
}

const char * AANoCreatures::getMenuText()
{
    return "No Creatures!";
}

AANoCreatures * AANoCreatures::clone() const
{
    AANoCreatures * a = NEW AANoCreatures(*this);
    a->isClone = 1;
    return a;
}

// AA No Spells
AANoSpells::AANoSpells(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who)
{
}
int AANoSpells::resolve()
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
        player->nospellinstant = true;
    }
    return 1;
}

const char * AANoSpells::getMenuText()
{
    return "No Spells!";
}

AANoSpells * AANoSpells::clone() const
{
    AANoSpells * a = NEW AANoSpells(*this);
    a->isClone = 1;
    return a;
}

//OnlyOne
AAOnlyOne::AAOnlyOne(int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who)
{
}

int AAOnlyOne::resolve()
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
        player->onlyoneinstant = true;
    }
    return 1;
}

const char * AAOnlyOne::getMenuText()
{
    return "Only One Spell!";
}

AAOnlyOne * AAOnlyOne::clone() const
{
    AAOnlyOne * a = NEW AAOnlyOne(*this);
    a->isClone = 1;
    return a;
}

//Random Discard
AARandomDiscarder::AARandomDiscarder(int _id, MTGCardInstance * card, Targetable * _target, int nbcards, ManaCost * _cost,
        int _tap, int who) :
    ActivatedAbilityTP(_id, card, _target, _cost, _tap, who), nbcards(nbcards)
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
        for (int i = 0; i < nbcards; i++)
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
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (source)
    {
        source->MaxLevelUp = value;
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
    vector<int>::size_type sz = abilities.size();
    for (unsigned int i = 0; i < sz; i++)
    {
        if (abilities[i] == NULL)
            continue;
        Targetable * backup = abilities[i]->target;
        if (target && target != source && abilities[i]->target == abilities[i]->source)
            abilities[i]->target = target;
        abilities[i]->resolve();
        abilities[i]->target = backup;
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
GenericTargetAbility::GenericTargetAbility(int _id, MTGCardInstance * _source, TargetChooser * _tc, MTGAbility * a,
        ManaCost * _cost, int _tap, int limit, int restrictions, MTGGameZone * dest) :
    TargetAbility(_id, _source, _tc, _cost, restrictions, _tap), limitPerTurn(limit), activeZone(dest)
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
    MTGCardInstance * _target = (MTGCardInstance *) target;
}

int AAlterCost::addToGame()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (amount < 0)
    {
        amount = abs(amount);
        if (_target->getManaCost()->hasColor(type))
        {
            if (_target->getManaCost()->getConvertedCost() >= 1)
            {
                _target->getManaCost()->remove(type, amount);
                if (_target->getManaCost()->alternative > 0)
                {
                    _target->getManaCost()->alternative->remove(type, amount);
                }
                if (_target->getManaCost()->BuyBack > 0)
                {
                    _target->getManaCost()->BuyBack->remove(type, amount);
                }
            }
        }
    }
    else
    {
        _target->getManaCost()->add(type, amount);
        if (_target->getManaCost()->alternative > 0)
        {
            _target->getManaCost()->alternative->add(type, amount);
        }
        if (_target->getManaCost()->BuyBack > 0)
        {
            _target->getManaCost()->BuyBack->add(type, amount);
        }
    }
    return MTGAbility::addToGame();
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
ATransformer::ATransformer(int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, string sabilities) :
    MTGAbility(id, source, target)
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    PopulateAbilityIndexVector(abilities, sabilities);
    PopulateColorIndexVector(colors, sabilities);

    remove = false;
    if (stypes == "removesubtypes")
        remove = true;
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
                        || s == "Level" || s == "Levelup" || s == "Mine" || s == "Oasis" || s == "World" || s == "Aura"
                )
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
    MTGCardInstance * _target = (MTGCardInstance *) target;
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
            _target->setColor(0, 1);
        }

        for (it = types.begin(); it != types.end(); it++)
        {
            if (remove == true)
            {
                _target->removeType(*it);
            }
            else
            {
                _target->addType(*it);
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
    }
    return MTGAbility::addToGame();
}

int ATransformer::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next;
        list<int>::iterator it;
        for (it = types.begin(); it != types.end(); it++)
        {
            if (remove == false)
                _target->removeType(*it);
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
        if (remove == true)
        {
            for (it = oldtypes.begin(); it != oldtypes.end(); it++)
            {
                if (!_target->hasSubtype(*it))
                    _target->addType(*it);
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

// AForeverTransformer
AForeverTransformer::AForeverTransformer(int id, MTGCardInstance * source, MTGCardInstance * target, string stypes,
        string sabilities) :
    MTGAbility(id, source, target)
{
    aType = MTGAbility::STANDARD_BECOMES;
    MTGCardInstance * _target = (MTGCardInstance *) target;

    PopulateAbilityIndexVector(abilities, sabilities);
    PopulateColorIndexVector(colors, sabilities);
    PopulateSubtypesIndexVector(types, stypes);
    menu = stypes;
}

int AForeverTransformer::addToGame()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next;
        list<int>::iterator it;
        for (it = colors.begin(); it != colors.end(); it++)
        {
            _target->setColor(0, 1);
        }
        for (it = types.begin(); it != types.end(); it++)
        {
            _target->addType(*it);
        }
        for (it = colors.begin(); it != colors.end(); it++)
        {
            _target->setColor(*it);
        }
        for (it = abilities.begin(); it != abilities.end(); it++)
        {
            _target->basicAbilities[*it]++;
        }
    }
    return MTGAbility::addToGame();
}

const char * AForeverTransformer::getMenuText()
{
    string s = menu;
    sprintf(menuText, "Becomes %s", s.c_str());
    return menuText;
}

AForeverTransformer * AForeverTransformer::clone() const
{
    AForeverTransformer * a = NEW AForeverTransformer(*this);
    a->isClone = 1;
    return a;
}
AForeverTransformer::~AForeverTransformer()
{
}

//ATransformerUEOT
ATransformerUEOT::ATransformerUEOT(int id, MTGCardInstance * source, MTGCardInstance * target, string types, string abilities) :
    InstantAbility(id, source, target)
{
    ability = NEW ATransformer(id, source, target, types, abilities);
    aType = MTGAbility::STANDARD_BECOMES;
}

int ATransformerUEOT::resolve()
{
    ATransformer * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}
const char * ATransformerUEOT::getMenuText()
{
    return ability->getMenuText();
}

ATransformerUEOT * ATransformerUEOT::clone() const
{
    ATransformerUEOT * a = NEW ATransformerUEOT(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

ATransformerUEOT::~ATransformerUEOT()
{
    SAFE_DELETE(ability);
}

// ATransformerFOREVER
ATransformerFOREVER::ATransformerFOREVER(int id, MTGCardInstance * source, MTGCardInstance * target, string types, string abilities) :
    InstantAbility(id, source, target)
{
    ability = NEW AForeverTransformer(id, source, target, types, abilities);
    aType = MTGAbility::STANDARD_BECOMES;
}

int ATransformerFOREVER::resolve()
{
    AForeverTransformer * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}

const char * ATransformerFOREVER::getMenuText()
{
    return ability->getMenuText();
}

ATransformerFOREVER * ATransformerFOREVER::clone() const
{
    ATransformerFOREVER * a = NEW ATransformerFOREVER(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

ATransformerFOREVER::~ATransformerFOREVER()
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

// ABecomes
ABecomes::ABecomes(int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, WParsedPT * wppt, string sabilities) :
    MTGAbility(id, source, target), wppt(wppt)
{

    aType = MTGAbility::STANDARD_BECOMES;

    PopulateAbilityIndexVector(abilities, sabilities);
    PopulateColorIndexVector(colors, sabilities);
    PopulateSubtypesIndexVector(types, stypes);
    menu = stypes;

}
int ABecomes::addToGame()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    list<int>::iterator it;
    for (it = types.begin(); it != types.end(); it++)
    {
        _target->addType(*it);
    }
    for (it = colors.begin(); it != colors.end(); it++)
    {
        _target->setColor(*it);
    }
    for (it = abilities.begin(); it != abilities.end(); it++)
    {
        _target->basicAbilities[*it]++;
    }

    if (wppt)
    {
        _target->power = wppt->power.getValue();
        _target->toughness = wppt->toughness.getValue();
        _target->life = _target->toughness;
    }
    return MTGAbility::addToGame();
}

int ABecomes::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    list<int>::iterator it;
    for (it = types.begin(); it != types.end(); it++)
    {
        _target->removeType(*it);
    }
    for (it = colors.begin(); it != colors.end(); it++)
    {
        _target->removeColor(*it);
    }
    for (it = abilities.begin(); it != abilities.end(); it++)
    {
        _target->basicAbilities[*it]--;
    }
    return 1;
}

const char * ABecomes::getMenuText()
{
    string s = menu;
    sprintf(menuText, "Becomes %s", s.c_str());
    return menuText;
}

ABecomes * ABecomes::clone() const
{
    ABecomes * a = NEW ABecomes(*this);
    if (a->wppt)
        a->wppt = NEW WParsedPT(*(a->wppt));
    a->isClone = 1;
    return a;
}

ABecomes::~ABecomes()
{
    SAFE_DELETE (wppt);
}

//  ABecomes

// ABecomesUEOT
ABecomesUEOT::ABecomesUEOT(int id, MTGCardInstance * source, MTGCardInstance * target, string types, WParsedPT * wpt,
        string abilities) :
    InstantAbility(id, source, target)
{
    ability = NEW ABecomes(id, source, target, types, wpt, abilities);
    aType = MTGAbility::STANDARD_BECOMES;
}

int ABecomesUEOT::resolve()
{
    ABecomes * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), a);
    wrapper->addToGame();
    return 1;
}

const char * ABecomesUEOT::getMenuText()
{
    return ability->getMenuText();
}

ABecomesUEOT * ABecomesUEOT::clone() const
{
    ABecomesUEOT * a = NEW ABecomesUEOT(*this);
    a->ability = this->ability->clone();
    a->isClone = 1;
    return a;
}

ABecomesUEOT::~ABecomesUEOT()
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

//AUpkeep
AUpkeep::AUpkeep(int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int _tap, int restrictions, int _phase,
        int _once) :
    ActivatedAbility(_id, card, _cost, restrictions, _tap), NestedAbility(a), phase(_phase), once(_once)
{
    paidThisTurn = 0;
    aType = MTGAbility::UPCOST;
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
        else if (newPhase == phase + 1 && !paidThisTurn)
        {
            ability->resolve();
        }
        if (newPhase == phase + 1 && once)
            once = 2;
    }
    ActivatedAbility::Update(dt);
}

int AUpkeep::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    if (currentPhase != phase || paidThisTurn || once >= 2)
        return 0;
    return ActivatedAbility::isReactingToClick(card, mana);
}

int AUpkeep::resolve()
{
    paidThisTurn = 1;
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
            if ((Constants::GetBasicAbilityIndex(*iter) != -1)
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
