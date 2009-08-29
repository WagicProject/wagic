#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiCombat.h"
#include "Closest.cpp"

static const float MARGIN = 70;


struct Left : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return ref->y == test->y && ref->x > test->x; } };
struct Right : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return ref->y == test->y && ref->x < test->x; } };


GuiCombat::GuiCombat(GameObserver* go) : GuiLayer(), go(go), active(false), ok(SCREEN_WIDTH - MARGIN, 210, 1, 0, 255), cursor_pos(NONE)
{
  if (!GameApp::CommonRes->GetTexture("Ok.png"))
    {
      GameApp::CommonRes->CreateTexture("Ok.png");
      GameApp::CommonRes->CreateQuad("OK", "Ok.png", 0, 0, 56, 45);
    }
  ok_quad = GameApp::CommonRes->GetQuad("OK");
  if(ok_quad)
    ok_quad->SetHotSpot(28, 22);
}

GuiCombat::~GuiCombat()
{
}

void GuiCombat::Update(float dt)
{
  if (NONE != cursor_pos)
    {
      for (vector<TransientCardView*>::iterator it = atkViews.begin(); it != atkViews.end(); ++it)
        (*it)->Update(dt);
      for (vector<TransientCardView*>::iterator it = blkViews.begin(); it != blkViews.end(); ++it)
        (*it)->Update(dt);
      ok.Update(dt);
    }
}

void GuiCombat::generateBlkViews(MTGCardInstance* card)
{
  for (vector<TransientCardView*>::iterator it = blkViews.begin(); it != blkViews.end(); ++it)
    SAFE_DELETE(*it);
  blkViews.clear();
  if (card && card->blockers.size() > 0)
    {
      float space = (SCREEN_WIDTH - 2*MARGIN) / card->blockers.size();
      float pos = MARGIN;
      for (list<MTGCardInstance*>::iterator it = card->blockers.begin(); it != card->blockers.end(); ++it)
        {
          TransientCardView* t = NEW TransientCardView(*it, 20, 60);
          blkViews.push_back(t);
          t->x = pos; t->y = 60;
          t->zoom = 2.2; t->t = 0;
          pos += space;
        }
    }
}

bool GuiCombat::CheckUserInput(u32 key)
{
  if (NONE == cursor_pos) return false;
  TransientCardView* oldActive = active;
  switch (key)
    {
    case PSP_CTRL_CIRCLE:
      if (BLK == cursor_pos)
	activeAtk->card->raiseBlockerRankOrder(active->card);
      else if (OK == cursor_pos) { cursor_pos = NONE; go->userRequestNextGamePhase(); };
      break;
    case PSP_CTRL_UP:
      if (ATK == cursor_pos)
        {
          activeAtk = active;
          active = blkViews.front();
          active->zoom = 2.7;
          cursor_pos = BLK;
        }
      return true;
    case PSP_CTRL_DOWN:
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
	case OK   : active = atkViews.back(); cursor_pos = ATK; break;
	case ATK  : active = closest<Left>(atkViews, NULL, active); break;
	case BLK  : active = closest<Left>(blkViews, NULL, active); break;
	}
      break;
    case PSP_CTRL_RIGHT:
      switch (cursor_pos)
	{
	case NONE :
	case OK   : break;
	case BLK  : active = closest<Right>(blkViews, NULL, active); break;
	case ATK  : active = closest<Right>(atkViews, NULL, active);
	  if (active == oldActive) { active = NULL; cursor_pos = OK; }
	  break;
	}
      break;
    case PSP_CTRL_SQUARE:
    case PSP_CTRL_RTRIGGER:
      active = NULL; cursor_pos = OK;
      break;
    }
  if (oldActive != active)
    {
      if (oldActive) oldActive->zoom = 2.2;
      if (active) active->zoom = 2.7;
      if (ATK == cursor_pos) generateBlkViews(active->card);
    }
  if (OK == cursor_pos) ok.zoom = 1.5; else ok.zoom = 1.0;
  return true;
}

void GuiCombat::Render()
{
  if (NONE == cursor_pos) return;
  JRenderer* renderer = JRenderer::GetInstance();
  renderer->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB(200,0,0,0));

  for (vector<TransientCardView*>::iterator it = atkViews.begin(); it != atkViews.end(); ++it)
    (*it)->Render();
  for (vector<TransientCardView*>::iterator it = blkViews.begin(); it != blkViews.end(); ++it)
    (*it)->Render();

  if(ok_quad)
    ok.Render(ok_quad);

  renderer->DrawLine(0, SCREEN_HEIGHT / 2 + 10, SCREEN_WIDTH, SCREEN_HEIGHT / 2 + 10, ARGB(255, 255, 64, 0));
}

int GuiCombat::receiveEventPlus(WEvent* e)
{
  if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
      if (NULL == event->after) return 0;
      cout << "Attacker : " << event->card->name << " " << event->before << " -> " << event->after << endl;
      attackers.push_back(event->card);
      return 1;
    }
  else if (WEventCreatureBlocker* event = dynamic_cast<WEventCreatureBlocker*>(e))
    {
      cout << "Blocker : " << event->card->name << " " << event->before << " -> " << event->after << endl;
      if (NULL == event->after) return 0;
      return 1;
    }
  else if (WEventCreatureBlockerRank* event = dynamic_cast<WEventCreatureBlockerRank*>(e))
    {
      cout << "Order : " << event->card->name << " -> " << event->exchangeWith->name << endl;
      vector<TransientCardView*>::iterator it1, it2;
      for (it1 = blkViews.begin(); it1 != blkViews.end(); ++it1) if ((*it1)->card == event->card) break; if (blkViews.end() == it1) return 1;
      for (it2 = blkViews.begin(); it2 != blkViews.end(); ++it2) if ((*it2)->card == event->exchangeWith) break; if (blkViews.end() == it2) return 1;
      float x = (*it1)->x;
      (*it1)->x = (*it2)->x;
      (*it2)->x = x;
      return 1;
    }
  return 0;
}
int GuiCombat::receiveEventMinus(WEvent* e)
{
  // (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e)
  if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
      if (NULL == event->before) return 0;
      for (vector<MTGCardInstance*>::iterator it = attackers.begin(); it != attackers.end(); ++it)
	if (*it == event->card)
	  {
	    attackers.erase(it);
	    return 1;
	  }
      return 1;
    }
  else if (WEventBlockersAssigned* event = dynamic_cast<WEventBlockersAssigned*>(e))
    {
      if (active) return 0; // Why do I take this event twice >.>
      if (go->currentPlayer->isAI()) return 0;
      unsigned size = 0;
      for (vector<MTGCardInstance*>::iterator it = attackers.begin(); it != attackers.end(); ++it)
	if (1 < (*it)->blockers.size()) ++size;
      if (0 == size) return 1;
      float space = (SCREEN_WIDTH - 2*MARGIN) / size;
      float pos = MARGIN;
      for (vector<MTGCardInstance*>::iterator it = attackers.begin(); it != attackers.end(); ++it)
	if (1 < (*it)->blockers.size())
	  {
	    TransientCardView* t = NEW TransientCardView(*it, *(*it)->view);
	    atkViews.push_back(t);
	    t->x = pos; t->y = 210;
	    t->zoom = 2.2; t->t = 0;
	    pos += space;
	  }
      active = *atkViews.begin();
      active->zoom = 2.7;
      generateBlkViews(active->card);
      cursor_pos = ATK;
    }
  return 0;
}
