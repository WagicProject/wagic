#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiCombat.h"
#include "Closest.cpp"

static const float MARGIN = 70;
static const float TOP_LINE = 80;

struct Left : public Exp { static inline bool test(DamagerDamaged* ref, DamagerDamaged* test)
  { return ref->y == test->y && ref->x > test->x && test->show; } };
struct Right : public Exp { static inline bool test(DamagerDamaged* ref, DamagerDamaged* test)
  { return ref->y == test->y && ref->x < test->x && test->show; } };

JQuad* GuiCombat::ok_quad = NULL;

GuiCombat::GuiCombat(GameObserver* go) : GuiLayer(), go(go), active(false), activeAtk(NULL), ok(SCREEN_WIDTH - MARGIN, 210, 1, 0, 255), cursor_pos(NONE), step(DAMAGE)
{
  if (NULL == ok_quad)
    {
      if (!GameApp::CommonRes->GetTexture("Ok.png"))
        {
          GameApp::CommonRes->CreateTexture("Ok.png");
          GameApp::CommonRes->CreateQuad("OK", "Ok.png", 0, 0, 56, 45);
        }
      ok_quad = GameApp::CommonRes->GetQuad("OK");
      if (ok_quad) ok_quad->SetHotSpot(28, 22);
    }
}

GuiCombat::~GuiCombat()
{
  for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
    delete (*it);
}

template <typename T>
static inline void repos(typename vector<T*>::iterator begin, typename vector<T*>::iterator end, signed size = -1)
{
  for (typename vector<T*>::iterator it = begin; it != end; ++it)
    if ((*it)->show) ++size;
  float space = (SCREEN_WIDTH - 2*MARGIN) / size;
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
}

void GuiCombat::remaskBlkViews(AttackerDamaged* before, AttackerDamaged* after)
{
  if (after)
    {
      for (vector<DefenserDamaged*>::iterator q = after->blockers.begin(); q != after->blockers.end(); ++q)
        {
          (*q)->actX = MARGIN; (*q)->y = TOP_LINE;
          (*q)->zoom = 2.2; (*q)->t = 0;
        }
      repos<DefenserDamaged>(after->blockers.begin(), after->blockers.end());
    }
}

void GuiCombat::reaffectDamage(AttackerDamaged* attacker, CombatStep step)
{
  attacker->clearDamage();
  unsigned damage = attacker->card->stepPower(step);
  for (vector<DefenserDamaged*>::iterator it = attacker->blockers.begin(); it != attacker->blockers.end(); ++it)
    {
      (*it)->clearDamage();
      unsigned actual_damage = MIN(damage, (unsigned)MAX((*it)->card->toughness, 0));
      (*it)->addDamage(actual_damage, attacker);
      attacker->addDamage((*it)->card->stepPower(step), *it);
      damage -= actual_damage;
    }
}

void GuiCombat::addOne(DefenserDamaged* blocker, CombatStep step)
{
  blocker->addDamage(1, activeAtk);
  signed damage = activeAtk->card->stepPower(step);
  for (vector<DamagerDamaged*>::iterator it = activeAtk->blockers.begin(); it != activeAtk->blockers.end(); ++it)
    {
      damage -= (*it)->sumDamages();
      if (0 > damage) { (*it)->addDamage(-1, activeAtk); break; }
    }
}
void GuiCombat::removeOne(DefenserDamaged* blocker, CombatStep step)
{
  blocker->addDamage(-1, activeAtk);
  for (vector<DamagerDamaged*>::iterator it = activeAtk->blockers.begin(); it != activeAtk->blockers.end(); ++it)
    if (!(*it)->hasLethalDamage()) { (*it)->addDamage(1, activeAtk); break; }
}

