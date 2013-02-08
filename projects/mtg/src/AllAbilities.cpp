#include "PrecompiledHeader.h"
#include "AllAbilities.h"
#include "Translate.h"
#include <boost/algorithm/string.hpp>


//display a text animation, this is not a real ability.
MTGEventText::MTGEventText(GameObserver* observer, int _id, MTGCardInstance * card, string textToShow) :
MTGAbility(observer, _id,card)
{
    textAlpha = 255;
    text = textToShow;
}

void MTGEventText::Update(float dt)
{
    if (textAlpha)
    {
        textAlpha -= static_cast<int> (200 * dt);
        Render();
        if (textAlpha < 0)
        {
            textAlpha = 0;
            this->forceDestroy = 1;
        }
    }
    MTGAbility::Update(dt);
}

void MTGEventText::Render()
{
    if (!textAlpha)
        return;
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::OPTION_FONT);
    float backup = mFont->GetScale();
    mFont->SetScale(2 - (float) textAlpha / 130);
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(text.c_str(), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, JGETEXT_CENTER);
    mFont->SetScale(backup);
}

MTGEventText * MTGEventText::clone() const
{
    return NEW MTGEventText(*this);
}

////////////////////////
//Activated Abilities

//Generic Activated Abilities
GenericActivatedAbility::GenericActivatedAbility(GameObserver* observer, string newName, string castRestriction, int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost,
        string limit,MTGAbility * sideEffects,string usesBeforeSideEffects, int restrictions, MTGGameZone * dest) :
    ActivatedAbility(observer, _id, card, _cost, restrictions,limit,sideEffects,usesBeforeSideEffects,castRestriction), NestedAbility(a), activeZone(dest),newName(newName)
{
    counters = 0;
    target = ability->target;
}

int GenericActivatedAbility::resolve()
{
    //Note: I've seen a similar block in some other MTGAbility, can this be refactored .
    if (abilityCost)
    {
        source->X = 0;
        ManaCost * diff = abilityCost->Diff(getCost());
        source->X = diff->hasX();
        SAFE_DELETE(diff);
    }
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
    a->ability = ability->clone();
    return a;
}

GenericActivatedAbility::~GenericActivatedAbility()
{
    SAFE_DELETE(ability);
}

//AA Alter Poison
AAAlterPoison::AAAlterPoison(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int poison, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), poison(poison)
{
}

int AAAlterPoison::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player * pTarget = (Player*)_target;
        if(!pTarget->inPlay()->hasAbility(Constants::POISONSHROUD) || poison < 0)
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
    return NEW AAAlterPoison(*this);
}

AAAlterPoison::~AAAlterPoison()
{
}

//Damage Prevent
AADamagePrevent::AADamagePrevent(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int preventing, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), preventing(preventing)
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
    return NEW AADamagePrevent(*this);
}

AADamagePrevent::~AADamagePrevent()
{
}

//AADamager
AADamager::AADamager(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, string d, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), d(d)
{
    aType = MTGAbility::DAMAGER;
    redirected = false;
    }

    int AADamager::resolve()
    {
        Damageable * _target = (Damageable *) getTarget();
        if (_target)
        {
            WParsedInt damage(d, NULL, (MTGCardInstance *)source);
            if(_target == game->opponent() && game->opponent()->inPlay()->hasType("planeswalker") && !redirected)
            {
                vector<MTGAbility*>selection;
                MTGCardInstance * check = NULL;
                this->redirected = true;
                MTGAbility * setPlayer = this->clone();
                this->redirected = false;
                selection.push_back(setPlayer);
                int checkWalkers = ((Player*)_target)->game->battlefield->cards.size();
                for(int i = 0; i < checkWalkers;++i)
                {
                    check = ((Player*)_target)->game->battlefield->cards[i];
                    if(check->hasType(Subtypes::TYPE_PLANESWALKER))
                    {
                        this->redirected = true;
                        MTGAbility * setWalker = this->clone();
                        this->redirected = false;
                        setWalker->oneShot = true;
                        setWalker->target = check;
                        selection.push_back(setWalker);
                    }
                }
                if(selection.size())
                {
                    MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), source, source,true,selection);
                    game->mLayers->actionLayer()->currentActionCard = source;
                    a1->resolve();
                }
                return 1;
            }
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
        MTGCardInstance * _target = dynamic_cast<MTGCardInstance*>(target);
        if(_target && _target->hasType(Subtypes::TYPE_PLANESWALKER))
            return _target->name.c_str();
        if(redirected)
            return "Damage Player";
        return "Damage";
    }

AADamager * AADamager::clone() const
{
    return NEW AADamager(*this);
}


//AADepleter
AADepleter::AADepleter(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who),nbcardsStr(nbcardsStr)
{

}
    int AADepleter::resolve()
    {
        Player * player = getPlayerFromTarget(getTarget());
        
        if (player)
        {
            WParsedInt numCards(nbcardsStr, NULL, source);
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
    return NEW AADepleter(*this);
}

//take extra turns or skip turns, values in the negitive will make you skip.
AAModTurn::AAModTurn(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbTurnStr, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who),nbTurnStr(nbTurnStr)
{

}
    int AAModTurn::resolve()
    {
        Player * player = getPlayerFromTarget(getTarget());
        if (player)
        {
            WParsedInt numTurns(nbTurnStr, NULL, source);
            if(numTurns.getValue() > 0)
            {
                player->extraTurn += numTurns.getValue();
            }
            else
            {
                player->skippingTurn += abs(numTurns.getValue());

            }
        }
        return 1;
    }

    const char * AAModTurn::getMenuText()
    {
        WParsedInt numTurns(nbTurnStr, NULL, source);
        if(numTurns.getValue() > 0)
            return "Take Extra Turn(s)";
        else
            return "Skip A Turn(s)";
    }

AAModTurn * AAModTurn::clone() const
{
    return NEW AAModTurn(*this);
}

//move target to bottom of owners library
AALibraryBottom::AALibraryBottom(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
}

int AALibraryBottom::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    _target = _target->owner->game->putInLibrary(_target);
    if (_target)
    {
        MTGLibrary * library = _target->owner->game->library;
        vector<MTGCardInstance *>oldOrder = library->cards;
        vector<MTGCardInstance *>newOrder;
        newOrder.push_back(_target);
        for(unsigned int k = 0;k < oldOrder.size();++k)
        {
            MTGCardInstance * rearranged = oldOrder[k];
            if(rearranged != _target)
                newOrder.push_back(rearranged);
        }
        library->cards = newOrder;
        return 1;
    }
    return 0;
}

const char * AALibraryBottom::getMenuText()
{
    return "Bottom Of Library";
}

AALibraryBottom * AALibraryBottom::clone() const
{
    return NEW AALibraryBottom(*this);
}

//AACopier
AACopier::AACopier(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
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
    return NEW AACopier(*this);
}

//phaser
AAPhaseOut::AAPhaseOut(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0)
{
    target = _target;
}

int AAPhaseOut::resolve()
{
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
    return NEW AAPhaseOut(*this);
}

//Counters
AACounter::AACounter(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target,string counterstring, const char * _name, int power, int toughness,
        int nb,int maxNb, ManaCost * cost) :
    ActivatedAbility(observer, id, source, cost, 0),counterstring(counterstring), nb(nb),maxNb(maxNb), power(power), toughness(toughness), name(_name)
{
    this->target = target;
    if (name.find("Level") != string::npos || name.find("level") != string::npos)
        aType = MTGAbility::STANDARD_LEVELUP;
    else
        aType = MTGAbility::COUNTERS;
        
    menu = "";
}

    int AACounter::resolve()
    {
        if (target)
        {
            MTGCardInstance * _target = (MTGCardInstance *) target;
            AbilityFactory af(game);
            if(counterstring.size())
            {
                Counter * checkcounter = af.parseCounter(counterstring, source, NULL);
                nb = checkcounter->nb;
                delete checkcounter;
            }
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
                    {
                        _target->counters->addCounter(name.c_str(), power, toughness);
                    }
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

            _target->doDamageTest = 1;
            if(!_target->afterDamage())
            {
                //If a creature with +1/+1 counters on it gets enough -1/-1 counters to kill it, 
                //it dies before the two counters have the chance to cancel out. For example, 
                //if your Strangleroot Geist with a +1/+1 counter on it got three -1/-1 counters 
                //from Skinrender's "enters the battlefield" ability, the Geist would die with //
                //one +1/+1 counter and three -1/-1 counters and wouldn't return to the battlefield.
                for (int i = 0; i < _target->counters->mCount; i++)
                {
                    if (_target->counters->counters[i]->cancels(power, toughness) && !name.size() && _target->counters->counters[i]->nb > 0)
                    {
                        _target->counters->counters[i]->cancelCounter(power,toughness);
                    }
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
    return NEW AACounter(*this);
}

//shield a card from a certain type of counter.
ACounterShroud::ACounterShroud(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target,TargetChooser * tc, Counter * counter) :
MTGAbility(observer, id, source),csTc(tc),counter(counter),re(NULL)
{
}

int ACounterShroud::addToGame()
{
    SAFE_DELETE(re);
    re = NEW RECountersPrevention(this,source,(MTGCardInstance*)target,csTc,counter);
    if (re)
    {
        game->replacementEffects->add(re);
        return MTGAbility::addToGame();
    }
    return 0;
}

int ACounterShroud::destroy()
{
    game->replacementEffects->remove(re);
    SAFE_DELETE(re);
    return 1;
}

ACounterShroud * ACounterShroud::clone() const
{
    ACounterShroud * a = NEW ACounterShroud(*this);
    a->re = NULL;
    return a;
}

ACounterShroud::~ACounterShroud()
{
    SAFE_DELETE(re);
    SAFE_DELETE(counter);
}

//track counters placed on a card
ACounterTracker::ACounterTracker(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string scounter) :
MTGAbility(observer, id, source, target),scounter(scounter)
{
    removed = 0;
}

int ACounterTracker::addToGame()
{
    MTGCardInstance * _target = (MTGCardInstance*)target;
    AbilityFactory af(game);
    Counter * counter = af.parseCounter(scounter, _target, NULL); //(Spell*)source);
    if (!counter)
    {
        return 0;
    }
    if(_target && !removed)
    {
        if(_target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness) && _target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness)->nb >= counter->nb)
        {
            for(int nb = 0;nb < counter->nb;nb++)
            {
                _target->counters->removeCounter(counter->name.c_str(),counter->power,counter->toughness);
                removed++;
            }
        }
        SAFE_DELETE(counter);
        return MTGAbility::addToGame();
    }
    SAFE_DELETE(counter);
    return 0;
}

int ACounterTracker::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance*)target;
    AbilityFactory af(game);
    Counter * counter = af.parseCounter(scounter, _target, NULL); //(Spell*)source);
    if (!counter)
    {
        return 0;
    }
    if(_target)
    {
        if(removed == counter->nb)
        {
            for(int nb = 0;nb < counter->nb;nb++)
            {
                _target->counters->addCounter(counter->name.c_str(),counter->power,counter->toughness);
            }
        }
    }
    SAFE_DELETE(counter);
    return 1;
}

int ACounterTracker::testDestroy()
{
    if(this->source->isInPlay(game))
        return 0;
    return 1;
}

ACounterTracker * ACounterTracker::clone() const
{
    ACounterTracker * a = NEW ACounterTracker(*this);
    return a;
}

ACounterTracker::~ACounterTracker()
{
}

//removeall counters of a certain type or all.
AARemoveAllCounter::AARemoveAllCounter(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, const char * _name, int power, int toughness,
        int nb,bool all, ManaCost * cost) :
    ActivatedAbility(observer, id, source, cost, 0), nb(nb), power(power), toughness(toughness), name(_name),all(all)
{
    this->target = target;
    menu = "";
}

int AARemoveAllCounter::resolve()
{
    if (!target)
        return 0;

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


    for (int i = 0; i < nb; i++)
    {
        while (_target->next)
            _target = _target->next;
        _target->counters->removeCounter(name.c_str(), power, toughness);
    }

    return nb;
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

    return menu.c_str();
}

AARemoveAllCounter * AARemoveAllCounter::clone() const
{
    return NEW AARemoveAllCounter(*this);
}

//proliferate a target
AAProliferate::AAProliferate(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target,ManaCost * cost) :
ActivatedAbility(observer, id, source, cost, 0)
{
    this->GetId();
}
 
