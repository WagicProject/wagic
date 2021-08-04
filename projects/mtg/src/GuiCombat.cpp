#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiCombat.h"
#include "AIPlayer.h"
#include "GameObserver.h"
#include "Trash.h"
#include "CardSelector.h"
#include "Closest.cpp"

static const float MARGIN = 70;
static const float TOP_LINE = 80;

const float kZoom_none = 1.0f;
const float kZoom_level1 = 1.4f;
const float kZoom_level2 = 2.2f;
const float kZoom_level3 = 2.7f;

struct GuiCombatTrue: public Exp
{
    static inline bool test(DamagerDamaged*, DamagerDamaged*)
    {
        return true;
    }
};
struct GuiCombatLeft: public Exp
{
    static inline bool test(DamagerDamaged* ref, DamagerDamaged* test)
    {
        return ref->y == test->y && ref->x > test->x && test->show;
    }
};
struct GuiCombatRight: public Exp
{
    static inline bool test(DamagerDamaged* ref, DamagerDamaged* test)
    {
        return ref->y == test->y && ref->x < test->x && test->show;
    }
};

JTexture* GuiCombat::ok_tex = NULL;

GuiCombat::GuiCombat(GameObserver* go) :
    GuiLayer(go), active(NULL), activeAtk(NULL), ok(SCREEN_WIDTH - MARGIN, 210, 1, 0, 255), enemy_avatar(SCREEN_WIDTH
                    - MARGIN, TOP_LINE, 2, 0, 255), cursor_pos(NONE), step(DAMAGE)
{
    if (NULL == ok_tex && go->getResourceManager())
    {
        ok_tex = go->getResourceManager()->RetrieveTexture("Ok.png", RETRIEVE_LOCK);
        // send a message out to listeners that we created the GO button and it's location
    }
}

GuiCombat::~GuiCombat()
{
    if (ok_tex && observer->getResourceManager())
    {
        observer->getResourceManager()->Release(ok_tex);
        ok_tex = NULL;
    }

    for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
        delete (*it);
}

template<typename T>
static inline void repos(typename vector<T*>::iterator begin, typename vector<T*>::iterator end, signed size = -1)
{
    for (typename vector<T*>::iterator it = begin; it != end; ++it)
        if ((*it)->show)
            ++size;
    float space = (SCREEN_WIDTH - 2 * MARGIN) / size;
    float pos = MARGIN;
    for (typename vector<T*>::iterator it = begin; it != end; ++it)
        if ((*it)->show)
        {
            (*it)->x = pos;
            pos += space;
        }
}

void GuiCombat::Update(float dt)
{
    for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
        (*it)->Update(dt);
    if (activeAtk)
        for (vector<DefenserDamaged*>::iterator q = activeAtk->blockers.begin(); q != activeAtk->blockers.end(); ++q)
            (*q)->Update(dt);
    ok.Update(dt);
    enemy_avatar.Update(dt);
}

void GuiCombat::remaskBlkViews(AttackerDamaged*, AttackerDamaged* after)
{
    if (after)
    {
        for (vector<DefenserDamaged*>::iterator q = after->blockers.begin(); q != after->blockers.end(); ++q)
        {
            (*q)->actX = MARGIN;
            (*q)->y = TOP_LINE;
            (*q)->zoom = kZoom_level2;
            (*q)->t = 0;
        }
        repos<DefenserDamaged> (after->blockers.begin(), after->blockers.end(), after->card->has(Constants::TRAMPLE) ? 0 : -1);
        enemy_avatar.actX = MARGIN;
        enemy_avatar.x = SCREEN_WIDTH - MARGIN;
    }
}

void GuiCombat::validateDamage()
{
    switch (step)
    {
    case FIRST_STRIKE:
        observer->nextCombatStep();
        break;
    case DAMAGE:
        observer->userRequestNextGamePhase(false, false);
        break;
    default:
        cout << "COMBAT : Cannot validate damage in this phase" << endl;
        break;
    }
}