bool GuiCombat::CheckUserInput(u32 key)
{
  if (NONE == cursor_pos) return false;
  DamagerDamaged* oldActive = active;
  switch (key)
    {
    case PSP_CTRL_CIRCLE:
      if (BLK == cursor_pos)
        if (ORDER == step) { activeAtk->card->raiseBlockerRankOrder(active->card); }
        else
          {
            signed damage = activeAtk->card->stepPower(step);
            for (vector<DamagerDamaged*>::iterator it = activeAtk->blockers.begin(); *it != active; ++it)
              damage -= (*it)->sumDamages();
            signed now = active->sumDamages();
            damage -= now;
            if (damage > 0) addOne(active, step);
            else
              for (now -= active->card->toughness; now >= 0; --now) removeOne(active, step);
          }
      else if (ATK == cursor_pos)
        {
          active = activeAtk->blockers.front();
          active->zoom = 2.7;
          cursor_pos = BLK;
        }
      else if (OK == cursor_pos)
        switch (step)
        {
        case ORDER        : go->receiveEvent(NEW WEventCombatStepChange(FIRST_STRIKE)); break;
        case FIRST_STRIKE : go->receiveEvent(NEW WEventCombatStepChange(DAMAGE)); break;
        case DAMAGE       : cursor_pos = NONE; go->userRequestNextGamePhase(); break;
        }
      break;
    case PSP_CTRL_TRIANGLE:
      if (BLK == cursor_pos)
        {
          oldActive->zoom = 2.2;
          active = activeAtk;
          cursor_pos = ATK;
        }
      return true;
    case PSP_CTRL_LEFT:
      switch (cursor_pos)
	{
	case NONE : break;
	case OK   :
          for (vector<AttackerDamaged*>::reverse_iterator it = attackers.rbegin(); it != attackers.rend(); ++it) if ((*it)->show) { active = *it; break; }
          activeAtk = static_cast<AttackerDamaged*>(active);
          cursor_pos = ATK;
          break;
	case ATK  : active = closest<Left>(attackers, NULL, static_cast<AttackerDamaged*>(active)); activeAtk = static_cast<AttackerDamaged*>(active); break;
	case BLK  : active = closest<Left>(activeAtk->blockers, NULL, static_cast<DefenserDamaged*>(active)); break;
	}
      break;
    case PSP_CTRL_RIGHT:
      switch (cursor_pos)
	{
	case NONE :
	case OK   : break;
	case BLK  : active = closest<Right>(activeAtk->blockers, NULL, static_cast<DefenserDamaged*>(active)); break;
	case ATK  : active = closest<Right>(attackers, NULL, static_cast<AttackerDamaged*>(active));
	  if (active == oldActive) { active = activeAtk = NULL; cursor_pos = OK; }
          else activeAtk = static_cast<AttackerDamaged*>(active);
	  break;
	}
      break;
    case PSP_CTRL_DOWN:
      if (ORDER == step || BLK != cursor_pos || active->sumDamages() <= 0) break;
      removeOne(active, step);
      break;
    case PSP_CTRL_UP:
      if (ORDER == step || BLK != cursor_pos) break;
      addOne(active, step);
      break;
    case PSP_CTRL_SQUARE:
    case PSP_CTRL_RTRIGGER:
      active = activeAtk = NULL; cursor_pos = OK;
      break;
    }
  if (oldActive != active)
    {
      if (oldActive && oldActive != activeAtk) oldActive->zoom = 2.2;
      if (active) active->zoom = 2.7;
      if (ATK == cursor_pos) remaskBlkViews(dynamic_cast<AttackerDamaged*>(oldActive), static_cast<AttackerDamaged*>(active));
    }
  if (OK == cursor_pos) ok.zoom = 1.5; else ok.zoom = 1.0;
  return true;
}

void GuiCombat::Render()
{
  if (NONE == cursor_pos) return;
  JRenderer* renderer = JRenderer::GetInstance();
  renderer->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(200,0,0,0));

  for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
    if ((*it)->show) (*it)->Render(step);
  if (activeAtk)
    for (vector<DefenserDamaged*>::iterator q = activeAtk->blockers.begin(); q != activeAtk->blockers.end(); ++q)
      (*q)->Render(step);
  if (ok_quad) ok.Render(ok_quad);
  renderer->DrawLine(0, SCREEN_HEIGHT / 2 + 10, SCREEN_WIDTH, SCREEN_HEIGHT / 2 + 10, ARGB(255, 255, 64, 0));
  if (FIRST_STRIKE == step)
    {
      JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
      mFont->SetColor(ARGB(255, 64, 255, 64));
      mFont->DrawString("First strike damage", 370, 2);
    }
}

