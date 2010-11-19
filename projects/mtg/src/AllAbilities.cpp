#include "PrecompiledHeader.h"
#include "AllAbilities.h"

// BanishCard implementations

AABanishCard::AABanishCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL,
        int _banishmentType = -1) :
    ActivatedAbility(_id, _source, _cost), banishmentType(_banishmentType)
{
    if (_target) target = _target;
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

AABuryCard::AABuryCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL, int _banishmentType =
        0) :
    AABanishCard(_id, _source, _target, _cost, AABanishCard::BURY)
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

AADestroyCard::AADestroyCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL,
        int _banishmentType = 0) :
    AABanishCard(_id, _source, _target, _cost, AABanishCard::DESTROY)
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
AASacrificeCard::AASacrificeCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL,
        int _banishmentType = 0) :
    AABanishCard(_id, _source, _target, _cost, AABanishCard::SACRIFICE)
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

AADiscardCard::AADiscardCard(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost = NULL,
        int _banishmentType = 0) :
    AABanishCard(_id, _source, _target, _cost, AABanishCard::DISCARD)
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

//Mana Redux
AManaRedux::AManaRedux(int id, MTGCardInstance * source, MTGCardInstance * target, int amount, int type) :
    MTGAbility(id, source, target), amount(amount), type(type)
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
}

int AManaRedux::addToGame()
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

AManaRedux * AManaRedux::clone() const
{
    AManaRedux * a = NEW AManaRedux(*this);
    a->isClone = 1;
    return a;
}

AManaRedux::~AManaRedux()
{
}

// ABecomes
ABecomes::ABecomes(int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, WParsedPT * wppt, string sabilities) :
    MTGAbility(id, source, target), wppt(wppt)
{

    aType = MTGAbility::STANDARD_BECOMES;

    for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++)
    {
        size_t found = sabilities.find(Constants::MTGBasicAbilities[j]);
        if (found != string::npos)
        {
            abilities.push_back(j);
        }
    }

    for (int j = 0; j < Constants::MTG_NB_COLORS; j++)
    {
        size_t found = sabilities.find(Constants::MTGColorStrings[j]);
        if (found != string::npos)
        {
            colors.push_back(j);
        }
    }

    string s = stypes;
    menu = stypes;
    while (s.size())
    {
        size_t found = s.find(" ");
        if (found != string::npos)
        {
            int id = Subtypes::subtypesList->find(s.substr(0, found));
            types.push_back(id);
            s = s.substr(found + 1);
        }
        else
        {
            int id = Subtypes::subtypesList->find(s);
            types.push_back(id);
            s = "";
        }
    }
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
    if (a->wppt) a->wppt = NEW WParsedPT(*(a->wppt));
    a->isClone = 1;
    return a;
}

ABecomes::~ABecomes()
{
    SAFE_DELETE (wppt);
}

//  ABecomes

// ABecomesUEOT
ABecomesUEOT::ABecomesUEOT(int id, MTGCardInstance * source, MTGCardInstance * target, string types, WParsedPT * wpt, string abilities) :
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
    SAFE_DELETE ability;
}