int AAProliferate::resolve()
{
    if (!target)
        return 0;

    vector<MTGAbility*>pcounters;
    
    Player * pTarget = dynamic_cast<Player *>(target);
    MTGCardInstance * cTarget = dynamic_cast<MTGCardInstance *>(target);

    if(pTarget && pTarget->poisonCount && pTarget != source->controller())
    {
        MTGAbility * a = NEW AAAlterPoison(game, game->mLayers->actionLayer()->getMaxId(), source, target, 1, NULL);
        a->oneShot = true;
        pcounters.push_back(a);
    }
    else if (cTarget && cTarget->counters)
    {
        Counters * counters = cTarget->counters;
        for(size_t i = 0; i < counters->counters.size(); ++i)
        {
            Counter * counter = counters->counters[i];
            MTGAbility * a = NEW AACounter(game, game->mLayers->actionLayer()->getMaxId(), source, cTarget,"", counter->name.c_str(), counter->power, counter->toughness, 1,0);
            a->oneShot = true;
            pcounters.push_back(a);
        }
    }
    if(pcounters.size())
    {
        MTGAbility * a = NEW MenuAbility(game, this->GetId(), target, source,false,pcounters);
        a->resolve();
    }
    return 1;

}

const char* AAProliferate::getMenuText()
{
    return "Proliferate";
}

AAProliferate * AAProliferate::clone() const
{
    return NEW AAProliferate(*this);
}

AAProliferate::~AAProliferate()
{
}
//
//choosing a type or color
GenericChooseTypeColor::GenericChooseTypeColor(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target,string _toAdd,bool chooseColor,bool nonwall, ManaCost * cost) :
ActivatedAbility(observer, id, source, cost, 0), baseAbility(_toAdd),chooseColor(chooseColor),ANonWall(nonwall)
{
    this->GetId();
    setColor = NULL;
}
int GenericChooseTypeColor::resolve()
{
    if (!target)
        return 0;
    vector<MTGAbility*>selection;
    if(chooseColor)
    {
        for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i)
        {
            setColor = NEW AASetColorChosen(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, i, baseAbility);
            MTGAbility * set = setColor->clone();
            set->oneShot = true;
            selection.push_back(set);
            SAFE_DELETE(setColor);
        }
    }
    else
    {
        vector<string> values = MTGAllCards::getCreatureValuesById();
        for (size_t i = 0; i < values.size(); ++i)
        {
            string menu = values[i];
            if(!ANonWall || (menu != "wall" && menu != "Wall"))
            {
            setType = NEW AASetTypeChosen(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, i,menu,baseAbility);
            MTGAbility * set = setType->clone();
            set->oneShot = true;
            selection.push_back(set);
            SAFE_DELETE(setType);
            }
        }
    }

    if(selection.size())
    {
        MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), target, source,true,selection);
        game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
        a1->resolve();
    }
    return 1;

}

const char* GenericChooseTypeColor::getMenuText()
{
    if(chooseColor)
        return "Choose a color";
    else
        return "Choose a type";
}

GenericChooseTypeColor * GenericChooseTypeColor::clone() const
{
    GenericChooseTypeColor * a = NEW GenericChooseTypeColor(*this);
    return a;
}

GenericChooseTypeColor::~GenericChooseTypeColor()
{
}

//set color choosen
 AASetColorChosen::AASetColorChosen(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target,int _color , string toAlter):
    InstantAbility(observer, id, source),color(_color), abilityToAlter(toAlter)
{
    this->target = _target;
    abilityAltered = NULL;
}
int AASetColorChosen::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    _target->chooseacolor = color;

    if(abilityToAlter.size())
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityToAlter, 0, NULL, _target);
        if(!abilityAltered)
            return 0;
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->target = _target;
            MayAbility * dontAdd = dynamic_cast<MayAbility*>(abilityAltered);
            if (!dontAdd)
            {
                _target->cardsAbilities.push_back(abilityAltered);
                for(unsigned int j = 0;j < _target->cardsAbilities.size();++j)
                {
                    if(_target->cardsAbilities[j] == this)
                        _target->cardsAbilities.erase(_target->cardsAbilities.begin() + j);
                }
            }
            abilityAltered->addToGame();
        }
        _target->skipDamageTestOnce = true;//some cards rely on this ability updating before damage test are run. otherwise they die before toughnes bonus applies.
    }
    return 1;
}

const char* AASetColorChosen::getMenuText()
{
    return Constants::MTGColorStrings[color];
}

AASetColorChosen * AASetColorChosen::clone() const
{
    return NEW AASetColorChosen(*this);
}

AASetColorChosen::~AASetColorChosen()
{
}

//set type choosen
 AASetTypeChosen::AASetTypeChosen(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target,int _type ,string _menu,string toAlter):
    InstantAbility(observer, id, source),type(_type), abilityToAlter(toAlter), menutext(_menu)
{
    this->target = _target;
    abilityAltered = NULL;
}
int AASetTypeChosen::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    string typeChoosen = menutext;
    _target->chooseasubtype = typeChoosen;

    if(abilityToAlter.size())
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityToAlter, 0, NULL, _target);
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->target = _target;
            MayAbility * dontAdd = dynamic_cast<MayAbility*>(abilityAltered);
            if (!dontAdd)
            {
                _target->cardsAbilities.push_back(abilityAltered);
                for(unsigned int j = 0;j < _target->cardsAbilities.size();++j)
                {
                    if(_target->cardsAbilities[j] == this)
                        _target->cardsAbilities.erase(_target->cardsAbilities.begin() + j);
                }
            }

            abilityAltered->addToGame();
        }
         _target->skipDamageTestOnce = true;//some cards rely on this ability updating before damage test are run. otherwise they die before toughnes bonus applies.
    }
    return 1;
}

const char* AASetTypeChosen::getMenuText()
{
    return menutext.c_str();
}

AASetTypeChosen * AASetTypeChosen::clone() const
{
    return NEW AASetTypeChosen(*this);
}

AASetTypeChosen::~AASetTypeChosen()
{
}

//
//choosing a type or color
GenericFlipACoin::GenericFlipACoin(GameObserver* observer, int id, MTGCardInstance * source, Targetable * target,string _toAdd, ManaCost * cost) :
ActivatedAbility(observer, id, source, cost, 0), baseAbility(_toAdd),chooseColor(chooseColor)
{
    this->GetId();
    setCoin = NULL;
}

int GenericFlipACoin::resolve()
{
    if (!target)
        return 0;
    vector<MTGAbility*>selection;
    for (int i = 0; i <2; ++i)
    {
        setCoin = NEW AASetCoin(game, game->mLayers->actionLayer()->getMaxId(), source,(MTGCardInstance*)target, i, baseAbility);
        MTGAbility * set = setCoin->clone();
        set->oneShot = true;
        selection.push_back(set);
        SAFE_DELETE(setCoin);
    }

    if(selection.size())
    {
        MTGAbility * a1 = NEW MenuAbility(game, this->GetId(), target, source,false,selection);
        game->mLayers->actionLayer()->currentActionCard = (MTGCardInstance *)target;
        a1->resolve();
    }
    return 1;

}

const char* GenericFlipACoin::getMenuText()
{
return "Flip A Coin";
}

GenericFlipACoin * GenericFlipACoin::clone() const
{
    GenericFlipACoin * a = NEW GenericFlipACoin(*this);
    return a;
}

GenericFlipACoin::~GenericFlipACoin()
{
}

//set color choosen
 AASetCoin::AASetCoin(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target,int _side , string toAlter):
    InstantAbility(observer, id, source),side(_side), abilityToAlter(toAlter)
{
    this->target = _target;
    abilityAltered = NULL;
}

int AASetCoin::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    _target->coinSide = side;

    int flip = game->getRandomGenerator()->random() % 2;
    vector<string>Win = parseBetween(abilityToAlter,"winability "," winabilityend");
    if(Win.size())
    {
        abilityWin = Win[1];
    }
    vector<string>Lose = parseBetween(abilityToAlter,"loseability "," loseabilityend");
    if(Lose.size())
    {
        abilityLose = Lose[1];
    }

    if(abilityWin.size() && flip == side)
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityWin, 0, NULL, _target);
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->addToGame();
        }
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Won The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityWin.size() && !abilityLose.size())
    {
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Lost The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityLose.size() && flip != side)
    {
        AbilityFactory af(game);
        abilityAltered = af.parseMagicLine(abilityLose, 0, NULL, _target);
        abilityAltered->canBeInterrupted = false;
        if(abilityAltered->oneShot)
        {
            abilityAltered->resolve();
            SAFE_DELETE(abilityAltered);
        }
        else
        {
            abilityAltered->addToGame();
        }
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Lost The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    else if(abilityLose.size())
    {
        MTGAbility * message = NEW MTGEventText(game,this->GetId(), source, "You Won The Flip");
        message->oneShot = true;
        message->addToGame();
    }
    _target->skipDamageTestOnce = true;
    return 1;
}

const char* AASetCoin::getMenuText()
{
    if(side == 1)
        return "Tails";
    return "Heads";
}

AASetCoin * AASetCoin::clone() const
{
    return NEW AASetCoin(*this);
}

AASetCoin::~AASetCoin()
{
}


//replace drawing a card with activation of an ability
ADrawReplacer::ADrawReplacer(GameObserver* observer, int id, MTGCardInstance * source, MTGAbility * replace, bool otherPlayer) :
MTGAbility(observer, id, source),re(NULL),replacer(replace),OtherPlayer(otherPlayer)
{
}

int ADrawReplacer::addToGame()
{
    SAFE_DELETE(re);
    if(OtherPlayer)
        re = NEW REDrawReplacement(this,source->controller()->opponent(),replacer);
    else
        re = NEW REDrawReplacement(this,source->controller(),replacer);
    if (re)
    {
        game->replacementEffects->add(re);
        return MTGAbility::addToGame();
    }
    return 0;
}

int ADrawReplacer::destroy()
{
    game->replacementEffects->remove(re);
    SAFE_DELETE(re);
    return 1;
}

ADrawReplacer * ADrawReplacer::clone() const
{
    ADrawReplacer * a = NEW ADrawReplacer(*this);
    a->re = NULL;
    return a;
}

ADrawReplacer::~ADrawReplacer()
{
    SAFE_DELETE(re);
    SAFE_DELETE(replacer);
}
//Reset Damage on creatures
 AAResetDamage::AAResetDamage(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, ManaCost * cost):
    ActivatedAbility(observer, id, source, cost, 0)
{
    this->target = _target;
}
int AAResetDamage::resolve()
{
    MTGCardInstance * _target =  (MTGCardInstance *)target; 
    _target->life = _target->toughness;
    return 1;
}

const char* AAResetDamage::getMenuText()
{
    return "Reset Damages";
}

AAResetDamage * AAResetDamage::clone() const
{
    return NEW AAResetDamage(*this);
}

//ability that resolves to do nothing.
 AAFakeAbility::AAFakeAbility(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, ManaCost * cost):
    ActivatedAbility(observer, id, source, cost, 0)
{
    this->target = _target;
}
int AAFakeAbility::resolve()
{
    return 1;
}

const char* AAFakeAbility::getMenuText()
{
    return "Ability";
}

AAFakeAbility * AAFakeAbility::clone() const
{
    return NEW AAFakeAbility(*this);
}

// Fizzler
AAFizzler::AAFizzler(GameObserver* observer, int _id, MTGCardInstance * card, Spell * _target, ManaCost * _cost) :
ActivatedAbility(observer, _id, card, _cost, 0)
{
	aType = MTGAbility::STANDARD_FIZZLER;
    target = _target;
}

int AAFizzler::resolve()
{
	ActionStack * stack = game->mLayers->stackLayer();
	//the next section helps Ai correctly recieve its targets for this effect
	if(!target && source->target)
	{
		//ai is casting a spell from its hand to fizzle.
		target = stack->getActionElementFromCard(source->target);
	}
	else if(MTGCardInstance * cTarget = dynamic_cast<MTGCardInstance *>(target))
	{
		//ai targeted using an ability on a card to fizzle.
		target = stack->getActionElementFromCard(cTarget);
	}
	Spell * sTarget = (Spell *) target;
	MTGCardInstance* sCard = sTarget->source;
	if(!sCard || !sTarget || sCard->has(Constants::NOFIZZLE))
		return 0;
	stack->Fizzle(sTarget);
	return 1;
}

const char * AAFizzler::getMenuText()
{
    return "Fizzle";
}

AAFizzler* AAFizzler::clone() const
{
    return NEW AAFizzler(*this);
}
// BanishCard implementations
// Bury

AABuryCard::AABuryCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
}

int AABuryCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        _target->bury();
        while(_target->next)
            _target = _target->next;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
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
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}

AABuryCard::~AABuryCard()
{
    SAFE_DELETE(andAbility);
}

// Destroy

AADestroyCard::AADestroyCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
}

int AADestroyCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        _target->destroy();
        while(_target->next)
            _target = _target->next;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
        return 1;
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
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;

}
AADestroyCard::~AADestroyCard()
{
    SAFE_DELETE(andAbility);
}
// Sacrifice
AASacrificeCard::AASacrificeCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
}

int AASacrificeCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        Player * p = _target->controller();
        MTGCardInstance * beforeCard = _target;
        p->game->putInGraveyard(_target);
        while(_target->next)
            _target = _target->next;
        WEvent * e = NEW WEventCardSacrifice(beforeCard,_target);
        game->receiveEvent(e);
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
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
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}
AASacrificeCard::~AASacrificeCard()
{
    SAFE_DELETE(andAbility);
}
// Discard 

