#include "PrecompiledHeader.h"

#include "Damage.h"
#include "MTGCardInstance.h"
#include "Counters.h"
#include "WEvent.h"
#include "Translate.h"
#include "WResourceManager.h"
#include "GameObserver.h"

Damage::Damage(GameObserver* observer, MTGCardInstance * source, Damageable * target)
    : Interruptible(observer)
{
    init(source, target, source->getPower(), DAMAGE_OTHER);
}

Damage::Damage(GameObserver* observer, MTGCardInstance * source, Damageable * target, int damage, int _typeOfDamage)
    : Interruptible(observer)
{
    init(source, target, damage, _typeOfDamage);
}

void Damage::init(MTGCardInstance * _source, Damageable * _target, int _damage, int _typeOfDamage)
{
    typeOfDamage = _typeOfDamage;
    target = _target;
    source = _source;

    if (_damage < 0)
        _damage = 0; //Negative damages cannot happen
    damage = _damage;
    mHeight = 40;
    type = ACTION_DAMAGE;
}

int Damage::resolve()
{
    if (damage < 0)
        damage = 0; //Negative damages cannot happen
    state = RESOLVED_OK;
    WEvent * e = NEW WEventDamage(this);
    //Replacement Effects
    e = observer->replacementEffects->replace(e);
    if (!e)
        return 0;
    WEventDamage * ev = dynamic_cast<WEventDamage*> (e);
    if (!ev)
    {
        observer->receiveEvent(e);
        return 0;
    }
    damage = ev->damage->damage;
    target = ev->damage->target;
    if (!damage)
        return 0;

    //asorbing effects for cards controller-----------

    //reserved for culmulitive absorb ability coding

    //prevent next damage-----------------------------
    if ((target)->preventable >= 1)
    {
        int preventing = (target)->preventable;
        for (int k = preventing; k > 0; k--)
        {
            //the following keeps preventable from ADDING toughness/life if damage was less then preventable amount.
            for (int i = damage; i >= 1; i--)
            {
                (target)->preventable -= 1;
                damage -= 1;
                break;//does the redux of damage 1 time, breaks the loop to deincrement preventing and start the loop over.
            }
        }
    }

    //set prevent next damage back to 0 if it is equal to less then 0
    if ((target)->preventable < 0)
    {
        (target)->preventable = 0;
    }

    //-------------------------------------------------
    if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
    {
        MTGCardInstance * _target = (MTGCardInstance *) target;
        if ((_target)->protectedAgainst(source))
            damage = 0;
        //rulings = 10/4/2004	The damage prevention ability works even if it has no counters, as long as some effect keeps its toughness above zero.
        //these creature are essentially immune to damage. however 0/-1 effects applied through lords or counters can kill them.
        if ((_target)->has(Constants::PHANTOM))
        {
            damage = 0;
            (_target)->counters->removeCounter(1, 1);
        }
        if ((_target)->has(Constants::ABSORB))
        {
            damage -=  (_target)->basicAbilities[(int)Constants::ABSORB];
            if(damage < 0)
                damage = 0;
        }
        if ((_target)->has(Constants::WILTING))
        {
            for (int j = damage; j > 0; j--)
            {
                (_target)->counters->addCounter(-1, -1);
            }
            damage = 0;
        }
        if ((_target)->has(Constants::VIGOR))
        {
            for (int j = damage; j > 0; j--)
            {
                (_target)->counters->addCounter(1, 1);
            }
            damage = 0;
        }
        if ((_target)->has(Constants::HYDRA))
        {
            for (int j = damage; j > 0; j--)
            {
                (_target)->counters->removeCounter(1, 1);
            }
            damage = 0;
        }
        if (!damage)
        {
            state = RESOLVED_NOK;
            delete (e);
            return 0;
        }
        _target->doDamageTest = 1;
    }

    int a = damage;

    if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE && (source->has(Constants::WITHER) || source->has(
                    Constants::INFECT)))
    {
        // Damage for WITHER or poison on creatures. This should probably go in replacement effects
        MTGCardInstance * _target = (MTGCardInstance *) target;
        for (int i = 0; i < damage; i++)
        {
            _target->counters->addCounter(-1, -1);
        }
        if(_target->toughness <= 0 && _target->has(Constants::INDESTRUCTIBLE))
            _target->controller()->game->putInGraveyard(_target);
    }
    else if (target->type_as_damageable == DAMAGEABLE_PLAYER && source->has(Constants::INFECT))
    {
        // Poison on player
        Player * _target = (Player *) target;
        _target->poisonCount += damage;//this will be changed to poison counters.
        _target->damageCount += damage;
    }
    else if (target->type_as_damageable == DAMAGEABLE_PLAYER && (source->has(Constants::POISONTOXIC) ||
                    source->has(Constants::POISONTWOTOXIC) || source->has(Constants::POISONTHREETOXIC)))
    {
        //Damage + 1, 2, or 3 poison counters on player
        Player * _target = (Player *) target;
        a = target->dealDamage(damage);
        target->damageCount += damage;
        if (source->has(Constants::POISONTOXIC))
        {
            _target->poisonCount += 1;
        }
        else if (source->has(Constants::POISONTWOTOXIC))
        {
            _target->poisonCount += 2;
        }
        else
        {
            _target->poisonCount += 3;
        }

    }
    else
    {
        // "Normal" case,
        //return the left over amount after effects have been applied to them.
        a = target->dealDamage(damage);
        target->damageCount += 1;
        if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
            ((MTGCardInstance*)target)->wasDealtDamage = true;
        if (target->type_as_damageable == DAMAGEABLE_PLAYER)
        {
            if(target == source->controller())
            {
                ((MTGCardInstance*)source)->damageToController = true;
            }
            else
            {
                ((MTGCardInstance*)source)->damageToOpponent = true;
            }
            target->lifeLostThisTurn += damage;
            WEvent * lifed = NEW WEventLife((Player*)target,-damage);
            observer->receiveEvent(lifed);
        }
    }

    //Send (Damage/Replaced effect) event to listeners
    observer->receiveEvent(e);
    return a;
}
void Damage::Render()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetBase(0);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[200];
    sprintf(buffer, _("Deals %i damage to").c_str(), damage);
    mFont->DrawString(buffer, x + 20, y, JGETEXT_LEFT);
    JRenderer * renderer = JRenderer::GetInstance();
    JQuadPtr quad = WResourceManager::Instance()->RetrieveCard(source, CACHE_THUMB);
    if (quad.get())
    {
        float scale = 30 / quad->mHeight;
        renderer->RenderQuad(quad.get(), x, y, 0, scale, scale);
    }
    else
    {
        mFont->DrawString(_(source->getName()).c_str(), x, y - 15);
    }
    quad = target->getIcon();
    if (quad.get())
    {
        float scale = 30 / quad->mHeight;
        renderer->RenderQuad(quad.get(), x + 150, y, 0, scale, scale);
    }
    else
    {
        if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
            mFont->DrawString(_(((MTGCardInstance *) target)->getName()).c_str(), x + 120, y);
    }

}

