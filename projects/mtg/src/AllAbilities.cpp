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

// ATransformer
ATransformer::ATransformer(int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, string sabilities) :
    MTGAbility(id, source, target)
{
    //TODO this is a copy/past of other code that's all around the place, everything should be in a dedicated parser class;
    MTGCardInstance * _target = (MTGCardInstance *) target;
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
    remove = false;
    if (stypes == "removesubtypes") remove = true;
    if (stypes == "allsubtypes" || stypes == "removesubtypes")
    {
        for (int i = Subtypes::LAST_TYPE + 1;; i++)
        {
            string s = Subtypes::subtypesList->find(i);
            {
                if (s == "") break;
                if (s.find(" ") != string::npos) continue;
                if (s == "Nothing" || s == "Swamp" || s == "Plains" || s == "Mountain" || s == "Forest" || s == "Island" || s
                        == "Shrine" || s == "Basic" || s == "Colony" || s == "Desert" || s == "Dismiss" || s == "Equipment" || s
                        == "Everglades" || s == "Grasslands" || s == "Lair" || s == "Level" || s == "Levelup" || s == "Mine" || s
                        == "Oasis" || s == "World" || s == "Aura")
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
        string s = stypes;
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
            if (_target->hasColor(j)) oldcolors.push_back(j);
        }
        for (int j = Subtypes::LAST_TYPE + 1;; j++)
        {
            string otypes = Subtypes::subtypesList->find(j);
            if (otypes == "") break;
            if (otypes.find(" ") != string::npos) continue;
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
            if (remove == false) _target->removeType(*it);
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
                if (!_target->hasSubtype(*it)) _target->addType(*it);
            }
        }
    }
    return 1;
}

const char * ATransformer::getMenuText()
{
    return "Transform";
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
    //TODO this is a copy/past of other code that's all around the place, everything should be in a dedicated parser class;
    MTGCardInstance * _target = (MTGCardInstance *) target;
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
int addToGame()
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
    return "Transform";
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
    return "Transform";
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
    return "Transform";
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
    SAFE_DELETE ability;
}