AADiscardCard::AADiscardCard(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target) :
   ActivatedAbility(observer, _id, _source)
{
    target = _target;
    andAbility = NULL;
}

int AADiscardCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        Player * p = _target->controller();
        WEvent * e = NEW WEventCardDiscard(_target);
        game->receiveEvent(e);
        p->game->putInGraveyard(_target);
        while(_target->next)
            _target = _target->next;
        if(andAbility)
        {
            MTGAbility * andAbilityClone = andAbility->clone();
            andAbilityClone->target = _target;
            if(andAbility->oneShot)
            {
                andAbilityClone->resolve();
                SAFE_DELETE(andAbilityClone);
            }
            else
            {
                andAbilityClone->addToGame();
            }
        }
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
    if(andAbility)
        a->andAbility = andAbility->clone();
    return a;
}
AADiscardCard::~AADiscardCard()
{
    SAFE_DELETE(andAbility);
}
AADrawer::AADrawer(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, string nbcardsStr,
        int who, bool noreplace) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who), nbcardsStr(nbcardsStr),noReplace(noreplace)
{
    aType = MTGAbility::STANDARD_DRAW;
}

    int AADrawer::resolve()
    {
        Player * player = getPlayerFromTarget(getTarget());

        if (player)
        {
            WParsedInt numCards(nbcardsStr, NULL, source);
            WEvent * e = NEW WEventDraw(player, numCards.getValue(),this);
            if(!noReplace)
            e = game->replacementEffects->replace(e);
            if(e)
            {
                game->mLayers->stackLayer()->addDraw(player, numCards.getValue());
                game->mLayers->stackLayer()->resolve();
                for(int i = numCards.getValue(); i > 0;i--)
                {
                    WEvent * e = NEW WEventcardDraw(player, 1);
                    game->receiveEvent(e);
                }
            }
            SAFE_DELETE(e);
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
    return NEW AADrawer(*this);
}

// AAFrozen: Prevent a card from untapping during next untap phase
AAFrozen::AAFrozen(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
ActivatedAbility(observer, id, card, _cost, 0)
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
    return NEW AAFrozen(*this);
}

// chose a new target for an aura or enchantment and equip it note: VERY basic right now.
AANewTarget::AANewTarget(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target,bool retarget, ManaCost * _cost) :
ActivatedAbility(observer, id, card, _cost, 0),retarget(retarget)
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
        Spell * reUp = NEW Spell(game, refreshed);
        if(reUp->source->hasSubtype(Subtypes::TYPE_AURA))
        {
            reUp->source->target = source;
            reUp->resolve();
        }
        if(_target->hasSubtype(Subtypes::TYPE_EQUIPMENT))
        {
            reUp->resolve();
            for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
            {
                MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
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
    a->oneShot = 1;
    return a;
}
// morph a card
AAMorph::AAMorph(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
ActivatedAbility(observer, id, card, _cost, restrictions)
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

        AbilityFactory af(game);
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
                game->removeObserver(a);
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
                    MayAbility * dontAdd = dynamic_cast<MayAbility*>(a);
                    if(!dontAdd)
                    {
                        _target->cardsAbilities.push_back(a);
                    }
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
            game->removeObserver(this);
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
    a->forceDestroy = 1;
    return a;
}
// flip a card
AAFlip::AAFlip(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target,string flipStats) :
InstantAbility(observer, id, card, _target),flipStats(flipStats)
{
    target = _target;
}

int AAFlip::resolve()
{
    MTGCardInstance * Flipper = (MTGCardInstance*)source;
    this->oneShot = true;
    if(Flipper->isFlipped)
    {
        game->removeObserver(this);
        return 0;
    }
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next; 

        AbilityFactory af(game);
        _target->isFlipped = true;
        GameObserver * game = _target->getObserver();
        if(flipStats.size())
        {
            MTGCard * fcard = MTGCollection()->getCardByName(flipStats);
            if(!fcard) return 0;
            MTGCardInstance * myFlip = NEW MTGCardInstance(fcard, _target->controller()->game);
            _target->name = myFlip->name;
            _target->setName(myFlip->name);
            _target->colors = myFlip->colors;
            _target->types = myFlip->types;
            _target->text = myFlip->text;
            _target->formattedText = myFlip->formattedText;
            _target->basicAbilities = myFlip->basicAbilities;

            for(unsigned int i = 0;i < _target->cardsAbilities.size();i++)
            {
                MTGAbility * a = dynamic_cast<MTGAbility *>(_target->cardsAbilities[i]);

                if(a) game->removeObserver(a);
            }
            _target->cardsAbilities.clear();
            _target->magicText = myFlip->magicText;
            af.getAbilities(&currentAbilities, NULL, _target);
            for (size_t i = 0; i < currentAbilities.size(); ++i)
            {
                MTGAbility * a = currentAbilities[i];
                a->source = (MTGCardInstance *) _target;
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
                        MayAbility * dontAdd = dynamic_cast<MayAbility*>(a);
                        if(!dontAdd)
                        {
                            _target->cardsAbilities.push_back(a);
                        }
                    }
                }
            }
            //power
            int powerMod = 0;
            int toughMod = 0;
            bool powerlessThanOriginal = false;
            bool toughLessThanOriginal = false;
            if(_target->power < _target->origpower)
            {
                powerMod = _target->origpower - _target->power;
                powerlessThanOriginal = true;
            }
            else
            {
                powerMod =_target->power - _target->origpower;
            }
            //toughness
            if(_target->toughness <= _target->origtoughness)
            {
                toughMod = _target->origtoughness - _target->toughness;
                toughLessThanOriginal = true;
            }
            else
            {
                toughMod =_target->toughness - _target->origtoughness;
            }
            _target->power = powerlessThanOriginal?myFlip->power - powerMod:myFlip->power + powerMod;
            _target->life = toughLessThanOriginal?myFlip->toughness - toughMod:myFlip->toughness + toughMod;
            _target->toughness = toughLessThanOriginal?myFlip->toughness - toughMod:myFlip->toughness + toughMod;
            _target->origpower = myFlip->origpower;
            _target->origtoughness = myFlip->origtoughness;
            SAFE_DELETE(myFlip);
            _target->mPropertiesChangedSinceLastUpdate = true;
        }

        currentAbilities.clear();
        testDestroy();
    }
    return 1;
}

int AAFlip::testDestroy()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(target)
    {
        if(_target->isFlipped)
        {
            this->forceDestroy = 1;
            _target->getObserver()->removeObserver(this);
            _target->isFlipped = false;
            return 1;
        }
    }
    return 0;
}

const char * AAFlip::getMenuText()
{
    string s = flipStats;
    sprintf(menuText, "Transform:%s", s.c_str());
    return menuText;
}