void GuiCombat::autoaffectDamage(AttackerDamaged* attacker, CombatStep step)
{
    attacker->clearDamage();
    unsigned damage = attacker->card->stepPower(step);
    for (vector<DefenserDamaged*>::iterator it = attacker->blockers.begin(); it != attacker->blockers.end(); ++it)
    {
        (*it)->clearDamage();
        unsigned actual_damage = MIN(damage, (unsigned)MAX((*it)->card->toughness, 0));
        if ((attacker->card->has(Constants::DEATHTOUCH) || attacker->card->has(Constants::PERPETUALDEATHTOUCH)) && actual_damage > 1)
            actual_damage = 1;
        (*it)->addDamage(actual_damage, attacker);
        attacker->addDamage((*it)->card->stepPower(step), *it);
        damage -= actual_damage;
    }
    if (damage > 0 && attacker->blockers.size() > 0 && !attacker->card->has(Constants::TRAMPLE))
        attacker->blockers[0]->addDamage(damage, attacker);
}

void GuiCombat::addOne(DefenserDamaged* blocker, CombatStep step)
{
    blocker->addDamage(1, activeAtk);
    signed damage = activeAtk->card->stepPower(step);
    for (vector<DamagerDamaged*>::iterator it = activeAtk->blockers.begin(); it != activeAtk->blockers.end(); ++it)
    {
        damage -= (*it)->sumDamages();
        if (0 > damage)
        {
            (*it)->addDamage(-1, activeAtk);
            break;
        }
    }
}
void GuiCombat::removeOne(DefenserDamaged* blocker, CombatStep)
{
    blocker->addDamage(-1, activeAtk);
    for (vector<DamagerDamaged*>::iterator it = activeAtk->blockers.begin(); it != activeAtk->blockers.end(); ++it)
        if ((activeAtk->card->has(Constants::DEATHTOUCH) || activeAtk->card->has(Constants::PERPETUALDEATHTOUCH)) ? ((*it)->sumDamages() < 1) : (!(*it)->hasLethalDamage()))
        {
            (*it)->addDamage(1, activeAtk);
            return;
        }
    if (!activeAtk->card->has(Constants::TRAMPLE) && activeAtk->blockers.size() > 0)
        activeAtk->blockers.back()->addDamage(1, activeAtk);
}

bool GuiCombat::clickOK()
{
    observer->logAction(observer->currentlyActing(), "combatok");
    active = activeAtk = NULL;
    cursor_pos = NONE;
    switch (step)
    {
    case BLOCKERS:
    case TRIGGERS:
        assert(false);//this is an assert for "do i show the screen that lets you select multiple blocker damage assignment.
        return false; // that should not happen

    case ORDER:
        observer->userRequestNextGamePhase(true, false);
        return true;
    case FIRST_STRIKE:
        return false;
    case DAMAGE:
        validateDamage();
        return true;
    case END_FIRST_STRIKE:
        return false;
    case END_DAMAGE:
        JGE::GetInstance()->SendCommand("combatGuiEndDamage");
        return false; // nothing;
    }
    return false;
}


void GuiCombat::shiftLeft()
{
    switch (cursor_pos)
    {
        case NONE:
            break;
        case OK:
            for (vector<AttackerDamaged*>::reverse_iterator it = attackers.rbegin(); it != attackers.rend(); ++it)
                if ((*it)->show)
                {
                    active = *it;
                    break;
                }
            activeAtk = static_cast<AttackerDamaged*> (active);
            cursor_pos = ATK;
            break;
        case ATK:
        {
            DamagerDamaged* old = active;
            active = closest<GuiCombatLeft> (attackers, NULL, static_cast<AttackerDamaged*> (active));
            activeAtk = static_cast<AttackerDamaged*> (active);
            if (old != active)
            {
                if (old)
                    old->zoom = kZoom_none;
                if (active)
                    active->zoom = kZoom_level1;
            }
        }
            break;
        case BLK:
        {
            DamagerDamaged* old = active;
            active = closest<GuiCombatLeft> (activeAtk->blockers, NULL, static_cast<DefenserDamaged*> (active));
            if (old != active)
            {
                if (old)
                    old->zoom = kZoom_none;
                if (active)
                    active->zoom = kZoom_level1;
            }
        }
            break;
    }
}


