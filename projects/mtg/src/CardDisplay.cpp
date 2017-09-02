#include "PrecompiledHeader.h"

#include "CardDisplay.h"
#include "CardGui.h"
#include "CardSelector.h"
#include "TargetChooser.h"
#include "MTGGameZones.h"
#include "GameObserver.h"

CardDisplay::CardDisplay(GameObserver* game) :
    PlayGuiObjectController(game), mId(0)
{
    tc = NULL;
    listener = NULL;
    nb_displayed_items = 7;
    start_item = 0;
    x = 0;
    y = 0;
    zone = NULL;
}

CardDisplay::CardDisplay(int id, GameObserver* game, int _x, int _y, JGuiListener * _listener, TargetChooser * _tc,
                int _nb_displayed_items) :
    PlayGuiObjectController(game), mId(id), x(_x), y(_y)
{
    tc = _tc;
    listener = _listener;
    nb_displayed_items = _nb_displayed_items;
    start_item = 0;
    if (x + nb_displayed_items * 30 + 25 > SCREEN_WIDTH) x = SCREEN_WIDTH - (nb_displayed_items * 30 + 25);
    if (y + 55 > SCREEN_HEIGHT) y = SCREEN_HEIGHT - 55;
    zone = NULL;
}

void CardDisplay::AddCard(MTGCardInstance * _card)
{
    CardGui * card = NEW CardView(CardView::nullZone, _card, static_cast<float> (x + 20 + (mObjects.size() - start_item) * 30),
                    static_cast<float> (y + 25));
    Add(card);
}

void CardDisplay::init(MTGGameZone * zone)
{
    resetObjects();
    if (!zone) return;
    start_item = 0;
    vector<MTGCardInstance*> newCD (zone->cards.rbegin(), zone->cards.rend());
    for (int i = 0; i < zone->nb_cards; i++)//invert display so the top will always be the first one to show
    {
        //AddCard(zone->cards[i]);
        AddCard(newCD[i]);
    }
    if (mObjects.size()) mObjects[0]->Entering();
}

void CardDisplay::rotateLeft()
{
    if (start_item == 0) return;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        CardGui * cardg = (CardGui *) mObjects[i];
        cardg->x += 30;
    }
    start_item--;
}

void CardDisplay::rotateRight()
{
    if (start_item == (int)(mObjects.size()) - 1) return;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        CardGui * cardg = (CardGui *) mObjects[i];
        cardg->x -= 30;
    }
    start_item++;
}

void CardDisplay::Update(float dt)
{
    bool update = false;

    if (zone)
    {//invert display so the top will always be the first one to show
        vector<MTGCardInstance*> newCD (zone->cards.rbegin(), zone->cards.rend());
        int size = zone->cards.size();
        for (int i = start_item; i < start_item + nb_displayed_items && i < (int)(mObjects.size()); i++)
        {
            if (i > size - 1)
            {
                update = true;
                break;
            }
            CardGui * cardg = (CardGui *) mObjects[i];
            if (cardg->card != newCD[i]) update = true;
        }
    }
    PlayGuiObjectController::Update(dt);
    if (update)
        init(zone);
}

bool CardDisplay::CheckUserInput(JButton key)
{
    if (JGE_BTN_SEC == key || JGE_BTN_PRI == key || JGE_BTN_UP == key || JGE_BTN_DOWN == key)
    {
        if (listener)
        {
            listener->ButtonPressed(mId, 0);
            return true;
        }
    }
    if (!mObjects.size()) return false;

    if (mActionButton == key)
    {
        if (mObjects[mCurr] && mObjects[mCurr]->ButtonPressed())
        {
            CardGui * cardg = (CardGui *) mObjects[mCurr];
            if (tc)
            {
                tc->toggleTarget(cardg->card);
                return true;
            }
            else
            {
                if (observer) observer->ButtonPressed(cardg);
                return true;
            }
        }
        return true;
    }

    switch (key)
    {
    case JGE_BTN_LEFT:
    {
        int n = mCurr;
        n--;
        if (n < start_item)
        {
            if (n < 0)
            {
                n = 0;
            }
            else
            {
                rotateLeft();
            }
        }
        if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(JGE_BTN_LEFT))
        {
            mCurr = n;
            mObjects[mCurr]->Entering();
        }
        return true;
    }
    case JGE_BTN_RIGHT:
    {
        int n = mCurr;
        n++;
        if (n >= (int) (mObjects.size()))
        {
            n = mObjects.size() - 1;
        }
        if (n >= start_item + nb_displayed_items)
        {
            rotateRight();
        }
        if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(JGE_BTN_RIGHT))
        {
            mCurr = n;
            mObjects[mCurr]->Entering();
        }
        return true;
    }
    default:
    {
      bool result = false;
      unsigned int distance2;
      unsigned int minDistance2 = -1;
      int n = mCurr;
      int x1,y1;
      JButton key;
      JGE* jge = observer?observer->getInput():JGE::GetInstance();
      if(jge)
      {
          if (jge->GetLeftClickCoordinates(x1, y1))
          {
              for (size_t i = 0; i < mObjects.size(); i++)
              {
                  float top, left;
                  if (mObjects[i]->getTopLeft(top, left))
                  {
                      distance2 = static_cast<unsigned int>((top - y1) * (top - y1) + (left - x1) * (left - x1));
                      if (distance2 < minDistance2)
                      {
                          minDistance2 = distance2;
                          n = i;
                      }
                  }
              }

              if (n < mCurr)
                  key = JGE_BTN_LEFT;
              else
                  key = JGE_BTN_RIGHT;

              if (n < start_item)
              {
                  rotateLeft();
              }
              else if (n >= (int)(mObjects.size()) && mObjects.size())
              {
                  n = mObjects.size() - 1;
              }
              if (n >= start_item + nb_displayed_items)
              {
                  rotateRight();
              }

              if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(key))
              {
                  mCurr = n;
                  mObjects[mCurr]->Entering();
                  result = true;
              }
              jge->LeftClickedProcessed();
          }
      }
      return result;
    }
    }

    return false;
}