AAFlip * AAFlip::clone() const
{
    AAFlip * a = NEW AAFlip(*this);
    a->forceDestroy = 1;
    return a;
}
// AADYNAMIC: dynamic ability builder
AADynamic::AADynamic(GameObserver* observer, int id, MTGCardInstance * card, Damageable * _target,int type,int effect,int who,int amountsource,MTGAbility * storedAbility, ManaCost * _cost) :
ActivatedAbility(observer, id, card, _cost, 0),type(type),effect(effect),who(who),amountsource(amountsource),storedAbility(storedAbility)
{
    target = _target;
    sourceamount = 0;
    targetamount = 0;
    eachother = false;
    tosrc = false;
    menu = "";
    OriginalSrc = source;
    clonedStored = NULL;
    mainAbility = NULL;
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
    while (dynamic_cast<MTGCardInstance *>(_target) && ((MTGCardInstance *)_target)->next)
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
        _target = secondaryTarget;
    if (_target)
    {
        while (dynamic_cast<MTGCardInstance *>(_target) && ((MTGCardInstance *)_target)->next)
            _target = ((MTGCardInstance *)_target)->next;
        if(sourceamount < 0)
            sourceamount = 0;
        if(targetamount < 0)
            targetamount = 0;
        std::stringstream out;
        std::stringstream out2;
        out << sourceamount;
        string sourceamountstring = out.str();
        out2 << targetamount;
        string targetamountstring = out2.str();
        //set values less then 0 to 0, it was reported that negitive numbers such as a creature who get -3/-3 having the power become
        //negitive, if then used as the amount, would cuase weird side effects on resolves.
        switch(effect)
        {
        case DYNAMIC_ABILITY_EFFECT_STRIKE://deal damage
            {
                mainAbility = NEW AADamager(game, this->GetId(), source,tosrc == true?(Targetable*)OriginalSrc:(Targetable*)_target,sourceamountstring);
                activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                if(eachother)
                {
                    mainAbility = NEW AADamager(game, this->GetId(), (MTGCardInstance*)_target,(Targetable*)OriginalSrc,targetamountstring);
                    activateMainAbility(mainAbility,source,OriginalSrc);
                }
                return 1;
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_DRAW://draw cards
            {
                mainAbility = NEW AADrawer(game, this->GetId(), source,_target,NULL, sourceamountstring);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_LIFEGAIN://gain life
            {
                mainAbility = NEW AALifer(game, this->GetId(), source,_target, sourceamountstring);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPPOWER://pump power
            {
                mainAbility = NEW PTInstant(game, this->GetId(), source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target,NEW WParsedPT(sourceamount,0));
                return activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPTOUGHNESS://pump toughness
            {
                mainAbility = NEW PTInstant(game, this->GetId(), source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target,NEW WParsedPT(0,sourceamount));
                return activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_PUMPBOTH://pump both
            {
                mainAbility = NEW PTInstant(game, this->GetId(), source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target,NEW WParsedPT(sourceamount,sourceamount));
                return activateMainAbility(mainAbility,source,tosrc == true?OriginalSrc:(MTGCardInstance*)_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_LIFELOSS://lose life
            {
                string altered = "-";
                altered.append(sourceamountstring);
                mainAbility = NEW AALifer(game, this->GetId(), source,_target, altered);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_DEPLETE://deplete cards
            {
                mainAbility = NEW AADepleter(game, this->GetId(), source,_target, sourceamountstring);
                return activateMainAbility(mainAbility,source,_target);
                break;
            }
        case DYNAMIC_ABILITY_EFFECT_COUNTERSONEONE:
            {
                if(!dynamic_cast<MTGCardInstance *>(_target))
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
int AADynamic::activateMainAbility(MTGAbility * toActivate,MTGCardInstance * source , Damageable * target)
{
    if(storedAbility)
        activateStored();
    if(!toActivate)
        return 0;
    if(PTInstant * a = dynamic_cast<PTInstant *>(toActivate))
    {
        a->addToGame();
        return 1;
    }
    toActivate->oneShot = true;
    toActivate->forceDestroy = 1;
    toActivate->resolve();
    SAFE_DELETE(toActivate);
    return 1;
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
    
    return menu.c_str();
}

AADynamic * AADynamic::clone() const
{
    AADynamic * a = NEW AADynamic(*this);
    a->storedAbility = storedAbility? storedAbility->clone() : NULL;
    return a;
}

AADynamic::~AADynamic()
{
    SAFE_DELETE(storedAbility);
}

//AALifer
AALifer::AALifer(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, string life_s, ManaCost * _cost, int who) :
ActivatedAbilityTP(observer, _id, card, _target, _cost, who),life_s(life_s)
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
    return NEW AALifer(*this);
}


//players max hand size
AASetHand::AASetHand(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, int hand, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), hand(hand)
{
}

int AASetHand::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;

    p->handsize = hand;

    return 1;
}

const char * AASetHand::getMenuText()
{
    return "Set Hand Size";
}

AASetHand * AASetHand::clone() const
{
    return NEW AASetHand(*this);
}

//Lifeset
AALifeSet::AALifeSet(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, WParsedInt * life, ManaCost * _cost,
        int who) :
    ActivatedAbilityTP(observer, _id, _source, _target, _cost, who), life(life)
{
}

int AALifeSet::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    Player * p = getPlayerFromDamageable(_target);

    if (!p)
        return 0;

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
    return a;
}

AALifeSet::~AALifeSet()
{
    SAFE_DELETE(life);
}

//AACloner 
//cloning...this makes a token thats a copy of the target.
AACloner::AACloner(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int who,
        string abilitiesStringList,string TypesList) :
    ActivatedAbility(observer, _id, _source, _cost, 0), who(who)
{
    aType = MTGAbility::CLONING;
    target = _target;
    source = _source;
    if (abilitiesStringList.size() > 0)
    {
        PopulateAbilityIndexVector(awith, abilitiesStringList);
        PopulateColorIndexVector(colors, abilitiesStringList);
    }
    if (TypesList.size())
    {
        PopulateSubtypesIndexVector(typesToAdd,TypesList);
    }

}

int AACloner::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (!_target)
        return 0;

    MTGCard* clone = (_target->isToken ? _target: MTGCollection()->getCardByName(_target->name));

    Player * targetPlayer = who == 1 ? source->controller()->opponent() : source->controller();

    MTGCardInstance * myClone = NEW MTGCardInstance(clone, targetPlayer->game);
    targetPlayer->game->temp->addCard(myClone);
                
    Spell * spell = NEW Spell(game, myClone);
    spell->resolve();
    spell->source->isToken = 1;
    spell->source->fresh = 1;
    spell->source->model = spell->source;
    spell->source->model->data = spell->source;
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
    for (it = typesToAdd.begin(); it != typesToAdd.end(); it++)
    {
        spell->source->addType(*it);
    }
    delete spell;
    return 1;

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
    return NEW AACloner(*this);
}
AACloner::~AACloner()
{
}

// Cast/Play Restriction modifier
ACastRestriction::ACastRestriction(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who) :
   AbilityTP(observer, _id, card, _target, who), restrictionsScope(_restrictionsScope), value(_value), modifyExisting(_modifyExisting),zoneId(_zoneId)
{
    existingRestriction = NULL;
    targetPlayer = NULL;
}

int ACastRestriction::addToGame()
{
    Targetable * _target = getTarget();
    targetPlayer = getPlayerFromTarget(_target);
    if (!targetPlayer)
        return 0;

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
    return 1;
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
    return a;
}

ACastRestriction::~ACastRestriction()
{
    SAFE_DELETE(value);
    SAFE_DELETE(restrictionsScope);
}


AInstantCastRestrictionUEOT::AInstantCastRestrictionUEOT(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, TargetChooser * _restrictionsScope, WParsedInt * _value, bool _modifyExisting, int _zoneId, int who) :
    InstantAbilityTP(observer, _id, card, _target, who)
{
    ability = NEW ACastRestriction(observer, _id, card, _target, _restrictionsScope, _value, _modifyExisting,  _zoneId, who);
}

int AInstantCastRestrictionUEOT::resolve()
{
    ACastRestriction * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
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
    return a;
}

AInstantCastRestrictionUEOT::~AInstantCastRestrictionUEOT()
{
    SAFE_DELETE(ability);
}


//AAMover
AAMover::AAMover(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, string dest, ManaCost * _cost) :
    ActivatedAbility(observer, _id, _source, _cost, 0), destination(dest)
{
    if (_target)
        target = _target;
    andAbility = NULL;
}

MTGGameZone * AAMover::destinationZone(Targetable * target)
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    return MTGGameZone::stringToZone(game, destination, source, _target);
}

int AAMover::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        Player* p = _target->controller();
        if (p)
        {
            MTGGameZone * fromZone = _target->getCurrentZone();
            MTGGameZone * destZone = destinationZone(target);

            //inplay is a special zone !
            for (int i = 0; i < 2; i++)
            {
                if (destZone == game->players[i]->game->inPlay && fromZone != game->players[i]->game->inPlay && fromZone
                        != game->players[i]->opponent()->game->inPlay)
                {
                    MTGCardInstance * copy = game->players[i]->game->putInZone(_target, fromZone, game->players[i]->game->temp);
                    Spell * spell = NEW Spell(game, copy);
                    spell->resolve();
                    if(andAbility)
                    {
                        MTGAbility * andAbilityClone = andAbility->clone();
                        andAbilityClone->target = spell->source;
                        if(andAbility->oneShot)
                        {
                            andAbilityClone->resolve();
                            SAFE_DELETE(andAbilityClone);
                        }
                        else
                        {
                            andAbilityClone->addToGame();
                        }
                    }
                    delete spell;
                    return 1;
                }
            }
            p->game->putInZone(_target, fromZone, destZone);
            while(_target->next)
                _target = _target->next;
            if(andAbility)
            {
                MTGAbility * andAbilityClone = andAbility->clone();
                andAbilityClone->target = _target;
                if(andAbility->oneShot)
                {
                    andAbilityClone->resolve();
                    SAFE_DELETE(andAbilityClone);
                }
                else
                {
                    andAbilityClone->addToGame();
                }
            }
            return 1;
        }
    }
    return 0;
}

const char * AAMover::getMenuText()
{
    return "Move";
}

const char * AAMover::getMenuText(TargetChooser * tc)
{
    MTGGameZone * dest = destinationZone();

    for (int i = 0; i < 2; i++)
    {
        // Move card to hand
        if (dest == game->players[i]->game->hand)
        {
            if (tc->targetsZone(game->players[i]->game->inPlay))
                return "Bounce";
            if (tc->targetsZone(game->players[i]->game->graveyard))
                return "Reclaim";
            if (tc->targetsZone(game->opponent()->game->hand))
                return "Steal";
        }

        // Move card to graveyard
        else if (dest == game->players[i]->game->graveyard)
        {
            if (tc->targetsZone(game->players[i]->game->inPlay))
                return "Sacrifice";
            if (tc->targetsZone(game->players[i]->game->hand))
                return "Discard";
            if (tc->targetsZone(game->opponent()->game->hand))
                return "Opponent Discards";
        }

        // move card to library
        else if (dest == game->players[i]->game->library)
        {
            if (tc->targetsZone(game->players[i]->game->graveyard))
                return "Recycle";
            return "Put in Library";
        }

        // move card to battlefield
        else if (dest == game->players[i]->game->battlefield)
        {
            if (tc->targetsZone(game->players[i]->game->graveyard))
                return "Reanimate";
            return "Put in Play";
        }

        // move card into exile
        else if (dest == game->players[i]->game->exile)
        {
            return "Exile";
        }

        // move card from Library
        else if (tc->targetsZone(game->players[i]->game->library))
        {
            return "Fetch";
        }
    }

    return "Move";
}

AAMover * AAMover::clone() const
{
    AAMover * a = NEW AAMover(*this);
    if(andAbility)
    a->andAbility = andAbility->clone();
    return a;
}

AAMover::~AAMover()
{
SAFE_DELETE(andAbility);
}

//random movement of a card from zone to zone
AARandomMover::AARandomMover(GameObserver* observer, int _id, MTGCardInstance * _source, MTGCardInstance * _target, string _tcs, string _from, string _to) :
    ActivatedAbility(observer, _id, _source, NULL, 0), abilityTC(_tcs),fromZone(_from),toZone(_to)
{
    if (_target)
        target = _target;
}

MTGGameZone * AARandomMover::destinationZone(Targetable * target,string zone)
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    return MTGGameZone::stringToZone(game, zone, source, _target);
}

int AARandomMover::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (target)
    {
        Player* p = _target->controller();
        if (p)
        {
            MTGGameZone * fromDest = destinationZone(target,fromZone);
            MTGGameZone * toDest = destinationZone(target,toZone);

            if (!fromDest->nb_cards)
                return 0;

            TargetChooserFactory tcf(game);
            TargetChooser * rTc = tcf.createTargetChooser(abilityTC, source);
            rTc->targetter = NULL;
            rTc->setAllZones();
            vector<MTGCardInstance*>selectedCards;
            for(unsigned int i = 0; i < fromDest->cards.size();++i)
            {
                if(rTc->canTarget(fromDest->cards[i]))
                    selectedCards.push_back(fromDest->cards[i]);
            }
            SAFE_DELETE(rTc);
            if(!selectedCards.size())
                return 0;
            int r = fromDest->owner->getObserver()->getRandomGenerator()->random() % (selectedCards.size());
            MTGCardInstance * toMove = selectedCards[r];


            //inplay is a special zone !
            for (int i = 0; i < 2; i++)
            {
                if (toDest == game->players[i]->game->inPlay && fromDest != game->players[i]->game->inPlay && fromDest
                        != game->players[i]->opponent()->game->inPlay)
                {
                    MTGCardInstance * copy = game->players[i]->game->putInZone(toMove, fromDest, game->players[i]->game->temp);
                    Spell * spell = NEW Spell(game, copy);
                    spell->resolve();
                    delete spell;
                    return 1;
                }
            }
            p->game->putInZone(toMove, fromDest, toDest);
            return 1;
        }
    }
    return 0;
}

const char * AARandomMover::getMenuText()
{
    return "Dig";
}

AARandomMover * AARandomMover::clone() const
{
    AARandomMover * a = NEW AARandomMover(*this);
    return a;
}

AARandomMover::~AARandomMover()
{
}

//Random Discard
AARandomDiscarder::AARandomDiscarder(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target,string nbcardsStr, ManaCost * _cost,
         int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who), nbcardsStr(nbcardsStr)
{
}

int AARandomDiscarder::resolve()
{
    Targetable * _target = getTarget();
    Player * player = getPlayerFromTarget(_target);
    if (player)
    {
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
    return NEW AARandomDiscarder(*this);
}

// Shuffle 
AAShuffle::AAShuffle(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who)
{
}

int AAShuffle::resolve()
{
    Player * player = getPlayerFromTarget(getTarget());
    if (player)
    {
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
    return NEW AAShuffle(*this);
}

// Remove Mana From ManaPool
AARemoveMana::AARemoveMana(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, string manaDesc, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, NULL, who)
{
    if (!manaDesc.size())
    {
        DebugTrace("ALL_ABILITIES: AARemoveMana ctor error"); 
        return;
    }
    mRemoveAll = (manaDesc[0] == '*');
    if (mRemoveAll)
        manaDesc = manaDesc.substr(1);
    
    mManaDesc = (manaDesc.size()) ? ManaCost::parseManaCost(manaDesc) : NULL;

}

int AARemoveMana::resolve()
{
    Player * player = getPlayerFromTarget(getTarget());
    if (player)
    {
        ManaPool * manaPool = player->getManaPool();
        if (mRemoveAll)
        {
            if (mManaDesc) // Remove all mana Matching a description
            {
                    for (int i = 0; i < Constants::NB_Colors; i++)
                    {
                        if (mManaDesc->hasColor(i))
                            manaPool->removeAll(i);
                    }
            }
            else //Remove all mana
            {
                manaPool->Empty();
            }
        }
        else //remove a "standard" mana Description
        {
            ((ManaCost *)manaPool)->remove(mManaDesc); //why do I have to cast here?
        }
    }
    return 1;
}

const char * AARemoveMana::getMenuText()
{
    if (mRemoveAll && !mManaDesc)
        return "Empty Manapool";
    return "Remove Mana";
}

AARemoveMana * AARemoveMana::clone() const
{
    AARemoveMana * a = NEW AARemoveMana(*this);
    a->mManaDesc = mManaDesc ? NEW ManaCost(mManaDesc) : NULL;
    return a;
}

AARemoveMana::~AARemoveMana()
{
    SAFE_DELETE(mManaDesc);
}

//Tapper
AATapper::AATapper(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, id, card, _cost, 0)
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
    return NEW AATapper(*this);
}

//AA Untapper
AAUntapper::AAUntapper(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
    ActivatedAbility(observer, id, card, _cost, 0)
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
    return NEW AAUntapper(*this);
}

AAWhatsMax::AAWhatsMax(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * source, ManaCost * _cost, int value) :
    ActivatedAbility(observer, id, card, _cost, 0), value(value)
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
    return NEW AAWhatsMax(*this);
}

// Win Game
AAWinGame::AAWinGame(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost, int who) :
    ActivatedAbilityTP(observer, _id, card, _target, _cost, who)
{
}

int AAWinGame::resolve()
{
    Player * p = getPlayerFromDamageable((Damageable *) getTarget());
    if (!p)
        return 0;

    bool canwin = true;

    MTGGameZone * z = p->opponent()->game->inPlay;
    int nbcards = z->nb_cards;

    for (int i = 0; i < nbcards; i++)
    {
        MTGCardInstance * c = z->cards[i];
        if (c->has(Constants::CANTLOSE))
        {
            canwin = false;
            break;
        }
    }

    if (canwin)
    {
        MTGGameZone * k = p->game->inPlay;
        int onbcards = k->nb_cards;
        for (int m = 0; m < onbcards; ++m)
        {
            MTGCardInstance * e = k->cards[m];
            if (e->has(Constants::CANTWIN))
            {
                canwin = false;
                break;
            }
        }
    }

    if (canwin)
    {
        game->setLoser(p->opponent());
    }

    return 1;
}

const char * AAWinGame::getMenuText()
{
    return "Win Game";
}

AAWinGame * AAWinGame::clone() const
{
    return NEW AAWinGame(*this);
}

//Generic Abilities

//a new affinity
ANewAffinity::ANewAffinity(GameObserver* observer, int _id, MTGCardInstance * _source, string Tc, string mana) :
MTGAbility(observer, _id, _source), tcString(Tc), manaString(mana)
{
}

void ANewAffinity::Update(float dt)
{
    testDestroy();
    return;
}

int ANewAffinity::testDestroy()
{
    if(this->source->isInPlay(game))
        return 1;
    return 0;
}
ANewAffinity * ANewAffinity::clone() const
{
    return NEW ANewAffinity(*this);
}

//IfThenEffect
IfThenAbility::IfThenAbility(GameObserver* observer, int _id, MTGAbility * delayedAbility, MTGAbility * delayedElseAbility, MTGCardInstance * _source, Targetable * _target, int type,string Cond) :
InstantAbility(observer, _id, _source),delayedAbility(delayedAbility),delayedElseAbility(delayedElseAbility), type(type),Cond(Cond)
{
    target = _target;
}

int IfThenAbility::resolve()
{
    MTGCardInstance * card = (MTGCardInstance*)source;
    AbilityFactory af(game);
    Targetable* aTarget = (Targetable*)target;
    int checkCond = af.parseCastRestrictions(card,card->controller(),Cond);
    if(Cond.find("cantargetcard(") != string::npos)
    {
        TargetChooser * condTc = NULL;
        vector<string>splitTarget = parseBetween(Cond, "card(", ")");
        if (splitTarget.size())
        {
            TargetChooserFactory tcf(game);
            condTc = tcf.createTargetChooser(splitTarget[1], source);
            condTc->targetter = NULL;
            if(aTarget)
                checkCond = condTc->canTarget(aTarget);
            SAFE_DELETE(condTc);
        }

    }
    MTGAbility * a1 = NULL;
    if((checkCond && type == 1)||(!checkCond && type == 2))
    {
        a1 = delayedAbility->clone();
    }
    else if(delayedElseAbility)
    {
        a1 = delayedElseAbility->clone();
    }
    if (!a1)
        return 0;
    else
    {
        if(a1->target && !dynamic_cast<Player *>(a1->target))
            a1->target = aTarget;

        if(a1->oneShot)
        {
            a1->resolve();
            SAFE_DELETE(a1);
        }
        else
            a1->addToGame();
        return 1;
    }
    return 0;
}

const char * IfThenAbility::getMenuText()
{
    return "";
}

IfThenAbility * IfThenAbility::clone() const
{
    IfThenAbility * a = NEW IfThenAbility(*this);
    a->delayedAbility = delayedAbility->clone();
    return a;
}

IfThenAbility::~IfThenAbility()
{
    SAFE_DELETE(delayedAbility);
    SAFE_DELETE(delayedElseAbility);
}
//
//May Abilities
MayAbility::MayAbility(GameObserver* observer, int _id, MTGAbility * _ability, MTGCardInstance * _source, bool must,string _cond) :
    MTGAbility(observer, _id, _source), NestedAbility(_ability), must(must), Cond(_cond)
{
    triggered = 0;
    mClone = NULL;
    optionalCost = NULL;
}

void MayAbility::Update(float dt)
{
    MTGAbility::Update(dt);
    if (!triggered && !game->getCurrentTargetChooser() && (!game->mLayers->actionLayer()->menuObject||game->mLayers->actionLayer()->menuObject == source))
    {
        triggered = 1;
        if(optionalCost && !source->controller()->getManaPool()->canAfford(optionalCost))
            return;
        if(Cond.size())
        {
            AbilityFactory af(game);
            int checkCond = af.parseCastRestrictions(source,source->controller(),Cond);
            if(!checkCond)
            {
                return;
            }
        }
        if (TargetAbility * ta = dynamic_cast<TargetAbility *>(ability))
        {
            if (!ta->getActionTc()->validTargetsExist() || ta->getActionTc()->maxtargets == 0)
                return;
        }
        game->mLayers->actionLayer()->setMenuObject(source, must);
        previousInterrupter = game->isInterrupting;
        game->mLayers->stackLayer()->setIsInterrupting(source->controller(), false);
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
    if(game->currentPlayer == source->controller() && game->isInterrupting == source->controller() && dynamic_cast<AManaProducer*>(AbilityFactory::getCoreAbility(ability)))
        //if its my turn, and im interrupting myself(why?) then set interrupting to previous interrupter if the ability was a manaability
        //special case since they don't use the stack.
        game->mLayers->stackLayer()->setIsInterrupting(previousInterrupter, false);
    return 1;
}

int MayAbility::isReactingToTargetClick(Targetable * card)
{
    if (card == source)
    {
        if(!optionalCost || source->controller()->getManaPool()->canAfford(optionalCost))
            return 1;
    }
    return 0;
}

int MayAbility::reactToTargetClick(Targetable * object)
{
    mClone = ability->clone();
    if(optionalCost)
    {
        source->controller()->getManaPool()->pay(optionalCost);
        optionalCost->setExtraCostsAction(this, source);
        optionalCost->doPayExtra();
    }
    mClone->addToGame();
    mClone->forceDestroy = 1;
    return mClone->reactToTargetClick(object);
}

MayAbility * MayAbility::clone() const
{
    MayAbility * a = NEW MayAbility(*this);
    a->ability = ability->clone();
    a->optionalCost = this->optionalCost;
    return a;
}

MayAbility::~MayAbility()
{
    SAFE_DELETE(ability);
    SAFE_DELETE(optionalCost);
}

//Menu building ability Abilities
//this will eventaully handle choosen discards/sacrifices.
MenuAbility::MenuAbility(GameObserver* observer, int _id, Targetable * mtarget, MTGCardInstance * _source, bool must,vector<MTGAbility*>abilities,Player * who, string newName) :
MayAbility(observer, _id,NULL,_source,must), must(must),abilities(abilities),who(who),newNameString(newName)
{
    triggered = 0;
    mClone = NULL;
    this->target = mtarget;
    removeMenu = false;
}

void MenuAbility::Update(float dt)
{
    MTGAbility::Update(dt);
    ActionLayer * object = game->mLayers->actionLayer();
    if (!triggered && !object->menuObject && !object->getCurrentTargetChooser())
    {

        triggered = 1;
        object->currentActionCard = (MTGCardInstance*)this->target;
        if (TargetAbility * ta = dynamic_cast<TargetAbility *>(ability))
        {
            if (!ta->getActionTc()->validTargetsExist())
                return;
        }
    }
    if(object->currentActionCard && this->target != object->currentActionCard)
    {
        triggered = 0;
    }
    if(triggered)
    {
        game->mLayers->actionLayer()->setCustomMenuObject(source, must,abilities,newNameString.size()?newNameString.c_str():"");
        previousInterrupter = game->isInterrupting;
        game->mLayers->stackLayer()->setIsInterrupting(source->controller(), false);
    }
}

int MenuAbility::resolve()
{
    this->triggered = 1;
    MTGAbility * a = this;
    return a->addToGame();
}

const char * MenuAbility::getMenuText()
{
    if((abilities.size() > 1 && must)||(abilities.size() > 2 && !must))
        return "choose one";
    return "Action";
}

int MenuAbility::testDestroy()
{
    if (!removeMenu)
        return 0;
    if (game->mLayers->actionLayer()->menuObject)
        return 0;
    if (game->mLayers->actionLayer()->getIndexOf(mClone) != -1)
        return 0;

    return 1;
}

int MenuAbility::isReactingToTargetClick(Targetable * card){return MayAbility::isReactingToTargetClick(card);}
int MenuAbility::reactToTargetClick(Targetable * object){return 1;}

int MenuAbility::reactToChoiceClick(Targetable * object,int choice,int control)
{
    ActionElement * currentAction = (ActionElement *) game->mLayers->actionLayer()->mObjects[control];
    if(currentAction != (ActionElement*)this)
        return 0;
    if(!abilities.size()||!triggered)
        return 0;
    for(int i = 0;i < int(abilities.size());i++)
    {
        if(choice == i)
            mClone = abilities[choice]->clone();
        else
            SAFE_DELETE(abilities[i]);
        //else
        //    abilities[i]->clone();//all get cloned for clean up purposes. EDIT:removed, cause memleaks.
    }
    if(!mClone)
    {
        if (source->controller() == game->isInterrupting)
            game->mLayers->stackLayer()->cancelInterruptOffer(ActionStack::DONT_INTERRUPT, false);
        return 0;
    }
    mClone->target = abilities[choice]->target;
    mClone->oneShot = true;
    mClone->forceDestroy = 1;
    mClone->canBeInterrupted = false;
    mClone->resolve();
    SAFE_DELETE(mClone);
    if (source->controller() == game->isInterrupting)
        game->mLayers->stackLayer()->cancelInterruptOffer(ActionStack::DONT_INTERRUPT, false);
    this->forceDestroy = 1;
    removeMenu = true;
    return reactToTargetClick(object);
}

MenuAbility * MenuAbility::clone() const
{
    MenuAbility * a = NEW MenuAbility(*this);
    a->canBeInterrupted = false;
    if(abilities.size())
    {
        for(int i = 0;i < int(abilities.size());i++)
        {
            a->abilities.push_back(abilities[i]->clone());
            a->abilities[i]->target = abilities[i]->target;
        }
    }
    else
    a->ability = ability->clone();
    return a;
}

MenuAbility::~MenuAbility()
{
    if(abilities.size())
    {
        for(int i = 0;i < int(abilities.size());i++)
        {
            if(abilities[i])
            {
                AASetColorChosen * chooseA = dynamic_cast<AASetColorChosen *>(abilities[i]);
                if(chooseA && chooseA->abilityAltered)
                    SAFE_DELETE(chooseA->abilityAltered);
                SAFE_DELETE(abilities[i]);
            }
        }
    }
    else
        SAFE_DELETE(ability);
}
///
//MultiAbility : triggers several actions for a cost
MultiAbility::MultiAbility(GameObserver* observer, int _id, MTGCardInstance * card, Targetable * _target, ManaCost * _cost) :
    ActivatedAbility(observer, _id, card, _cost, 0)
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
    for (size_t i = 0; i < abilities.size(); ++i)
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
        if(Phaseactiontarget && dynamic_cast<APhaseActionGeneric *> (abilities[i]))
            abilities[i]->target = Phaseactiontarget;
    }
    return 1;
}

int MultiAbility::addToGame()
{
    for (size_t i = 0; i < abilities.size(); ++i)
    {
        if (abilities[i] == NULL)
            continue;

        MTGAbility * a = abilities[i]->clone();
        a->target = target;
        a->addToGame();
        clones.push_back(a);
    }
    MTGAbility::addToGame();
    return 1;
}

int MultiAbility::destroy()
{
    for (size_t i = 0; i < clones.size(); ++i)
    {
        //I'd like to call game->removeObserver here instead of using forceDestroy, but I get a weird crash after that, need to investigate a bit
        clones[i]->forceDestroy = 1;
    }
    clones.clear();
    return ActivatedAbility::destroy();
}

const char * MultiAbility::getMenuText()
{
    if (abilities.size() && abilities[0])
        return abilities[0]->getMenuText();
    return "";
}

MultiAbility * MultiAbility::clone() const
{
    MultiAbility * a = NEW MultiAbility(*this);
    a->abilities.clear();
    for (size_t i = 0; i < abilities.size(); ++i)
    {
        a->abilities.push_back(abilities[i]->clone());
    }
    return a;
}

MultiAbility::~MultiAbility()
{
    for (size_t i = 0; i < abilities.size(); ++i)
    {
        SAFE_DELETE(abilities[i]);
    }

    abilities.clear();
}
//Generic Target Ability
GenericTargetAbility::GenericTargetAbility(GameObserver* observer, string newName, string castRestriction, int _id, MTGCardInstance * _source, TargetChooser * _tc, MTGAbility * a,
        ManaCost * _cost, string limit,MTGAbility * sideEffects,string usesBeforeSideEffects, int restrictions, MTGGameZone * dest,string _tcString) :
    TargetAbility(observer, _id, _source, _tc, _cost, restrictions, castRestriction), limit(limit), activeZone(dest),newName(newName),sideEffects(sideEffects),usesBeforeSideEffects(usesBeforeSideEffects),tcString(_tcString)
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

    //Special case for move
    MTGAbility * core = AbilityFactory::getCoreAbility(ability);
    if (AAMover * move = dynamic_cast<AAMover *>(core))
        return (move->getMenuText(tc));
    return ability->getMenuText();

}

int GenericTargetAbility::resolve()
{
    counters++;
    tc->done = false;
    if(sideEffects && usesBeforeSideEffects.size())
    {
        WParsedInt * use = NEW WParsedInt(usesBeforeSideEffects.c_str(),NULL,source);
        uses = use->getValue();
        delete use;
        if(counters == uses)
        {
            sa = sideEffects->clone();
            sa->target = this->target;
            sa->source = this->source;
            if(sa->oneShot)
            {
                sa->fireAbility();
            }
            else
            {
                GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), sa);
                wrapper->addToGame();
            }
        }
    }
    return TargetAbility::resolve();
}

int GenericTargetAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{
    limitPerTurn = 0;
    if(limit.size())
    {
        WParsedInt value(limit.c_str(),NULL,source);
        limitPerTurn = value.getValue();
    }
    if (limitPerTurn && counters >= limitPerTurn)
        return 0;
    if(tcString.size() && !tc->targetListSet())
    {
        TargetChooser * current = this->getActionTc();
        TargetChooserFactory tcf(game);
        TargetChooser *refreshed = tcf.createTargetChooser(tcString, source, this);
        refreshed->setTargetsTo(current->getTargetsFrom());
        this->setActionTC(refreshed);
        SAFE_DELETE(current);
    }
    return TargetAbility::isReactingToClick(card, mana);
}

void GenericTargetAbility::Update(float dt)
{
    if (newPhase != currentPhase && newPhase == MTG_PHASE_AFTER_EOT)
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
    return a;
}

GenericTargetAbility::~GenericTargetAbility()
{
    SAFE_DELETE(ability);
    SAFE_DELETE(sideEffects);
}

//Alter Cost
AAlterCost::AAlterCost(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, int amount, int type) :
MTGAbility(observer, id, source, target), amount(amount), type(type)
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
            _target->getReducedManaCost()->copy(reduced);
            delete reduced;
        }
        _target->getReducedManaCost()->add(type,abs(amount));
    }
    return MTGAbility::addToGame();
}

