#include "PrecompiledHeader.h"

#include "PlayGuiObject.h"
#include "CardGui.h"
#include "CardSelector.h"
#include "GuiHand.h"
#include "Closest.cpp"
#include "GameObserver.h"

//using std::cout;

// The X lib annoyingly defines True to be 1, leading to
// hard to understand syntax errors. Not using it, so it's
// safe to undefine it.
#ifdef True
#undef True
#endif

struct Left: public Exp
{
    static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
    {
        return ref->x - test->x > fabs(ref->y - test->y);
    }
};
struct Right: public Exp
{
    static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
    {
        return test->x - ref->x > fabs(ref->y - test->y);
    }
};
struct Up: public Exp
{
    static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
    {
        return ref->y - test->y > fabs(ref->x - test->x);
    }
};
struct Down: public Exp
{
    static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
    {
        return test->y - ref->y > fabs(ref->x - test->x);
    }
};
struct Diff: public Exp
{
    static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
    {
        return ref != test;
    }
};
struct True: public Exp
{
    static inline bool test(CardSelector::Target* ref, CardSelector::Target* test)
    {
        return true;
    }
};

CardSelector::SelectorMemory::SelectorMemory(PlayGuiObject* object) :
    object(object)
{
    if (object)
    {
        x = object->x;
        y = object->y;
    }
}
CardSelector::SelectorMemory::SelectorMemory()
{
    object = NULL;
    x = y = 0;
}

CardSelector::CardSelector(DuelLayers* duel) :
    active(NULL), duel(duel), limitor(NULL), bigpos(300, 145, 1.0, 0.0, 220)
{
}

void CardSelector::Add(CardSelector::Target* target)
{
    if (NULL == active)
        if (NULL == limitor || limitor->select(active))
            active = target;
    CardView* c = dynamic_cast<CardView*> (target);
    if (c)
        c->zoom = 1.0f;
    c = dynamic_cast<CardView*> (active);
    if (c)
        c->zoom = 1.4f;
    cards.push_back(target);
}

void CardSelector::Remove(CardSelector::Target* card)
{
    for (vector<Target*>::iterator it = cards.begin(); it != cards.end(); ++it)
        if (card == *it)
        {
            if (active == *it)
            {
                CardView* c = dynamic_cast<CardView*> (active);
                if (c)
                    c->zoom = 1.0f;
                active = closest<Diff> (cards, limitor, active);
                c = dynamic_cast<CardView*> (active);
                if (c)
                    c->zoom = 1.4f;
            }
            if (active == *it)
                active = NULL;
            cards.erase(it);
            return;
        }
}

CardSelector::Target* CardSelector::fetchMemory(SelectorMemory& memory)
{
    if (NULL == memory.object)
        return NULL;
    for (vector<Target*>::iterator it = cards.begin(); it != cards.end(); ++it)
        if (*it == memory.object)
        {
            if ((NULL == limitor) || (limitor->select(memory.object)))
                return memory.object;
            else
                break;
        }
    // We come here if the card is not in the selector any more, or if
    // it is there but it is now refused by the limitor.
    return closest<True> (cards, limitor, memory.x, memory.y);
}

void CardSelector::Push()
{
    memoryStack.push(SelectorMemory(active));
}

void CardSelector::Pop()
{
    Target* oldactive = active;
    if (!memoryStack.empty())
    {
        active = fetchMemory(memoryStack.top());
        memoryStack.pop();
        CardView::SelectorZone oldowner;
        if (CardView *q = dynamic_cast<CardView*>(oldactive))
            oldowner = q->owner;
        else
            oldowner = CardView::nullZone;
        if (CardView::nullZone != oldowner)
            lasts[oldowner] = SelectorMemory(oldactive);
    }
    if (active != oldactive)
    {
        {
            CardView* c = dynamic_cast<CardView*> (oldactive);
            if (c)
                c->zoom = 1.0f;
        } //Is this needed, I think it is one in Leaving(0) ?
        {
            CardView* c = dynamic_cast<CardView*> (active);
            if (c)
                c->zoom = 1.4f;
        } //Is this needed, I think it is one in Entering() ?
        if (oldactive)
            oldactive->Leaving(JGE_BTN_NONE);
        if (active)
            active->Entering();
    }
}