void CardDisplay::Render(bool norect)
{
    //norect - code shop
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    JRenderer * r = JRenderer::GetInstance();
    //if(norect)
       // r->FillRect(0,0,SCREEN_WIDTH_F,SCREEN_HEIGHT_F,ARGB(180,5,5,5));

    if(norect)
    {
        //info
        r->FillRect(static_cast<float> (x), static_cast<float> (10), static_cast<float> (nb_displayed_items * 30 + 20), 192,
                    ARGB(200,5,5,5));
        r->DrawRect(static_cast<float> (x), static_cast<float> (10), static_cast<float> (nb_displayed_items * 30 + 20), 192,
                    ARGB(255,240,240,240));
        r->DrawRect(static_cast<float> (x)+1, static_cast<float> (10)+1, static_cast<float> (nb_displayed_items * 30 + 20)-2, 192-2,
                    ARGB(255,89,89,89));

        //navi
        r->FillRect(static_cast<float> (x), static_cast<float> (y), static_cast<float> (nb_displayed_items * 30 + 20), 50,
                    ARGB(200,5,5,5));
        r->DrawRect(static_cast<float> (x), static_cast<float> (y), static_cast<float> (nb_displayed_items * 30 + 20), 50,
                    ARGB(255,240,240,240));
        r->DrawRect(static_cast<float> (x)+1, static_cast<float> (y)+1, static_cast<float> (nb_displayed_items * 30 + 20)-2, 50-2,
                    ARGB(255,89,89,89));
    }
    else
        r->DrawRect(static_cast<float> (x), static_cast<float> (y), static_cast<float> (nb_displayed_items * 30 + 20), 50,
                    ARGB(255,255,255,255));
    if (!mObjects.size()) return;
    for (int i = start_item; i < start_item + nb_displayed_items && i < (int)(mObjects.size()); i++)
    {
        if (mObjects[i])
        {
            mObjects[i]->Render();
            if (tc)
            {
                CardGui * cardg = (CardGui *) mObjects[i];
                if (tc->alreadyHasTarget(cardg->card))
                {
                    r->DrawCircle(cardg->x + 5, cardg->y + 5, 5, ARGB(255,255,0,0));
                }
                else if (!tc->canTarget(cardg->card))
                {
                    r->FillRect(cardg->x, cardg->y, 30, 40, ARGB(200,0,0,0));
                }
            }
        }
    }

    //TODO: CardSelector should handle the graveyard and the library in the future...
    if (mObjects.size() && mObjects[mCurr] != NULL)
    {
        mObjects[mCurr]->Render();
        CardGui * cardg = ((CardGui *) mObjects[mCurr]);
        //Pos pos = Pos(CardGui::BigWidth / 2, CardGui::BigHeight / 2 - 10, 1.0, 0.0, 220);
        Pos pos = Pos((CardGui::BigWidth / 2), CardGui::BigHeight / 2 - 10, 0.80f, 0.0, 220);
        
        if(norect)
            pos = Pos((CardGui::BigWidth / 2), CardGui::BigHeight / 2 - 7, 1.0, 0.0, 220);

        int drawMode = DrawMode::kNormal;
        if (observer)
        {
            //pos.actY = 145;
            pos.actY = 142;//reduce y a little
            if (x < (CardGui::BigWidth / 2)) pos.actX = SCREEN_WIDTH - 10 - CardGui::BigWidth / 2;
            drawMode = observer->getCardSelector()->GetDrawMode();
        }
        if(norect)
        {
            mFont->SetColor(ARGB(255,240,230,140));
            mFont->SetScale(1.5f);
            mFont->DrawString(cardg->card->data->name.c_str(),SCREEN_WIDTH_F/2,20);
            mFont->SetColor(ARGB(255,255,255,255));
            mFont->SetScale(1.0f);
            string details = "";
            std::vector<string> txt = cardg->card->data->getFormattedText(true);

            for (std::vector<string>::const_iterator it = txt.begin(); it != txt.end(); ++it)
            {
                details.append("\n");
                details.append(it->c_str());
            }
            mFont->DrawString(details.c_str(),SCREEN_WIDTH_F/2,25);
        }
        cardg->DrawCard(pos, drawMode);
    }
}

ostream& CardDisplay::toString(ostream& out) const
{
    return (out << "CardDisplay ::: x,y : " << x << "," << y << " ; start_item : " << start_item << " ; nb_displayed_items "
                    << nb_displayed_items << " ; tc : " << tc << " ; listener : " << listener);
}

std::ostream& operator<<(std::ostream& out, const CardDisplay& m)
{
    return m.toString(out);
}