void GuiCombat::shiftRight( DamagerDamaged* oldActive )
{
    switch (cursor_pos)
    {
        case NONE:
        case OK:
            break;
        case BLK:
        {
            DamagerDamaged* old = active;
            active = closest<GuiCombatRight> (activeAtk->blockers, NULL, static_cast<DefenserDamaged*> (active));
            if (old != active)
            {
                if (old)
                    old->zoom = kZoom_none;
                if (active)
                    active->zoom = kZoom_level1;
            }
        }
            break;
        case ATK:
        {
            DamagerDamaged* old = active;
            active = closest<GuiCombatRight> (attackers, NULL, static_cast<AttackerDamaged*> (active));
            if (active == oldActive)
            {
                active = activeAtk = NULL;
                cursor_pos = OK;
            }
            else
            {
                if (old != active)
                {
                    if (old)
                        old->zoom = kZoom_none;
                    if (active)
                        active->zoom = kZoom_level1;
                }
                activeAtk = static_cast<AttackerDamaged*> (active);
            }
        }
            break;
    }
}

bool GuiCombat::didClickOnButton( Pos buttonPosition, int& x, int& y)
{
    int x1 = static_cast<int>(buttonPosition.x - MARGIN);
    int y1 = static_cast<int>(buttonPosition.y);
    int x2 = static_cast<int>(buttonPosition.x + buttonPosition.width);
    int y2 = static_cast<int>(buttonPosition.y + buttonPosition.height);
    if ( (x >= x1 && x < x2) && (y >= y1 && y < y2))
        return true;
    return false;
}

bool GuiCombat::CheckUserInput(JButton key)
{
    if (NONE == cursor_pos)
        return false;
    DamagerDamaged* oldActive = active;
    
    int x,y;
    if(observer->getInput()->GetLeftClickCoordinates(x, y))
    {
        // determine if OK button was clicked on
        if (ok.width)
        {
            if (didClickOnButton(ok, x, y))
            {
                cursor_pos = OK;
            }
        }
        // position selected card
        if (BLK == cursor_pos)
        {
            DamagerDamaged* selectedCard = closest<GuiCombatTrue> (activeAtk->blockers, NULL, static_cast<float> (x), static_cast<float> (y));
            // find the index into the vector where the current selected card is.
            int c1 = 0, c2 = 0;
            int i = 0;
            for ( vector<DamagerDamaged*>::iterator it = activeAtk->blockers.begin(); it != activeAtk->blockers.end(); ++it)
            {
                if ( *it == selectedCard )
                    c2 = i;
                else if ( *it == active)
                    c1 = i;
                i++;
            }
            // simulate pressing the "Left/Right D-Pad" control c1 - c2 times
            if ( c1 > c2 ) // card selected is to the left of the current active card
            {
                for (int x = 0; x < c1 - c2; x++)
                    shiftLeft();
            }
            else if ( c1 < c2 )
            {
                for (int x = 0; x < c2 - c1; x++)
                    shiftRight( oldActive );
            }
        }
    }
    
    switch (key)
    {
        case JGE_BTN_OK:
            if (BLK == cursor_pos)
            {
                if (ORDER == step)
                    observer->cardClick(active->card); //  { activeAtk->card->raiseBlockerRankOrder(active->card); }
                else
                {
                    signed damage = activeAtk->card->stepPower(step);
                    for (vector<DamagerDamaged*>::iterator it = activeAtk->blockers.begin(); *it != active; ++it)
                        damage -= (*it)->sumDamages();
                    signed now = active->sumDamages();
                    damage -= now;
                    if (damage > 0)
                        addOne(active, step);
                    else if (activeAtk->card->has(Constants::DEATHTOUCH) || activeAtk->card->has(Constants::PERPETUALDEATHTOUCH))
                        for (; now >= 1; --now)
                            removeOne(active, step);
                    else
                        for (now -= active->card->toughness; now >= 0; --now)
                            removeOne(active, step);
                }
            }
            else if (ATK == cursor_pos)
            {
                active = activeAtk->blockers.front();
                active->zoom = kZoom_level3;
                cursor_pos = BLK;
            }
            else if (OK == cursor_pos)
            {
                clickOK();
            }
            break;
        case JGE_BTN_CANCEL:
            if (BLK == cursor_pos)
            {
                oldActive->zoom = kZoom_level2;
                active = activeAtk;
                cursor_pos = ATK;
            }
            return true;
        case JGE_BTN_LEFT:
            shiftLeft();
            break;
            
        case JGE_BTN_RIGHT:
            shiftRight( oldActive );
            break;
            
        case JGE_BTN_DOWN:
            if (ORDER == step || BLK != cursor_pos || active->sumDamages() <= 0)
                break;
            removeOne(active, step);
            break;
        case JGE_BTN_UP:
            if (ORDER == step || BLK != cursor_pos)
                break;
            addOne(active, step);
            break;
        case JGE_BTN_PRI:
            active = activeAtk = NULL;
            cursor_pos = OK;
            break;
        case JGE_BTN_NEXT:
            if (!options[Options::REVERSETRIGGERS].number)
                return false;
            active = activeAtk = NULL;
            cursor_pos = OK;
            break;
        case JGE_BTN_PREV:
            if (options[Options::REVERSETRIGGERS].number)
                return false;
            active = activeAtk = NULL;
            cursor_pos = OK;
            break;
        default:
            ;
    }
    if (oldActive != active)
    {
        if (oldActive && oldActive != activeAtk)
            oldActive->zoom = kZoom_level2;
        if (active)
            active->zoom = kZoom_level3;
        if (ATK == cursor_pos)
            remaskBlkViews(dynamic_cast<AttackerDamaged*> (oldActive), static_cast<AttackerDamaged*> (active));
    }
    if (OK == cursor_pos)
        ok.zoom = 1.5;
    else
        ok.zoom = kZoom_none;
    return true;
}