int GuiCombat::receiveEventPlus(WEvent* e)
{
  if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
      if (NULL == event->after) return 0;
      cout << "Attacker : " << event->card->name << " " << event->before << " -> " << event->after << endl;
      AttackerDamaged* t = NEW AttackerDamaged(event->card, *(event->card->view), true, NULL);
      attackers.push_back(t);
      return 1;
    }
  else if (WEventCreatureBlocker* event = dynamic_cast<WEventCreatureBlocker*>(e))
    {
      cout << "Blocker : " << event->card->name << " " << event->before << " -> " << event->after << endl;
      for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
        if ((*it)->card == event->after)
          {
            DefenserDamaged* t = NEW DefenserDamaged(event->card, *(event->card->view), true, NULL);
            t->y = t->actY = TOP_LINE; t->actT = t->t = 0; t->actZ = t->zoom = 2.2;
            (*it)->blockers.push_back(t);
            return 1;
          }
      return 0;
    }
  else if (WEventCreatureBlockerRank* event = dynamic_cast<WEventCreatureBlockerRank*>(e))
    {
      cout << "Order : " << event->card->name << " -> " << event->exchangeWith->name << endl;
      for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
        if ((*it)->card == event->attacker)
          {
            vector<DefenserDamaged*>::iterator it1, it2;
            for (it1 = (*it)->blockers.begin(); it1 != (*it)->blockers.end(); ++it1) if ((*it1)->card == event->card) break;
            if ((*it)->blockers.end() == it1) return 1;
            for (it2 = (*it)->blockers.begin(); it2 != (*it)->blockers.end(); ++it2) if ((*it2)->card == event->exchangeWith) break;
            if ((*it)->blockers.end() == it2) return 1;
            float x = (*it1)->x;
            (*it1)->x = (*it2)->x;
            (*it2)->x = x;
            std::iter_swap(it1, it2);
            reaffectDamage(*it, DAMAGE);
          }
      return 1;
    }
  return 0;
}
int GuiCombat::receiveEventMinus(WEvent* e)
{
  if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
      if (NULL == event->before) return 0;
      for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
	if ((*it)->card == event->card)
	  {
	    attackers.erase(it);
	    return 1;
	  }
      return 1;
    }
  else if (WEventCreatureBlocker* event = dynamic_cast<WEventCreatureBlocker*>(e))
    {
      cout << "Blocker : " << event->card->name << " " << event->before << " -> " << event->after << endl;
      for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
        if ((*it)->card == event->before)
          for (vector<DefenserDamaged*>::iterator q = (*it)->blockers.begin(); q != (*it)->blockers.end(); ++q)
            if ((*q)->card == event->card)
              {
                DefenserDamaged* d = *q;
                (*it)->blockers.erase(q);
                SAFE_DELETE(d);
                return 1;
              }
      return 0;
    }
  else if (WEventCombatStepChange* event = dynamic_cast<WEventCombatStepChange*>(e))
    switch (event->step)
    {
    case ORDER:
      {
        if (ORDER == step) return 0; // Why do I take this twice ? >.>
        if (go->currentPlayer->isAI()) { go->receiveEvent(NEW WEventCombatStepChange(FIRST_STRIKE)); return 1; }
        for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
          {
            (*it)->show = (1 < (*it)->blockers.size());
            reaffectDamage(*it, DAMAGE);
          }
        active = NULL;
        for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
          if ((*it)->show)
            {
              (*it)->y = 210;
              (*it)->zoom = 2.2; (*it)->t = 0;
              if (!active) active = *it;
            }
        repos<AttackerDamaged>(attackers.begin(), attackers.end(), 0);
        if (active)
          {
            active->zoom = 2.7; // We know there is at least one, so this cannot be NULL
            activeAtk = static_cast<AttackerDamaged*>(active);
            remaskBlkViews(NULL, static_cast<AttackerDamaged*>(active));
            cursor_pos = ATK;
            step = ORDER;
          }
        else
          go->receiveEvent(NEW WEventCombatStepChange(FIRST_STRIKE));
        return 1;
      }
    case FIRST_STRIKE:
      for (inner_iterator attacker = attackers.begin(); attacker != attackers.end(); ++attacker)
        if ((*attacker)->card->has(Constants::FIRSTSTRIKE) || (*attacker)->card->has(Constants::DOUBLESTRIKE)) goto DAMAGE;
      go->receiveEvent(NEW WEventCombatStepChange(DAMAGE));
      return 1;
    case DAMAGE: DAMAGE:
      if (go->currentPlayer->isAI()) { go->userRequestNextGamePhase(); return 1; }
      step = event->step;
      cursor_pos = ATK;
      for (inner_iterator attacker = attackers.begin(); attacker != attackers.end(); ++attacker)
        reaffectDamage(*attacker, step);
      for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it)
        (*it)->show = ((*it)->card->has(Constants::DOUBLESTRIKE) || ((*it)->card->has(Constants::FIRSTSTRIKE) ^ (DAMAGE == step))) && (1 < (*it)->blockers.size());
      repos<AttackerDamaged>(attackers.begin(), attackers.end(), 0);
      active = NULL;
      for (inner_iterator it = attackers.begin(); it != attackers.end(); ++it) if ((*it)->show) { active = *it; break; }
      if (active)
        {
          active->zoom = 2.7;
          activeAtk = static_cast<AttackerDamaged*>(active);
          remaskBlkViews(NULL, static_cast<AttackerDamaged*>(active));
        }
      else
        {
          if (FIRST_STRIKE == step) go->receiveEvent(NEW WEventCombatStepChange(DAMAGE));
          else go->userRequestNextGamePhase();
        }
      return 1;
    }
  return 0;
}
