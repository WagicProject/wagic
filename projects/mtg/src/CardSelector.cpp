#include <iostream>
#include "../include/PlayGuiObject.h"
#include "../include/CardGui.h"
#include "../include/CardSelector.h"
#include "../include/GuiHand.h"

using std::cout;

#define closest(src, expr)			\
  {									\
  float curdist = 1000000.0f; /* This is bigger than any possible distance */ \
  for (vector<Target*>::iterator it = cards.begin(); it != cards.end(); ++it) \
    {									\
  if (!(expr)) continue;						\
  if ((*it)->actA < 32) continue;					\
  if ((NULL != limitor) && (!limitor->select(*it))) continue;		\
  if (active)								\
    {									\
      float dist = ((*it)->x - active->x) * ((*it)->x - active->x) +	\
	((*it)->y - active->y) * ((*it)->y - active->y);		\
      if (dist < curdist)						\
	{								\
	  curdist = dist;						\
	  card = *it;							\
	}								\
    }									\
  else									\
    card = *it;								\
    }									\
{ CardView* c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.0; } \
active = card;								\
{ CardView* c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.4; } \
}


template<>
CardSelector::ObjectSelector(DuelLayers* duel) : active(NULL), showBig(true), duel(duel), limitor(NULL), bigpos(300, 150, 1.0, 0.0, 220) {}

template<>
void CardSelector::Add(CardSelector::Target* target)
{
  if (NULL == active)
    if (NULL == limitor || limitor->select(active))
      active = target;
  CardView* c = dynamic_cast<CardView*>(target);
  if (c) c->zoom = 1.0;
  c = dynamic_cast<CardView*>(active);
  if (c) c->zoom = 1.4;
  cards.push_back(target);
}
template<>
void CardSelector::Remove(CardSelector::Target* card)
{
  for (vector<Target*>::iterator it = cards.begin(); it != cards.end(); ++it)
    {
      if (card == *it)
	{
	  if (active == *it)
	    {
	      CardView* c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.0;
	      closest(active, active != (*it));
	      c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.4;
	    }
	  if (active == *it) active = NULL;
	  cards.erase(it);
	  return;
	}
    }
}

template<>
bool CardSelector::CheckUserInput(u32 key)
{
  if (!active)
    {
      for (vector<Target*>::iterator it = cards.begin(); it != cards.end(); ++it)
	if ((NULL == limitor) || (limitor->select(*it)))
	  {
	    active = *it;
	    active->Entering();
	    return true;
	  }
      return true;
    }
  Target* oldactive = active;
  Target* card = active;
  switch (key)
    {
    case PSP_CTRL_CIRCLE:
      GameObserver::GetInstance()->ButtonPressed(active);
      break;
    case PSP_CTRL_LEFT:
      closest(active, (
		       active->x - (*it)->x > fabs(active->y - (*it)->y)
		       ));
      if (active != oldactive) { oldactive->Leaving(key); active->Entering(); }
      break;
    case PSP_CTRL_RIGHT:
      closest(active, (
		       (*it)->x - active->x > fabs(active->y - (*it)->y)
		       ));
      if (active != oldactive) { oldactive->Leaving(key); active->Entering(); }
      break;
    case PSP_CTRL_UP:
      closest(active, (
		       active->y - (*it)->y > fabs(active->x - (*it)->x)
		       ));
      if (active != oldactive) { oldactive->Leaving(key); active->Entering(); }
      break;
    case PSP_CTRL_DOWN:
      closest(active, (
		       (*it)->y - active->y > fabs(active->x - (*it)->x)
		       ));
      if (active != oldactive) { oldactive->Leaving(key); active->Entering(); }
      break;
    case PSP_CTRL_LTRIGGER:
      showBig = !showBig;
      return true;
    default:
      return false;
    }
  return true;
}

template<>
void CardSelector::Update(float dt)
{
  float boundary = duel->RightBoundary();
  float position = boundary - CardGui::BigWidth / 2;
  if (CardView* c = dynamic_cast<CardView*>(active))
    if ((c->x + CardGui::Width / 2 > position - CardGui::BigWidth / 2) &&
	(c->x - CardGui::Width / 2 < position + CardGui::BigWidth / 2))
      position = CardGui::BigWidth / 2 - 10;
  bigpos.x = position;
  bigpos.Update(dt);
}

template<>
void CardSelector::Render()
{
  if (active)
    {
      active->Render();
      if (CardView* c = dynamic_cast<CardView*>(active))
	if (showBig)
	  c->RenderBig(bigpos);
    }
}

template<>
void CardSelector::Limit(LimitorFunctor<Target>* limitor)
{
  this->limitor = limitor;
  if (limitor && !limitor->select(active))
    {
      Target* card = NULL;
      closest(active, true);
      if (limitor && !limitor->select(active)) active = NULL;
    }
}