void GuiCombat::Render()
{
    if (NONE == cursor_pos)
        return;
    JRenderer* renderer = JRenderer::GetInstance();
    renderer->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(200,0,0,0));

    for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
        if ((*it)->show)
            (*it)->Render(step);
    if (activeAtk)
    {
        float setH = 0;
        float setW = 0;
        signed damage = activeAtk->card->stepPower(step);
        for (vector<DefenserDamaged*>::iterator q = activeAtk->blockers.begin(); q != activeAtk->blockers.end(); ++q)
        {
            (*q)->Render(step);
            damage -= (*q)->sumDamages();
            setH = (*q)->Height;
            setW = (*q)->Width;
        }
        if (damage < 0)
            damage = 0;
        if (activeAtk->card->has(Constants::TRAMPLE))
        {
            if(activeAtk->card->isAttacking && activeAtk->card->isAttacking != observer->opponent())
            {
                JQuadPtr enemy = WResourceManager::Instance()->RetrieveCard((MTGCardInstance*)activeAtk->card->isAttacking, CACHE_THUMB);
                float oldH = enemy->mHeight;
                float oldW = enemy->mWidth;
                enemy->mHeight = setH;
                enemy->mWidth = setW;
                enemy->SetHotSpot(18, 25);
                enemy_avatar.Render(enemy.get());
                enemy->mHeight = oldH;
                enemy->mWidth = oldW;
            }
            else
            {
                observer->opponent()->getIcon()->mHeight = 50.f;
                observer->opponent()->getIcon()->mWidth = 35.f;
                observer->opponent()->getIcon()->SetHotSpot(18, 25);
                enemy_avatar.Render(observer->opponent()->getIcon().get());
            }
            WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
            mFont->SetColor(ARGB(255, 255, 64, 0));
            {
                char buf[10];
                sprintf(buf, "%i", damage);
                mFont->DrawString(buf, enemy_avatar.actX - 25, enemy_avatar.actY - 40);
            }

        }
    }
    if (ok_tex)
    {
        JQuadPtr ok_quad = WResourceManager::Instance()->RetrieveQuad("Ok.png", 0.0f, 0.0f, ok_tex->mWidth - 4.5f, ok_tex->mHeight - 4.5f);
        ok_quad->mWidth = 56.f;
        ok_quad->mHeight = 45.f;
        ok_quad->SetHotSpot(ok_quad->mWidth/2, ok_quad->mHeight/2);
        ok.Render(ok_quad.get());
    }
    renderer->DrawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, ARGB(255, 255, 64, 0));
    if (FIRST_STRIKE == step)
    {
        WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
        mFont->SetColor(ARGB(255, 64, 255, 64));
        mFont->DrawString("First strike damage", 370, 2);
    }
}