int AAlterCost::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if(!this->manaReducer->isInPlay(game))
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

int AAlterCost::testDestroy()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if(!this->manaReducer->isInPlay(game))
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
        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::NB_Colors;k++)
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
        for(int k = Constants::MTG_COLOR_ARTIFACT; k < Constants::NB_Colors;k++)
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
    return NEW AAlterCost(*this);
}

AAlterCost::~AAlterCost()
{
}

// ATransformer
ATransformer::ATransformer(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string stypes, string sabilities,string newpower,bool newpowerfound,string newtoughness,bool newtoughnessfound,vector<string> newAbilitiesList,bool newAbilityFound,bool aForever, bool aUntilNext) :
    MTGAbility(observer, id, source, target),newpower(newpower),newpowerfound(newpowerfound),newtoughness(newtoughness),newtoughnessfound(newtoughnessfound),newAbilitiesList(newAbilitiesList),newAbilityFound(newAbilityFound),aForever(aForever),UYNT(aUntilNext)
{

    PopulateAbilityIndexVector(abilities, sabilities);
    PopulateColorIndexVector(colors, sabilities);
    if(sabilities.find("chosencolor") != string::npos)
    {
        colors.push_back(source->chooseacolor);
    }
    myCurrentTurn = 1000;
    //this subkeyword adds a color without removing the existing colors.
    addNewColors = (sabilities.find("newcolors") != string::npos);
	remove = (stypes.find("removealltypes") != string::npos);
	removeCreatureSubtypes = (stypes.find("removecreaturesubtypes") != string::npos);
    removeTypes = (stypes.find("removetypes") != string::npos);

	if (stypes.find("allsubtypes") != string::npos || stypes.find("removecreaturesubtypes") != string::npos)
	{  
        const vector<string> values = MTGAllCards::getValuesById();
        for (size_t i = 0; i <values.size(); ++i)
        {
            if (!MTGAllCards::isSubtypeOfType(i,Subtypes::TYPE_CREATURE))
                continue;

            types.push_back(i);
        }
	}
    else
    {
        if(stypes.find("chosentype") != string::npos)
        {
            stypes = source->chooseasubtype;
        }
        PopulateSubtypesIndexVector(types, stypes);
    }

    menu = stypes;
}

