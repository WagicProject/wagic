#include "PrecompiledHeader.h"

#include "DamagerDamaged.h"

/*
 Temporary objects that store the damages dealt to/from creatures during the combat phase
 */

DamagerDamaged::DamagerDamaged(MTGCardInstance* card, float x, float y, bool show, Player * damageSelecter) :
    TransientCardView(card, x, y), show(show), damageSelecter(damageSelecter)
{
}
DamagerDamaged::DamagerDamaged(MTGCardInstance* card, const Pos& ref, bool show, Player * damageSelecter) :
    TransientCardView(card, ref), show(show), damageSelecter(damageSelecter)
{
}

DamagerDamaged::~DamagerDamaged()
{
}

int DamagerDamaged::sumDamages()
{
    int total = 0;
    for (vector<Damage>::iterator i = damages.begin(); i != damages.end(); ++i)
        total += i->damage;
    return total;
}

bool DamagerDamaged::hasLethalDamage()
{
    return (sumDamages() >= card->life);
}

void DamagerDamaged::addDamage(int damage, DamagerDamaged* source)
{
    for (vector<Damage>::iterator i = damages.begin(); i != damages.end(); ++i)
        if (i->source == source->card)
        {
            i->damage += damage;
            if (0 >= i->damage)
                damages.erase(i);
            return;
        }
    if (0 < damage)
        damages.push_back(Damage(card->getObserver(), source->card, card, damage, Damage::DAMAGE_COMBAT));
    return;
}

int DamagerDamaged::removeDamagesFrom(DamagerDamaged* source)
{
    for (vector<Damage>::iterator i = damages.begin(); i != damages.end(); ++i)
        if (i->source == source->card)
        {
            int damage = i->damage;
            damages.erase(i);
            return damage;
        }
    return 0;
}

void DamagerDamaged::clearDamage()
{
    damages.clear();
}

void DamagerDamaged::Render(CombatStep mode)
{
    TransientCardView::Render();
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MENU_FONT);
    mFont->SetBase(0);

    switch (mode)
    {
    case BLOCKERS:
    case TRIGGERS:
    case ORDER:
		mFont->SetScale(2);
        mFont->SetColor(ARGB(255,255,0,0));
        break;
    case FIRST_STRIKE:
    case END_FIRST_STRIKE:
    case DAMAGE:
    case END_DAMAGE:
		mFont->SetScale(2);
        mFont->SetColor(ARGB(255,255,0,0));
        break;
    }

    char buf[6];
    sprintf(buf, "%i", sumDamages());
    mFont->DrawString(buf, actX - 14 * actZ + 5, actY - 14 * actZ);

}

AttackerDamaged::AttackerDamaged(MTGCardInstance* card, float x, float y, bool show, Player * damageSelecter) :
    DamagerDamaged(card, x, y, show, damageSelecter)
{
}
AttackerDamaged::AttackerDamaged(MTGCardInstance* card, const Pos& ref, bool show, Player * damageSelecter) :
    DamagerDamaged(card, ref, show, damageSelecter)
{
}

AttackerDamaged::~AttackerDamaged()
{
    for (vector<DefenserDamaged*>::iterator q = blockers.begin(); q != blockers.end(); ++q)
        delete (*q);
}