int GuiCombat::resolve() // Returns the number of damage objects dealt this turn.
{
    DamageStack* stack = NEW DamageStack(observer);
    for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
    {
        MTGCardInstance * attacker = (*it)->card;
        signed dmg = attacker->stepPower(step);
        for (vector<DefenserDamaged*>::iterator q = (*it)->blockers.begin(); q != (*it)->blockers.end(); ++q)
        {
            for (vector<Damage>::iterator d = (*q)->damages.begin(); d != (*q)->damages.end(); ++d)
                stack->Add(NEW Damage(*d));
            dmg -= (*q)->sumDamages();
        }

        if (dmg > 0 && ((!attacker->isBlocked()) || attacker->has(Constants::TRAMPLE)))
            stack->Add(NEW Damage(observer, (*it)->card, (Damageable*)attacker->isAttacking?(Damageable*)attacker->isAttacking:observer->opponent(), dmg, Damage::DAMAGE_COMBAT));

        for (vector<Damage>::iterator d = (*it)->damages.begin(); d != (*it)->damages.end(); ++d)
            stack->Add(NEW Damage(*d));
    }
    int v = stack->mObjects.size();
    if (v > 0)
    {
        observer->mLayers->stackLayer()->Add(stack);
        observer->mLayers->stackLayer()->resolve(); // This will delete the damage stack which will in turn delete the Damage it contains
    }
    else
        SAFE_DELETE(stack);
    return v;
}

