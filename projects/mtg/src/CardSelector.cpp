#include <iostream>
#include "../include/PlayGuiObject.h"
#include "../include/CardGui.h"
#include "../include/CardSelector.h"
#include "../include/GuiHand.h"
#include "Closest.cpp"

using std::cout;




struct Left : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return ref->x - test->x > fabs(ref->y - test->y); } };
struct Right : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return test->x - ref->x > fabs(ref->y - test->y); } };
struct Up : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return ref->y - test->y > fabs(ref->x - test->x); } };
struct Down : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return test->y - ref->y > fabs(ref->x - test->x); } };
struct Diff : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return ref != test; } };
struct True : public Exp { static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
  { return true; } };

template<>
CardSelector::ObjectSelector(DuelLayers* duel) : active(NULL), duel(duel), limitor(NULL), bigpos(300, 150, 1.0, 0.0, 220), bigMode(BIG_MODE_SHOW) {}

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
	      active = closest<Diff>(cards, limitor, active);
	      c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.4;
	    }
	  if (active == *it) active = NULL;
	  cards.erase(it);
	  return;
	}
    }
}

template <>
CardSelector::Target* CardSelector::fetchMemory(SelectorMemory& memory)
{
  if (NULL == memory.object) return NULL;
  for (vector<Target*>::iterator it = cards.begin(); it != cards.end(); ++it)
    {
      if (*it == memory.object)
        {
          if ((NULL == limitor) || (limitor->select(memory.object)))
            return memory.object;
          else break;
        }
    }
  // We come here if the card is not in the selector any more, or if
  // it is there but it is now refused by the limitor.
  return closest<True>(cards, limitor, memory.x, memory.y);
}
template<>
void CardSelector::Push()
{
  memoryStack.push(SelectorMemory(active));
}
template<>
void CardSelector::Pop()
{
  Target* oldactive = active;
  if (!memoryStack.empty())
    {
      active = fetchMemory(memoryStack.top());
      memoryStack.pop();
      SelectorZone oldowner;
      if (CardView *q = dynamic_cast<CardView*>(oldactive)) oldowner = q->owner; else oldowner = nullZone;
      if (nullZone != oldowner) lasts[oldowner] = SelectorMemory(oldactive);
    }
  if (active != oldactive)
    {
      { CardView* c = dynamic_cast<CardView*>(oldactive); if (c) c->zoom = 1.0; } //Is this needed, I think it is one in Leaving(0) ?
      { CardView* c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.4; } //Is this needed, I think it is one in Entering() ?
      if (oldactive) oldactive->Leaving(0);
      if (active) active->Entering();
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
  switch (key)
    {
    case PSP_CTRL_CROSS:
      GameObserver::GetInstance()->cancelCurrentAction();
      return true;
    case PSP_CTRL_CIRCLE:
      GameObserver::GetInstance()->ButtonPressed(active);
      return true;
      break;
    case PSP_CTRL_LEFT:
      active = closest<Left>(cards, limitor, active);
      break;
    case PSP_CTRL_RIGHT:
      active = closest<Right>(cards, limitor, active);
      break;
    case PSP_CTRL_UP:
      active = closest<Up>(cards, limitor, active);
      break;
    case PSP_CTRL_DOWN:
      active = closest<Down>(cards, limitor, active);
      break;
    case PSP_CTRL_TRIANGLE:
      bigMode = (bigMode+1) % NB_BIG_MODES;
      return true;
    default:
      return false;
    }
  if (active != oldactive)
    {
      SelectorZone oldowner, owner;
      if (CardView *q = dynamic_cast<CardView*>(oldactive)) oldowner = q->owner; else oldowner = nullZone;
      if (CardView *q = dynamic_cast<CardView*>(active))       owner = q->owner; else    owner = nullZone;
      if (oldowner != owner)
        {
          if (nullZone != owner)
            if (PlayGuiObject* old = fetchMemory(lasts[owner]))
              active = old;
          lasts[oldowner] = SelectorMemory(oldactive);
        }
    }
  if (active != oldactive)
    {
      { CardView* c = dynamic_cast<CardView*>(oldactive); if (c) c->zoom = 1.0; } //Is this needed, I think it is one in Leaving(0) ?
      { CardView* c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.4; } //Is this needed, I think it is one in Entering() ?
      if (oldactive) oldactive->Leaving(0);
      if (active) active->Entering();
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
      if (CardView* c = dynamic_cast<CardView*>(active)){
        switch(bigMode){
        case BIG_MODE_SHOW:
          c->RenderBig(bigpos);
          break;
        case BIG_MODE_TEXT:
          c->alternateRenderBig(bigpos);
          break;
        default:
          break;
        }
      }
    }
}

template<>
void CardSelector::Limit(LimitorFunctor<Target>* limitor, SelectorZone destzone)
{
  this->limitor = limitor;
  if (limitor && !limitor->select(active))
    {
      Target* oldactive = active;
      SelectorZone oldowner;
      if (CardView *q = dynamic_cast<CardView*>(oldactive)) oldowner = q->owner; else oldowner = nullZone;
      if (oldowner != destzone)
        {
          if (nullZone != destzone)
            if (PlayGuiObject* old = fetchMemory(lasts[destzone]))
              active = old;
          lasts[oldowner] = SelectorMemory(oldactive);
        }

      if (limitor && !limitor->select(active))
        {
          active = NULL;
          for (vector<Target*>::iterator it = cards.begin(); it != cards.end(); ++it)
	          if (limitor->select(*it))
	            {
	              active = *it;
                break;
	            }
        }

      if (active != oldactive)
        {
          { CardView* c = dynamic_cast<CardView*>(oldactive); if (c) c->zoom = 1.0; }
          { CardView* c = dynamic_cast<CardView*>(active); if (c) c->zoom = 1.4; }
          if (oldactive) oldactive->Leaving(0);
          if (active) active->Entering();
        }
    }
}
