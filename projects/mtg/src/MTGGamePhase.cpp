#include "PrecompiledHeader.h"

#include "MTGGamePhase.h"
#include "GuiPhaseBar.h"

MTGGamePhase::MTGGamePhase(int id) :
    ActionElement(id)
{
    animation = 0;
    currentState = -1;
    mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetBase(0); // using 2nd font
}

void MTGGamePhase::Update(float dt)
{

    int newState = GameObserver::GetInstance()->getCurrentGamePhase();
    if (newState != currentState)
    {
        activeState = ACTIVE;
        animation = 4;
        currentState = newState;
    }

    if (animation > 0)
    {
        animation--;
    }
    else
    {
        activeState = INACTIVE;
        animation = 0;

    }

}

bool MTGGamePhase::CheckUserInput(JButton key)
{
    GameObserver * game = GameObserver::GetInstance();
    if (activeState == INACTIVE)
    {
        int x1,y1;
        JButton trigger = (options[Options::REVERSETRIGGERS].number ? JGE_BTN_NEXT : JGE_BTN_PREV);
        if(JGE::GetInstance()->GetLeftClickCoordinates(x1, y1))
        {
          if(x1 < 28 && y1 <185 && y1 > 106)
          { /* See GuiPhaseBar to understand where those values come from */
            GuiPhaseBar::GetInstance()->Zoom(float(1.4));
            if(key == JGE_BTN_OK)
            {
              key = trigger;
              JGE::GetInstance()->LeftClickedProcessed();
            }
          }
          else
          {
            GuiPhaseBar::GetInstance()->Zoom(1.0);
          }
        }

        if ((trigger == key) && game->currentActionPlayer == game->currentlyActing())
        {
            activeState = ACTIVE;
            game->userRequestNextGamePhase();
            return true;
        }
    }
    return false;
}

MTGGamePhase * MTGGamePhase::clone() const
{
    return NEW MTGGamePhase(*this);
}

ostream& MTGGamePhase::toString(ostream& out) const
{
    return out << "MTGGamePhase ::: animation " << animation << " ; currentState : " << currentState;
}
