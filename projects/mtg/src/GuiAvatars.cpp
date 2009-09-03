#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiAvatars.h"

GuiAvatars::GuiAvatars(CardSelector* cs) : cs(cs), active(NULL)
{
  Add(self           = NEW GuiAvatar   (SCREEN_WIDTH, SCREEN_HEIGHT, false,
					GameObserver::GetInstance()->players[0], GuiAvatar::BOTTOM_RIGHT, this));
  Add(selfGraveyard  = NEW GuiGraveyard(SCREEN_WIDTH - GuiAvatar::Width - GuiGameZone::Width/2, SCREEN_HEIGHT - GuiAvatar::Height - 10, false,
					GameObserver::GetInstance()->players[0], this));
  Add(selfLibrary    = NEW GuiLibrary  (SCREEN_WIDTH - GuiAvatar::Width - GuiGameZone::Width/2, SCREEN_HEIGHT - GuiAvatar::Height - 10 + GuiGameZone::Height + 5, false,
					GameObserver::GetInstance()->players[0], this));

  Add(opponent          = NEW GuiAvatar   (0,  0,  false, GameObserver::GetInstance()->players[1], GuiAvatar::TOP_LEFT, this));
  Add(opponentGraveyard = NEW GuiGraveyard(GuiAvatar::Width - GuiGameZone::Width / 2, 5,  false, GameObserver::GetInstance()->players[1], this));
  Add(opponentLibrary   = NEW GuiLibrary  (GuiAvatar::Width - GuiGameZone::Width / 2, 5 + GuiGameZone::Height + 5, false, GameObserver::GetInstance()->players[1], this));

  cs->Add(self); cs->Add(selfGraveyard); cs->Add(selfLibrary);
  cs->Add(opponent); cs->Add(opponentGraveyard); cs->Add(opponentLibrary);
  selfGraveyard->alpha = selfLibrary->alpha = opponentGraveyard->alpha = opponentLibrary->alpha = 0;
}

GuiAvatars::~GuiAvatars()
{
}

void GuiAvatars::Activate(PlayGuiObject* c)
{
  if ((opponentGraveyard == c) || (opponentLibrary == c) || (opponent == c))
    { opponentGraveyard->alpha = opponentLibrary->alpha = 255; active = opponent; }
  else if ((selfGraveyard == c) || (selfLibrary == c) || (self == c))
    { selfGraveyard->alpha = selfLibrary->alpha = 255; self->zoom = 1.0; active = self; }
}
void GuiAvatars::Deactivate(PlayGuiObject* c)
{
  if ((opponentGraveyard == c) || (opponentLibrary == c) || (opponent == c))
    { opponentGraveyard->alpha = opponentLibrary->alpha = 0; }
  else if ((selfGraveyard == c) || (selfLibrary == c) || (self == c))
    { selfGraveyard->alpha = selfLibrary->alpha = 0; self->zoom = 0.3; }
  active = NULL;
}

int GuiAvatars::receiveEventPlus(WEvent* e)
{
  return selfGraveyard->receiveEventPlus(e) | opponentGraveyard->receiveEventPlus(e);
}

int GuiAvatars::receiveEventMinus(WEvent* e)
{
  selfGraveyard->receiveEventMinus(e);
  opponentGraveyard->receiveEventMinus(e);
  return 1;
}


void GuiAvatars::Update(float dt)
{
  self->Update(dt);
  opponent->Update(dt);
  selfGraveyard->Update(dt);
  opponentGraveyard->Update(dt);
  selfLibrary->Update(dt);
  opponentLibrary->Update(dt);
}