ostream& Damage::toString(ostream& out) const
{
    out << "Damage ::: target : " << target << " ; damage " << damage;
    return out;
}

DamageStack::DamageStack(GameObserver *observer)
    : GuiLayer(observer), Interruptible(observer)
{
    currentState = -1;
    type = ACTION_DAMAGES;
}

/* Damage Stack resolve process:
 1 - apply damages to targets. For each of them, send an event to the GameObserver (for Damage triggers)
 2 - Once this is done, send a "Damage Stakc Resolved" event to the GameObserver
 3 - Once that message is received on the DamageStack's side, do the "afterDamage" effects (send to graveyard, etc...)
 Using events in 2 and 3 guarantees that the "send to graveyard" effect will only apply AFTER Damaged triggers are applied
 */
int DamageStack::resolve()
{
    for (int i = (int)(mObjects.size()) - 1; i >= 0; i--)
    {
        Damage * damage = (Damage*) mObjects[i];
        if (damage->state == NOT_RESOLVED)
            damage->resolve();
    }
    ((Interruptible*)this)->getObserver()->receiveEvent(NEW WEventDamageStackResolved());
    return 1;
}

int DamageStack::receiveEvent(WEvent * e)
{
    WEventDamageStackResolved *event = dynamic_cast<WEventDamageStackResolved*> (e);
    if (!event)
        return 0;

    for (int i = (int)(mObjects.size()) - 1; i >= 0; i--)
    {
        Damage * damage = (Damage*) mObjects[i];
        if (damage->state == RESOLVED_OK)
            damage->target->afterDamage();
    }
    return 1;
}

void DamageStack::Render()
{
    float currenty = y;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        Damage * damage = (Damage*) mObjects[i];
        if (damage->state == NOT_RESOLVED)
        {
            damage->x = x;
            damage->y = currenty;
            currenty += damage->mHeight;
            damage->Render();
        }
    }
}

ostream& DamageStack::toString(ostream& out) const
{
    return (out << "DamageStack ::: currentState : " << currentState);
}


ostream& operator<<(ostream& out, const Damageable& p)
{
    out << "life=" << p.life << endl;
    out << "poisoncount=" << p.poisonCount << endl;
    out << "damagecount=" << p.damageCount << endl;
    out << "preventable=" << p.preventable << endl;
    return out;
}


bool Damageable::parseLine(const string& s)
{
    size_t limiter = s.find("=");
    if (limiter == string::npos) limiter = s.find(":");
    string areaS;
    if (limiter != string::npos)
    {
        areaS = s.substr(0, limiter);
        if (areaS.compare("life") == 0)
        {
            life = atoi((s.substr(limiter + 1)).c_str());
        }
        else if (areaS.compare("poisoncount") == 0)
        {
            poisonCount = atoi((s.substr(limiter + 1)).c_str());
        }
        else if (areaS.compare("damagecount") == 0)
        {
            damageCount = atoi((s.substr(limiter + 1)).c_str());
        }
        else if (areaS.compare("preventable") == 0)
        {
            preventable = atoi((s.substr(limiter + 1)).c_str());
        }
        else
        {
            return false;
        }
    }

    return true;
}

