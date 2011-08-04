#include "PrecompiledHeader.h"

#include "MTGGamePhase.h"
#include "GuiPhaseBar.h"

MTGGamePhase* MTGGamePhase::instance = 0;

MTGGamePhase::MTGGamePhase(int id) :
    ActionElement(id)
{
    animation = 0;
    currentState = -1;
    mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetBase(0); // using 2nd font
    instance = this;
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

bool MTGGamePhase::NextGamePhase()
{
    GameObserver * game = GameObserver::GetInstance();
    if (activeState == INACTIVE)
    {
        if (game->currentActionPlayer == game->currentlyActing())
        {
            activeState = ACTIVE;
            game->userRequestNextGamePhase();
            return true;
        }
    }
    return false;
}


bool MTGGamePhase::CheckUserInput(JButton key)
{
    JButton trigger = (options[Options::REVERSETRIGGERS].number ? JGE_BTN_NEXT : JGE_BTN_PREV);
    if (trigger == key)
    {
        return NextGamePhase();
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