bool CardSelector::CheckUserInput(JButton key)
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
    case JGE_BTN_SEC:
        GameObserver::GetInstance()->cancelCurrentAction();
        return true;
    case JGE_BTN_OK:
        GameObserver::GetInstance()->ButtonPressed(active);
        return true;
        break;
    case JGE_BTN_LEFT:
        active = closest<Left> (cards, limitor, active);
        break;
    case JGE_BTN_RIGHT:
        active = closest<Right> (cards, limitor, active);
        break;
    case JGE_BTN_UP:
        active = closest<Up> (cards, limitor, active);
        break;
    case JGE_BTN_DOWN:
        active = closest<Down> (cards, limitor, active);
        break;
    case JGE_BTN_CANCEL:
        mDrawMode = (mDrawMode + 1) % DrawMode::kNumDrawModes;
        if (mDrawMode == DrawMode::kText)
            options[Options::DISABLECARDS].number = 1;
        else
            options[Options::DISABLECARDS].number = 0;
        return true;
    default:
      {
        int x,y;
        if(JGE::GetInstance()->GetLeftClickCoordinates(x, y))
        {
          active = closest<True> (cards, limitor, static_cast<float> (x), static_cast<float> (y));
        }
        else
        {
          return false;
        }
      }
    }
    if(key != JGE_BTN_NONE)
    {
      if (active != oldactive)
      {
          CardView::SelectorZone oldowner, owner;
          if (CardView *q = dynamic_cast<CardView*>(oldactive))
              oldowner = q->owner;
          else
              oldowner = CardView::nullZone;
          if (CardView *q = dynamic_cast<CardView*>(active))
              owner = q->owner;
          else
              owner = CardView::nullZone;
          if (oldowner != owner)
          {
              if (CardView::nullZone != owner)
              {
                  if (PlayGuiObject* old = fetchMemory(lasts[owner]))
                      switch (key)
                      {
                      case JGE_BTN_LEFT:
                          if (old->x < oldactive->x)
                              active = old;
                          break;
                      case JGE_BTN_RIGHT:
                          if (old->x > oldactive->x)
                              active = old;
                          break;
                      case JGE_BTN_UP:
                          if (old->y < oldactive->y)
                              active = old;
                          break;
                      case JGE_BTN_DOWN:
                          if (old->y > oldactive->y)
                              active = old;
                          break;
                      default:
                          if (old)
                              active = old;
                          break;
                      }
              }
              lasts[oldowner] = SelectorMemory(oldactive);
          }
      }
      else
      {
          // active card hasn't changed - that means we're probably at an edge of the battlefield.
          // check if we're not already a selected avatar - if not, select one depending whether we're going up/down.
          GuiAvatar* avatar = dynamic_cast<GuiAvatar*> (active);
          if (!avatar)
          {
              if (key == JGE_BTN_DOWN)
              {
                  active = duel->GetAvatars()->GetSelf();
              }
              else if (key == JGE_BTN_UP)
              {
                  active = duel->GetAvatars()->GetOpponent();
              }
          }
      }
    }
    if (active != oldactive)
    {
        {
            CardView* c = dynamic_cast<CardView*> (oldactive);
            if (c)
                c->zoom = 1.0f;
        }
        {
            CardView* c = dynamic_cast<CardView*> (active);
            if (c)
                c->zoom = 1.4f;
        }
        if (oldactive)
            oldactive->Leaving(JGE_BTN_NONE);
        if (active)
            active->Entering();
    }
    return true;
}

void CardSelector::Update(float dt)
{
    float boundary = duel->RightBoundary();
    float position = boundary - CardGui::BigWidth / 2;
    if (CardView* c = dynamic_cast<CardView*>(active))
        if ((c->x + CardGui::Width / 2 > position - CardGui::BigWidth / 2) && (c->x - CardGui::Width / 2 < position
                        + CardGui::BigWidth / 2))
            position = CardGui::BigWidth / 2 - 10;
    if (position < CardGui::BigWidth / 2)
        position = CardGui::BigWidth / 2;
    bigpos.x = position;
    bigpos.Update(dt);
}

void CardSelector::Render()
{
    if (active)
    {
        active->Render();
        if (CardView* card = dynamic_cast<CardView*>(active))
        {
            card->DrawCard(bigpos, mDrawMode);
        }
    }
}

void CardSelector::Limit(LimitorFunctor<PlayGuiObject>* limitor, CardView::SelectorZone destzone)
{
    this->limitor = limitor;
    if (limitor && !limitor->select(active))
    {
        PlayGuiObject* oldactive = active;
        CardView::SelectorZone oldowner;
        if (CardView *q = dynamic_cast<CardView*>(oldactive))
            oldowner = q->owner;
        else
            oldowner = CardView::nullZone;
        if (oldowner != destzone)
        {
            if (CardView::nullZone != destzone)
                if (PlayGuiObject* old = fetchMemory(lasts[destzone]))
                    active = old;
            lasts[oldowner] = SelectorMemory(oldactive);
        }

        if (limitor && !limitor->select(active))
        {
            active = NULL;
            for (vector<PlayGuiObject*>::iterator it = cards.begin(); it != cards.end(); ++it)
                if (limitor->select(*it))
                {
                    active = *it;
                    break;
                }
        }

        if (active != oldactive)
        {
            {
                CardView* c = dynamic_cast<CardView*> (oldactive);
                if (c)
                    c->zoom = 1.0f;
            }
            {
                CardView* c = dynamic_cast<CardView*> (active);
                if (c)
                    c->zoom = 1.4f;
            }
            if (oldactive)
                oldactive->Leaving(JGE_BTN_NONE);
            if (active)
                active->Entering();
        }
    }
}

void CardSelector::PushLimitor()
{
    if (NULL == limitor)
        return;
    CardView::SelectorZone owner;
    if (CardView *q = dynamic_cast<CardView*>(active))
        owner = q->owner;
    else
        owner = CardView::nullZone;
    limitorStack.push(make_pair(limitor, owner));
}

void CardSelector::PopLimitor()
{
    if (limitorStack.empty())
        return;
    Limit(limitorStack.top().first, limitorStack.top().second);
    limitorStack.pop();
}