int ATransformer::addToGame()
{
    if(UYNT)
        myCurrentTurn = game->turn;
    MTGCardInstance * _target = NULL;
        Interruptible * action = (Interruptible *) target;
    if (action && action->type == ACTION_SPELL && action->state == NOT_RESOLVED)
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

    if (!_target)
    {
        DebugTrace("ALL_ABILITIES: Target not set in ATransformer::addToGame\n");
        return 0;
    }
        
    while (_target->next)
        _target = _target->next;

    for (int j = 0; j < Constants::NB_Colors; j++)
    {
        if (_target->hasColor(j))
            oldcolors.push_back(j);
    }
    for (size_t j = 0; j < _target->types.size(); ++j)
        oldtypes.push_back( _target->types[j]);

    list<int>::iterator it;
    for (it = colors.begin(); it != colors.end(); it++)
    {
        if(!addNewColors)
            _target->setColor(0, 1);
    }

    if (removeTypes)
    {
        //remove the main types from a card, ie: hidden enchantment cycle.
        for (int i = 0; i < Subtypes::LAST_TYPE; ++ i)
            _target->removeType(i,1);
    }
	else if (remove)
	{
		for (it = oldtypes.begin(); it != oldtypes.end(); it++)
		{
			_target->removeType(*it);
		}
	}
	else
	{
for (it = types.begin(); it != types.end(); it++)
		{

            if(removeCreatureSubtypes)
            {
                _target->removeType(*it);
            }
            else if(_target->hasSubtype(*it))
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
        _target->basicAbilities.set(*it);
    }

    if(newAbilityFound)
    {
        for (unsigned int k = 0 ; k < newAbilitiesList.size();k++)
        {
            AbilityFactory af(game);
            MTGAbility * aNew = af.parseMagicLine(newAbilitiesList[k], 0, NULL, _target);
            if(!aNew)
                continue;
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
            MultiAbility * abi = dynamic_cast<MultiAbility*>(aNew);
            if (abi)
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
            {
                aNew->addToGame();
                newAbilities[_target].push_back(aNew);
            }
        }
    }
    if(newpowerfound )
    {
        WParsedInt * val = NEW WParsedInt(newpower,NULL, source);
        oldpower = _target->power;
        _target->power += val->getValue();
        _target->power -= oldpower;
        _target->power += reapplyCountersBonus(_target,false,true);
        delete val;
    }
    if(newtoughnessfound )
    {
        WParsedInt * val = NEW WParsedInt(newtoughness,NULL, source);
        oldtoughness = _target->toughness;
        _target->addToToughness(val->getValue());
        _target->addToToughness(-oldtoughness);
        _target->addToToughness(reapplyCountersBonus(_target,true,false));
        _target->life = _target->toughness;
        delete val;
    }

    return MTGAbility::addToGame();
}
    
    int ATransformer::reapplyCountersBonus(MTGCardInstance * rtarget,bool powerapplied,bool toughnessapplied)
    {
        if(!rtarget->counters || !rtarget->counters->counters.size())
            return 0;
        Counter * c = rtarget->counters->counters[0];
        int rNewPower = 0;
        int rNewToughness = 0;
        for (int t = 0; t < rtarget->counters->mCount; t++)
        {
            if (c)
            {
                for(int i = 0;i < c->nb;i++)
                {
                    rNewPower += c->power;
                    rNewToughness += c->toughness;   
                }
            }
            c = rtarget->counters->getNext(c);
        }
        if(toughnessapplied)
            return rNewToughness;
        return rNewPower;
    }

    int ATransformer::testDestroy()
    {
        if(UYNT)
        {
            if(myCurrentTurn != 1000 && game->turn > myCurrentTurn && source->controller()->getId() == game->currentPlayer->getId())
            {
                return 1;
            }
        }
        return MTGAbility::testDestroy();
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
        
        if (!remove)
        {
            for (it = types.begin(); it != types.end(); it++)
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
            _target->basicAbilities.reset(*it);
        }

        for (it = oldcolors.begin(); it != oldcolors.end(); it++)
        {
            _target->setColor(*it);
        }

        if(newpowerfound )
        {
            _target->power = oldpower;
        }
        if(newtoughnessfound )
        {
            _target->setToughness(oldtoughness);
        }
        if(newAbilityFound)
        {
            for (unsigned int i = 0;i < newAbilities[_target].size(); i++)
            {
                if(newAbilities[_target].at(i))
                {
                    newAbilities[_target].at(i)->forceDestroy = 1;
                    newAbilities[_target].at(i)->removeFromGame();
                }
            }
            if (newAbilities.find(_target) != newAbilities.end())
            {
                newAbilities.erase(_target);
            }
        }
		if (remove || removeCreatureSubtypes)
		{
			for (it = oldtypes.begin(); it != oldtypes.end(); it++)
			{
                if(!_target->hasSubtype(*it))
                    _target->addType(*it);
			}
		}
		////in the case that we removed or added types to a card, so that it retains its original name when the effect is removed.
		//if(_target->model->data->name.size())//tokens don't have a model name.
		//    _target->setName(_target->model->data->name.c_str());

        //edit: this ability shouldn't have to reset the name on a card becuase removing a subtype changes the name of a land.
        //that should be handled in addType...not here.
        //im sure commenting this out will reintroduce a bug somewhere but it needs to be handled correctly. furthermore, why does adding and removing a type touch the name of a card?
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
    return NEW ATransformer(*this);
}

ATransformer::~ATransformer()
{
}

//ATransformerInstant
ATransformerInstant::ATransformerInstant(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, string types, string abilities,string newpower,bool newpowerfound,string newtoughness,bool newtoughnessfound,vector<string>newAbilitiesList,bool newAbilityFound,bool aForever,bool aUntilNext) :
    InstantAbility(observer, id, source, target),newpower(newpower),newpowerfound(newpowerfound),newtoughness(newtoughness),newtoughnessfound(newtoughnessfound),newAbilitiesList(newAbilitiesList),newAbilityFound(newAbilityFound),aForever(aForever),UYNT(aUntilNext)
{
    ability = NEW ATransformer(game, id, source, target, types, abilities,newpower,newpowerfound,newtoughness,newtoughnessfound,newAbilitiesList,newAbilityFound,aForever,aUntilNext);
    aType = MTGAbility::STANDARD_BECOMES;
}

int ATransformerInstant::resolve()
{
    ATransformer * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
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
    return a;
}

ATransformerInstant::~ATransformerInstant()
{
    SAFE_DELETE(ability);
}

//P/t ueot
PTInstant::PTInstant(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target, WParsedPT * wppt,string s,bool nonstatic) :
InstantAbility(observer, id, source, target), wppt(wppt),s(s),nonstatic(nonstatic)
{
    ability = NEW APowerToughnessModifier(game, id, source, target, wppt,s,nonstatic);
    aType = MTGAbility::STANDARD_PUMP;
}

int PTInstant::resolve()
{
    APowerToughnessModifier * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
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
    return a;
}

PTInstant::~PTInstant()
{
    SAFE_DELETE(ability);
}
// ASwapPTUEOT
ASwapPTUEOT::ASwapPTUEOT(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * target) :
    InstantAbility(observer, id, source, target)
{
    ability = NEW ASwapPT(observer, id, source, target);
}

int ASwapPTUEOT::resolve()
{
    ASwapPT * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
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
    return a;
}

ASwapPTUEOT::~ASwapPTUEOT()
{
    SAFE_DELETE(ability);
}

//exhange life with targetchooser
AAExchangeLife::AAExchangeLife(GameObserver* observer, int _id, MTGCardInstance * _source, Targetable * _target, ManaCost * _cost,
    int who) :
ActivatedAbilityTP(observer, _id, _source, _target, _cost, who)
{
}

int AAExchangeLife::resolve()
{
    Damageable * _target = (Damageable *) getTarget();
    if (_target)
    {
        Player *player = source->controller();
        int oldlife = player->getLife();
        int targetOldLife = _target->getLife();
        int modifier = oldlife > targetOldLife? oldlife - targetOldLife:targetOldLife - oldlife;
        if (_target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        {
            int increaser = 0;
            MTGCardInstance * card = ((MTGCardInstance*)_target);
            int toughMod = 0;
            targetOldLife <= card->origtoughness?toughMod = card->origtoughness - targetOldLife: toughMod = targetOldLife - card->origtoughness;
            if(oldlife > targetOldLife)
            {
                increaser = oldlife - targetOldLife;
                player->gainOrLoseLife(modifier * -1);
                card->addToToughness(increaser+toughMod);
            }
            else
            {
                _target->life = oldlife;
                card->toughness = oldlife;
                player->gainOrLoseLife(modifier);
            }

            return 1;
        }
        Player * opponent = (Player*)_target;
        if(oldlife > targetOldLife)
        {
            player->gainOrLoseLife(modifier * -1);
            opponent->gainOrLoseLife(modifier);
        }
        else
        {
            player->gainOrLoseLife(modifier);
            opponent->gainOrLoseLife(modifier * -1);
        }
        return 1;
    }
    return 0;
}

const char * AAExchangeLife::getMenuText()
{
    return "Exchange life";
}

AAExchangeLife * AAExchangeLife::clone() const
{
    return NEW AAExchangeLife(*this);
}

//ALoseAbilities
ALoseAbilities::ALoseAbilities(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target) :
    MTGAbility(observer, id, source)
{
    target = _target;
}

int ALoseAbilities::addToGame()
{
    if (storedAbilities.size())
    {
        DebugTrace("FATAL:storedAbilities shouldn't be already set inALoseAbilitie\n");
        return 0;
    }
    MTGCardInstance * _target = (MTGCardInstance *)target;

    ActionLayer * al = game->mLayers->actionLayer();


    //Build a list of Lords in game, this is a hack mostly for lands, see below
    vector <ALord *> lordsInGame;
    for (int i = (int)(al->mObjects.size()) - 1; i > 0; i--)      //0 is not a mtgability...hackish
    {
        if (al->mObjects[i])
        {
            MTGAbility * currentAction = (MTGAbility *) al->mObjects[i];
			ALord * l = dynamic_cast<ALord*> (currentAction);
			if(l)
                lordsInGame.push_back(l);
        }
    }

    for (int i = (int)(al->mObjects.size()) - 1; i > 0; i--)      //0 is not a mtgability...hackish
    {
        if (al->mObjects[i])
        {
            MTGAbility * currentAction = (MTGAbility *) al->mObjects[i];
			ALoseAbilities * la = dynamic_cast<ALoseAbilities*> (currentAction);
			if(la)
				continue;
            if (currentAction->source == _target)
            {
                bool canRemove = true;

                //Hack: we don't remove abilities on the card if they are provided by an external lord ability.
                //This is partly to solve the following issues:
                // http://code.google.com/p/wagic/issues/detail?id=647
                // http://code.google.com/p/wagic/issues/detail?id=700
                // But also because "most" abilities granted by lords will actually go away by themselves,
                // based on the fact that we usually remove abilities AND change the type of the card
                //Also in a general way we don't want to remove the card's abilities if it is provided by a Lord,
                //although there is also a problem with us not handling the P/T layer correctly
                for (size_t i = 0; i < lordsInGame.size(); ++i)
                {
                    if (lordsInGame[i]->isParentOf(_target, currentAction))
                    {
                        canRemove = false;
                        break;
                    }
                }
                
                if (canRemove)
                {
                    storedAbilities.push_back(currentAction);
                    al->removeFromGame(currentAction);
                }
            }
        }
    }

    return MTGAbility::addToGame();
}

int ALoseAbilities::destroy()
{
    for (size_t i = 0; i < storedAbilities.size(); ++i)
    {
        MTGAbility * a = storedAbilities[i];
        //OneShot abilities are not supposed to stay in the game for long.
        // If we copied one, something wrong probably happened
		if (a->oneShot)
		{
            DebugTrace("ALLABILITIES: Ability should not be one shot");
			continue;
        }

        //Avoid inifinite loop of removing/putting back abilities
        if (dynamic_cast<ALoseAbilities*> (a))
        {
			DebugTrace("ALLABILITIES: loseability won't be put in the loseability list");
			continue;
		}

 		a->addToGame();
    }
    storedAbilities.clear();
    return 1;
}

ALoseAbilities * ALoseAbilities::clone() const
{
    return NEW ALoseAbilities(*this);
}

//ALoseSubtypes
ALoseSubtypes::ALoseSubtypes(GameObserver* observer, int id, MTGCardInstance * source, MTGCardInstance * _target, int parentType) :
    MTGAbility(observer, id, source), parentType(parentType)
{
    target = _target;
}

int ALoseSubtypes::addToGame()
{
    if (storedSubtypes.size())
    {
        DebugTrace("FATAL:storedSubtypes shouldn't be already set inALoseSubtypes\n");
        return 0;
    }
    MTGCardInstance * _target = (MTGCardInstance *)target;

    for (int i = ((int)_target->types.size())-1; i >= 0; --i)
    {
        int subtype = _target->types[i];
        if (MTGAllCards::isSubtypeOfType(subtype, parentType))
        {
            storedSubtypes.push_back(subtype);
            _target->removeType(subtype);
        }
    }

    return MTGAbility::addToGame();
}

int ALoseSubtypes::destroy()
{
    MTGCardInstance * _target = (MTGCardInstance *)target;
    for (size_t i = 0; i < storedSubtypes.size(); ++i)
        _target->addType(storedSubtypes[i]);
    storedSubtypes.clear();
    return 1;
}

ALoseSubtypes * ALoseSubtypes::clone() const
{
    return NEW ALoseSubtypes(*this);
}

//APreventDamageTypes
APreventDamageTypes::APreventDamageTypes(GameObserver* observer, int id, MTGCardInstance * source, string to, string from, int type) :
    MTGAbility(observer, id, source), to(to), from(from), type(type)
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
    TargetChooserFactory tcf(game);
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
    a->re = NULL;
    return a;
}

APreventDamageTypes::~APreventDamageTypes()
{
    SAFE_DELETE(re);
}

//APreventDamageTypesUEOT
APreventDamageTypesUEOT::APreventDamageTypesUEOT(GameObserver* observer, int id, MTGCardInstance * source, string to, string from, int type) :
    InstantAbility(observer, id, source)
{
    ability = NEW APreventDamageTypes(observer, id, source, to, from, type);
}

int APreventDamageTypesUEOT::resolve()
{
    APreventDamageTypes * a = ability->clone();
    GenericInstantAbility * wrapper = NEW GenericInstantAbility(game, 1, source, (Damageable *) (this->target), a);
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
    return a;
}

APreventDamageTypesUEOT::~APreventDamageTypesUEOT()
{
    SAFE_DELETE(ability);
}

//AVanishing creature also fading
AVanishing::AVanishing(GameObserver* observer, int _id, MTGCardInstance * card, ManaCost * _cost, int restrictions, int amount, string counterName) :
MTGAbility(observer, _id, source, target),amount(amount),counterName(counterName)
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
        if(newPhase == MTG_PHASE_UPKEEP)
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
                if (newPhase == MTG_PHASE_UPKEEP && timeLeft <= 0 && next == 0)
                {
                    MTGCardInstance * beforeCard = source;
                    source->controller()->game->putInGraveyard(source);
                    WEvent * e = NEW WEventCardSacrifice(beforeCard,source);
                    game->receiveEvent(e);
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
    return NEW AVanishing(*this);
}

AVanishing::~AVanishing()
{
}

//AUpkeep
AUpkeep::AUpkeep(GameObserver* observer, int _id, MTGCardInstance * card, MTGAbility * a, ManaCost * _cost, int restrictions, int _phase,
        int _once,bool Cumulative) :
    ActivatedAbility(observer, _id, card, _cost, restrictions), NestedAbility(a), phase(_phase), once(_once),Cumulative(Cumulative)
{
    paidThisTurn = 0;
    aType = MTGAbility::UPCOST;
    if(Cumulative)
    {
        backupMana = NEW ManaCost();
        backupMana->copy(this->getCost());
        backupMana->addExtraCosts(this->getCost()->extraCosts);
    }
}

    int AUpkeep::receiveEvent(WEvent * event)
    {
        if (WEventPhaseChange* pe = dynamic_cast<WEventPhaseChange*>(event))
        {
            if (MTG_PHASE_DRAW == pe->to->id)
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
        if (newPhase == MTG_PHASE_BEFORE_BEGIN)
        {
            paidThisTurn = 0;
        }
        else if(newPhase == MTG_PHASE_UPKEEP && Cumulative )
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
            {
                paidThisTurn = 0;
                this->getCost()->copy(backupMana);
                for(int age = 0;age < currentage;age++)
                {
                    this->getCost()->add(backupMana); 
                    this->getCost()->addExtraCosts(backupMana->extraCosts);
                }
            }
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
    a->ability = ability->clone();
    return a;
}

AUpkeep::~AUpkeep()
{
    if(Cumulative)
    {
        SAFE_DELETE(backupMana);
    }
    SAFE_DELETE(ability);
}

//A Phase based Action
APhaseAction::APhaseAction(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * target, string sAbility, int restrictions, int _phase,bool forcedestroy,bool next,bool myturn,bool opponentturn,bool once) :
MTGAbility(observer, _id, card),sAbility(sAbility), phase(_phase),forcedestroy(forcedestroy),next(next),myturn(myturn),opponentturn(opponentturn),once(once)
{
    abilityId = _id;
    abilityOwner = card->controller();
    psMenuText = "";
    AbilityFactory af(game);
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
                bool isTargetable = false;
                
                if(target)
                {
                    _target = static_cast<MTGCardInstance *>(target);
                    isTargetable = (_target && !_target->currentZone && _target != this->source);
                }
                
                if(!sAbility.size() || (!target || isTargetable))
                {
                    this->forceDestroy = 1;
                    return;
                }
                else
                {
                    while(_target && _target->next)
                        _target = _target->next;
                }
                
                AbilityFactory af(game);
                MTGAbility * ability = af.parseMagicLine(sAbility, abilityId, NULL, _target);

                MTGAbility * a = ability->clone();
                a->target = _target;
                a->resolve();
                delete (a);
                delete (ability);
                if(this->oneShot || once)
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
    return a;
}

APhaseAction::~APhaseAction()
{

}

// the main ability
APhaseActionGeneric::APhaseActionGeneric(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * target, string sAbility, int restrictions, int _phase,bool forcedestroy,bool next,bool myturn,bool opponentturn,bool once) :
    InstantAbility(observer, _id, card, target)
{
    MTGCardInstance * _target = target;
    ability = NEW APhaseAction(game, _id, card,_target, sAbility, restrictions, _phase,forcedestroy,next,myturn,opponentturn,once);
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
    return a;
}

APhaseActionGeneric::~APhaseActionGeneric()
{
    SAFE_DELETE(ability);
}

//a blink
ABlink::ABlink(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target,bool blinkueot,bool blinkForSource,bool blinkhand,MTGAbility * stored) :
MTGAbility(observer, _id, card),blinkueot(blinkueot),blinkForSource(blinkForSource),blinkhand(blinkhand),stored(stored)
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

    if ((blinkueot && currentPhase == MTG_PHASE_ENDOFTURN)||(blinkForSource && !source->isInPlay(game)))
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
        Spell * spell = NEW Spell(game, Blinker);
        spell->source->counters->init();
        if(spell->source->hasSubtype(Subtypes::TYPE_AURA) && !blinkhand)
        {
            TargetChooserFactory tcf(game);
            TargetChooser * tc = tcf.createTargetChooser(spell->source->spellTargetType,spell->source);
            if(!tc->validTargetsExist())
            {
                spell->source->owner->game->putInExile(spell->source);
                SAFE_DELETE(spell);
                SAFE_DELETE(tc);
                this->forceDestroy = 1;
                return;
            }

            MTGGameZone * inplay = spell->source->owner->game->inPlay;
            spell->source->target = NULL;
            for(int i = game->getRandomGenerator()->random()%inplay->nb_cards;;i = game->getRandomGenerator()->random()%inplay->nb_cards)
            {
                if(tc->canTarget(inplay->cards[i]) && spell->source->target == NULL)
                {
                    spell->source->target = inplay->cards[i];
                    spell->getNextCardTarget();
                    spell->resolve();

                    SAFE_DELETE(spell);
                    SAFE_DELETE(tc);
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
            Spell * spell = NEW Spell(game, Blinker);
            spell->source->counters->init();
            if(spell->source->hasSubtype(Subtypes::TYPE_AURA) && !blinkhand)
            {
                TargetChooserFactory tcf(game);
                TargetChooser * tc = tcf.createTargetChooser(spell->source->spellTargetType,spell->source);
                if(!tc->validTargetsExist())
                {
                    spell->source->owner->game->putInExile(spell->source);
                    SAFE_DELETE(spell);
                    SAFE_DELETE(tc);
                    this->forceDestroy = 1;
                    return;
                }

                MTGGameZone * inplay = spell->source->owner->game->inPlay;
                spell->source->target = NULL;
                for(int i = game->getRandomGenerator()->random()%inplay->nb_cards;;i = game->getRandomGenerator()->random()%inplay->nb_cards)
                {
                    if(tc->canTarget(inplay->cards[i]) && spell->source->target == NULL)
                    {
                        spell->source->target = inplay->cards[i];
                        spell->getNextCardTarget();
                        spell->resolve();
                        SAFE_DELETE(spell);
                        SAFE_DELETE(tc);
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
            SAFE_DELETE(spell);
            SAFE_DELETE(tc);
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
    a->stored = stored ? stored->clone() : NULL;
    a->forceDestroy = -1;
    return a;
};
ABlink::~ABlink()
{
    SAFE_DELETE(stored);
}

ABlinkGeneric::ABlinkGeneric(GameObserver* observer, int _id, MTGCardInstance * card, MTGCardInstance * _target,bool blinkueot,bool blinkForSource,bool blinkhand,MTGAbility * stored) :
    InstantAbility(observer, _id, source, _target)
{
    ability = NEW ABlink(observer, _id,card,_target,blinkueot,blinkForSource,blinkhand,stored);
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
    return a;
}

ABlinkGeneric::~ABlinkGeneric()
{
    SAFE_DELETE(ability);
}

// target becomes blocked by source
AABlock::AABlock(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
InstantAbility(observer, id, card, target)
{
    target = _target;
}

int AABlock::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    source = (MTGCardInstance*)source;
    if (_target && source->canBlock(_target))
    {
       source->toggleDefenser(_target);
       source->getObserver()->isInterrupting = NULL;
    }
    return 1;
}

AABlock * AABlock::clone() const
{
    return NEW AABlock(*this);
}

// target becomes pair of source
PairCard::PairCard(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
InstantAbility(observer, id, card, target)
{
    target = _target;
    oneShot = true;
    forceDestroy = 1;
}

int PairCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    source = (MTGCardInstance*)source;
    if (_target && !_target->myPair && source)
    {
        source->myPair = _target;
        _target->myPair = source;
    }
    return 1;
}

PairCard * PairCard::clone() const
{
    return NEW PairCard(*this);
}
//target is dredged
dredgeCard::dredgeCard(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
InstantAbility(observer, id, card, target)
{
    target = _target;
    oneShot = true;
    forceDestroy = 1;
}

int dredgeCard::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if(_target)
    {
        for(int j = 0; j < _target->data->dredge();j++)
        {
            _target->controller()->game->putInZone(
                _target->controller()->game->library->cards[_target->controller()->game->library->nb_cards - 1],
                _target->controller()->game->library, _target->controller()->game->graveyard);
        }
        _target->controller()->game->putInZone(_target,_target->currentZone,_target->controller()->game->hand);
    }
    return 1;
}

dredgeCard * dredgeCard::clone() const
{
    return NEW dredgeCard(*this);
}

// target becomes a parent of card(source)
AAConnect::AAConnect(GameObserver* observer, int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost) :
InstantAbility(observer, id, card, target)
{
    target = _target;
}

int AAConnect::resolve()
{
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (_target)
    {
        while (_target->next)
            _target = _target->next;
        _target->childrenCards.push_back(source);
        source->parentCards.push_back(_target);
        //weapon
        if(source->hasSubtype(Subtypes::TYPE_EQUIPMENT))
        {
            for (size_t i = 1; i < game->mLayers->actionLayer()->mObjects.size(); i++)
            {
                MTGAbility * a = ((MTGAbility *) game->mLayers->actionLayer()->mObjects[i]);
                AEquip * eq = dynamic_cast<AEquip*> (a);
                if (eq && eq->source == source)
                {
                    ((AEquip*)a)->unequip();
                    ((AEquip*)a)->equip(_target);
                }
            }
        }
        else
        {
            if(source->target)
                source->target = NULL;
            //clearing the source target allows us to use target= line
            //without creating side effects on any other abilities a card has
            //connect has to be the first ability in the cards lines unless you want it to do effects to the targeted card!!!
        }
    }
    return 1;
}

AAConnect * AAConnect::clone() const
{
    return NEW AAConnect(*this);
}

//Tutorial Messaging

ATutorialMessage::ATutorialMessage(GameObserver* observer, MTGCardInstance * source, string message, int limit)
    : MTGAbility(observer, 0, source), IconButtonsController(observer->getInput(), 0, 0), mLimit(limit)
{
    mBgTex = NULL;

    mElapsed = 0;
    mIsImage = false;

    for (int i = 0; i < 9; i++)
        mBg[i] = NULL;

    if(game->getResourceManager())
    {
        string gfx = game->getResourceManager()->graphicsFile(message);
        if (fileExists(gfx.c_str()))
        {
            mIsImage = true;
            mMessage = message;
        }
        else
        {
            mMessage = _(message); //translate directly here, remove this and translate at rendering time if it bites us
            boost::replace_all(mMessage, "\\n", "\n");
        }
    }

    if (mIsImage)
    {
        mX = SCREEN_WIDTH_F / 2;
        mY = SCREEN_HEIGHT_F / 2;

    }
    else
    {
        mX = 0;
        mY = -SCREEN_HEIGHT_F - 0.1f; //Offscreen
    }
    mDontShow = mUserCloseRequest = (mLimit > 0) && (alreadyShown() >= mLimit);

    if(mDontShow)
        forceDestroy = 1;
}


string ATutorialMessage::getOptionName()
{
    std::stringstream out;
    out << "tuto_";
    out << hash_djb2(mMessage.c_str());
    return out.str();
}

int ATutorialMessage::alreadyShown()
{
    return options[getOptionName()].number;
}

bool ATutorialMessage::CheckUserInput(JButton key)
{
    if (mUserCloseRequest) return false;

    if(key == JGE_BTN_SEC || key == JGE_BTN_OK)
    {
        ButtonPressed(0, 1);
        return true;
    }

    //Required for Mouse/touch input
    IconButtonsController::CheckUserInput(key);

    return true; //this ability is modal, so it catches all key events until it gets closed
}

void ATutorialMessage::Update(float dt)
{
    if (!game->mLayers->stackLayer()->getCurrentTutorial() && !mDontShow)
        game->mLayers->stackLayer()->setCurrentTutorial(this);

    if (game->mLayers->stackLayer()->getCurrentTutorial() != this)
        return;

    if (mUserCloseRequest && mY < -SCREEN_HEIGHT)
        mDontShow = true;

    if (mDontShow)
    {
        game->mLayers->stackLayer()->setCurrentTutorial(0);
        forceDestroy = 1;
        return;
    }

    mElapsed += dt;

    IconButtonsController::Update(dt);

    if (mIsImage)
        return;

    //Below this only affects "text" mode
    if (!mUserCloseRequest && mY < 0)
    {
        mY = -SCREEN_HEIGHT + (SCREEN_HEIGHT * mElapsed / 0.75f); //Todo: more physical drop-in.
        if (mY >= 0)
            mY = 0;
    }
    else if (mUserCloseRequest && mY > -SCREEN_HEIGHT)
    {
        mY = -(SCREEN_HEIGHT * mElapsed / 0.75f);
    }
}

void ATutorialMessage::ButtonPressed(int controllerId, int controlId)
{
    //TODO : cancel ALL tips/tutorials for JGE_BTN_SEC?
    if (mLimit)
    {
        string optionName = getOptionName();
        options[optionName].number = options[optionName].number + 1;
        options.save(); //TODO: if we experience I/O slowness in tutorials, move this save at the end of a turn, or at the end of the game.
    }
    mElapsed = 0;
    mUserCloseRequest = true;
}

void ATutorialMessage::Render()
{
    if (mDontShow)
        return;

    if (mY < -SCREEN_HEIGHT)
        return;

    if (!mBgTex)
    {
        if (mIsImage)
        {
            mBgTex = game->getResourceManager()->RetrieveTexture(mMessage, RETRIEVE_LOCK);
            if (mBgTex)
            {
                mBg[0] = NEW JQuad(mBgTex, 0, 0, (float) mBgTex->mWidth, (float) mBgTex->mHeight);
                mBg[0]->SetHotSpot(mBg[0]->mWidth / 2, mBg[0]->mHeight / 2);

                //Continue Button
                JQuadPtr quad =  game->getResourceManager()->RetrieveQuad("iconspsp.png", 4 * 32, 0, 32, 32, "iconpsp4", RETRIEVE_MANAGE);
                quad->SetHotSpot(16, 16);
                IconButton * iconButton = NEW IconButton(1, this, quad.get(), 0, mBg[0]->mHeight / 2, 0.7f, Fonts::MAGIC_FONT, _("continue"), 0, 16, true);
                Add(iconButton);
            }

            if (options[Options::SFXVOLUME].number > 0)
            {
				game->getResourceManager()->PlaySample("tutorial.wav");
            }
        }
        else
        {
            mBgTex = game->getResourceManager()->RetrieveTexture("taskboard.png", RETRIEVE_LOCK);

            float unitH = static_cast<float> (mBgTex->mHeight / 4);
            float unitW = static_cast<float> (mBgTex->mWidth / 4);
            if (unitH == 0 || unitW == 0) return;

            if (mBgTex)
            {
                mBg[0] = NEW JQuad(mBgTex, 0, 0, unitW, unitH);
                mBg[1] = NEW JQuad(mBgTex, unitW, 0, unitW * 2, unitH);
                mBg[2] = NEW JQuad(mBgTex, unitW * 3, 0, unitW, unitH);
                mBg[3] = NEW JQuad(mBgTex, 0, unitH, unitW, unitH * 2);
                mBg[4] = NEW JQuad(mBgTex, unitW, unitH, unitW * 2, unitH * 2);
                mBg[5] = NEW JQuad(mBgTex, unitW * 3, unitH, unitW, unitH * 2);
                mBg[6] = NEW JQuad(mBgTex, 0, unitH * 3, unitW, unitH);
                mBg[7] = NEW JQuad(mBgTex, unitW, unitH * 3, unitW * 2, unitH);
                mBg[8] = NEW JQuad(mBgTex, unitW * 3, unitH * 3, unitW, unitH);
            }

            //Continue Button
            JQuadPtr quad =  game->getResourceManager()->RetrieveQuad("iconspsp.png", 4 * 32, 0, 32, 32, "iconpsp4", RETRIEVE_MANAGE);
            quad->SetHotSpot(16, 16);
            IconButton * iconButton = NEW IconButton(1, this, quad.get(), SCREEN_WIDTH_F / 2,  SCREEN_HEIGHT_F - 60, 0.7f, Fonts::MAGIC_FONT, _("continue"), 0, 16, true);
            Add(iconButton);

            mSH = 64 / unitH;
            mSW = 64 / unitW;

            if (options[Options::SFXVOLUME].number > 0)
            {
                game->getResourceManager()->PlaySample("chain.wav");
            }
        }
    }

    JRenderer * r = JRenderer::GetInstance();

    //Render background board
    if (mBgTex)
    {
        if (mIsImage)
        {
            int alpha = mUserCloseRequest ? MAX(0, 255 - (int)(mElapsed * 500)) : MIN(255, (int)(mElapsed * 500)) ;
            if (mUserCloseRequest && alpha == 0)
                mDontShow = true;

            r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(alpha / 2,0,0,0));
            mBg[0]->SetColor(ARGB(alpha,255,255,255));
            r->RenderQuad(mBg[0], SCREEN_WIDTH_F /2 , SCREEN_HEIGHT_F / 2 , 0);
            IconButtonsController::SetColor(ARGB(alpha,255,255,255));
        }
        else 
        {
            //Setup fonts.
            WFont * f2 = game->getResourceManager()->GetWFont(Fonts::MAGIC_FONT);
            f2->SetColor(ARGB(255, 205, 237, 240));

            r->FillRect(0, mY, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(128,0,0,0));
            r->RenderQuad(mBg[0], 0, mY, 0, mSW, mSH); //TL
            r->RenderQuad(mBg[2], SCREEN_WIDTH - 64, mY, 0, mSW, mSH); //TR
            r->RenderQuad(mBg[6], 0, mY + SCREEN_HEIGHT - 64, 0, mSW, mSH); //BL
            r->RenderQuad(mBg[8], SCREEN_WIDTH - 64, mY + SCREEN_HEIGHT - 64, 0, mSW, mSH); //BR

            //Stretch the sides
            float stretchV = (144.0f / 128.0f) * mSH;
            float stretchH = (176.0f / 128.0f) * mSW;
            r->RenderQuad(mBg[3], 0, mY + 64, 0, mSW, stretchV); //L
            r->RenderQuad(mBg[5], SCREEN_WIDTH - 64, mY + 64, 0, mSW, stretchV); //R
            r->RenderQuad(mBg[1], 64, mY, 0, stretchH, mSH); //T1
            r->RenderQuad(mBg[1], 240, mY, 0, stretchH, mSH); //T1
            r->RenderQuad(mBg[7], 64, mY + 208, 0, stretchH, mSH); //B1
            r->RenderQuad(mBg[7], 240, mY + 208, 0, stretchH, mSH); //B1
            r->RenderQuad(mBg[4], 64, mY + 64, 0, stretchH, stretchV); //Center1
            r->RenderQuad(mBg[4], 240, mY + 64, 0, stretchH, stretchV); //Center2
        }
    }
    else
    {
        r->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(128,0,0,0));
        r->FillRect(10, 10 + mY, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, ARGB(128,0,0,0));
    }

    if (!mBgTex || !mIsImage)
    {
        float posX = 40, posY = mY + 20;
        string title = _("Help");

        WFont * f = game->getResourceManager()->GetWFont(Fonts::MAGIC_FONT);
        WFont * f3 = game->getResourceManager()->GetWFont(Fonts::MENU_FONT); //OPTION_FONT
        f->SetColor(ARGB(255, 55, 46, 34));
        f3->SetColor(ARGB(255, 219, 206, 151));

        f3->DrawString(title.c_str(), static_cast<float> ((SCREEN_WIDTH - 20) / 2 - title.length() * 4), posY);
        posY += 30;

        f->DrawString(_(mMessage).c_str(), posX, posY);
    
        f->SetScale(1);
    }

    IconButtonsController::Render();

}

ATutorialMessage * ATutorialMessage::clone() const
{
    ATutorialMessage * copy =  NEW ATutorialMessage(*this);
    copy->mUserCloseRequest = (copy->alreadyShown() > 0);
    return copy;
}

ATutorialMessage::~ATutorialMessage()
{
    if (mBgTex)
    {
        game->getResourceManager()->Release(mBgTex);
        for (int i = 0; i < 9; i++)
            SAFE_DELETE(mBg[i]);
    }
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
        for (int colorIndex = Constants::MTG_COLOR_ARTIFACT; colorIndex < Constants::NB_Colors; ++colorIndex)
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
        size_t id = MTGAllCards::findType(subtype);
        if (id != string::npos)
            types.push_back(id);
    }
}