int GuiCombat::receiveEventPlus(WEvent* e)
{
    if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
        if (NULL == event->after)
            return 0;
        Pos pos(0, 0, 0, 0, 255);
        if (event->card->view != NULL)
            pos = *event->card->view;

        AttackerDamaged* t = NEW AttackerDamaged(event->card, pos, true, NULL);
        attackers.push_back(t);
        return 1;
    }
    else if (WEventCreatureBlocker* event = dynamic_cast<WEventCreatureBlocker*>(e))
    {
        for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
            if ((*it)->card == event->after)
            {
                Pos pos(0, 0, 0, 0, 255);
                if (event->card->view != NULL)
                    pos = *event->card->view;

                DefenserDamaged* t = NEW DefenserDamaged(event->card, pos, true, NULL);
                t->y = t->actY = TOP_LINE;
                t->actT = t->t = 0;
                t->actZ = t->zoom = kZoom_level2;
                (*it)->blockers.push_back(t);
                return 1;
            }
        return 0;
    }
    else if (WEventCreatureBlockerRank* event = dynamic_cast<WEventCreatureBlockerRank*>(e))
    {
        for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
            if ((*it)->card == event->attacker)
            {
                vector<DefenserDamaged*>::iterator it1, it2;
                for (it1 = (*it)->blockers.begin(); it1 != (*it)->blockers.end(); ++it1)
                    if ((*it1)->card == event->card)
                        break;
                if ((*it)->blockers.end() == it1)
                    return 1;
                for (it2 = (*it)->blockers.begin(); it2 != (*it)->blockers.end(); ++it2)
                    if ((*it2)->card == event->exchangeWith)
                        break;
                if ((*it)->blockers.end() == it2)
                    return 1;
                float x = (*it1)->x;
                (*it1)->x = (*it2)->x;
                (*it2)->x = x;
                std::iter_swap(it1, it2);
                autoaffectDamage(*it, DAMAGE);
            }
        return 1;
    }
    return 0;
}
int GuiCombat::receiveEventMinus(WEvent* e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
        if (observer->players[0]->game->inPlay == event->from || observer->players[1]->game->inPlay == event->from)
        {
            for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
                if ((*it)->card == event->card->previous || (*it)->card == event->card)
                {
                    AttackerDamaged* d = *it;
                    if (activeAtk == *it)
                        activeAtk = NULL;
                    attackers.erase(it);
                    observer->mTrash->trash(d);
                    return 1;
                }
                else
                    for (vector<DefenserDamaged*>::iterator q = (*it)->blockers.begin(); q != (*it)->blockers.end(); ++q)
                        if ((*q)->card == event->card->previous || (*q)->card == event->card)
                        {
                            DefenserDamaged* d = *q;
                            (*it)->blockers.erase(q);
                            observer->mTrash->trash(d);
                            return 1;
                        }
            return 0;
        }
    if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
        if (NULL == event->before)
            return 0;
        for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
            if ((*it)->card == event->card)
            {
                AttackerDamaged* d = *it;
                attackers.erase(it);
                observer->mTrash->trash(d);
                return 1;
            }
        return 0;
    }
    else if (WEventCreatureBlocker* event = dynamic_cast<WEventCreatureBlocker*>(e))
    {
        for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
            if ((*it)->card == event->before)
                for (vector<DefenserDamaged*>::iterator q = (*it)->blockers.begin(); q != (*it)->blockers.end(); ++q)
                    if ((*q)->card == event->card)
                    {
                        DefenserDamaged* d = *q;
                        (*it)->blockers.erase(q);
                        observer->mTrash->trash(d);
                        return 1;
                    }
        return 0;
    }
    else if (WEventPhaseChange* event = dynamic_cast<WEventPhaseChange*>(e))
    {
        if (MTG_PHASE_COMBATDAMAGE == event->to->id)
            step = BLOCKERS;
        else
            cursor_pos = NONE;
    }
    else if (WEventCombatStepChange* event = dynamic_cast<WEventCombatStepChange*>(e))
        switch (event->step)
        {
        case BLOCKERS:
            break;

        case TRIGGERS:
            step = TRIGGERS;
            return 1;

        case ORDER:
        {
            if (ORDER == step)
                return 0; // Why do I take this twice ? >.>
            if (!observer->currentPlayer->displayStack())
            {
                observer->nextCombatStep();
                return 1;
            }
            for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
            {
                (*it)->show = (1 < (*it)->blockers.size());
                autoaffectDamage(*it, DAMAGE);
            }
            active = activeAtk = NULL;
            for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
                if ((*it)->show)
                {
                    (*it)->y = 210;
                    (*it)->zoom = kZoom_level2;
                    (*it)->t = 0;
                    if (!active)
                        active = *it;
                }
            repos<AttackerDamaged> (attackers.begin(), attackers.end(), 0);
            if (active)
            {
                active->zoom = kZoom_level3;
                activeAtk = static_cast<AttackerDamaged*> (active);
                remaskBlkViews(NULL, static_cast<AttackerDamaged*> (active));
                cursor_pos = ATK;
                step = ORDER;
            }
            else
                observer->userRequestNextGamePhase(false, false);
            return 1;
        }
        case FIRST_STRIKE:
            step = FIRST_STRIKE;
            for (inner_iterator attacker = attackers.begin(); attacker != attackers.end(); ++attacker)
                if ((*attacker)->card->has(Constants::FIRSTSTRIKE) || (*attacker)->card->has(Constants::DOUBLESTRIKE))
                    goto DAMAGE;
            observer->nextCombatStep();
            break;
        case END_FIRST_STRIKE:
            step = END_FIRST_STRIKE;
            for (inner_iterator attacker = attackers.begin(); attacker != attackers.end(); ++attacker)
                autoaffectDamage(*attacker, FIRST_STRIKE);
            if (0 == resolve())
                observer->nextCombatStep();
            //else go->mLayers->stackLayer()->AddNextGamePhase(); //uncomment to add "interrupt" offer after first strike, rather than giving priority to current player
            return 1;
        case DAMAGE:
            DAMAGE: step = event->step;
            if (!observer->currentPlayer->displayStack())
            {
                ((AIPlayer *) observer->currentPlayer)->affectCombatDamages(step);
                observer->userRequestNextGamePhase(false, false);
                return 1;
            }
            for (inner_iterator attacker = attackers.begin(); attacker != attackers.end(); ++attacker)
                autoaffectDamage(*attacker, step);
            for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
                (*it)->show = ((*it)->card->has(Constants::DOUBLESTRIKE) 
                                || ((*it)->card->has(Constants::FIRSTSTRIKE) ^ (DAMAGE == step))) 
                                && (((*it)->card->has(Constants::TRAMPLE) ? (size_t) 0 : (size_t) 1) < (*it)->blockers.size());
            repos<AttackerDamaged> (attackers.begin(), attackers.end(), 0);
            active = activeAtk = NULL;
            for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
                if ((*it)->show)
                {
                    active = *it;
                    break;
                }
            if (active)
            {
                active->zoom = kZoom_level3;
                activeAtk = static_cast<AttackerDamaged*> (active);
                remaskBlkViews(NULL, static_cast<AttackerDamaged*> (active));
                cursor_pos = ATK;
            }
            else
                observer->nextCombatStep();
            return 1;
        case END_DAMAGE:
            step = END_DAMAGE;
            if (0 == resolve())
                observer->userRequestNextGamePhase(false, false);
            return 1;
        }
    return 0;
}
