#include "../include/config.h"
#include "../include/Trash.h"
#include "../include/GuiStatic.h"

GuiStatic::GuiStatic(float desiredHeight, float x, float y, bool hasFocus, GuiAvatars* parent) : PlayGuiObject(desiredHeight, x, y, hasFocus), parent(parent) {}
void GuiStatic::Entering()
{
  parent->Activate(this);
}
bool GuiStatic::Leaving(u32 key)
{
  parent->Deactivate(this);
  return false;
}

GuiAvatar::GuiAvatar(float x, float y, bool hasFocus, Player * player, Corner corner, GuiAvatars* parent) : GuiStatic(GuiAvatar::Height, x, y, hasFocus, parent), avatarRed(255), currentLife(player->life), corner(corner), player(player) {
  type = GUI_AVATAR;
}

void GuiAvatar::Render()
{
  GameObserver * game = GameObserver::GetInstance();
  JRenderer * r = JRenderer::GetInstance();
  int life = player->life;
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  //Avatar
  int lifeDiff = life - currentLife;
  if (lifeDiff < 0 && currentLife > 0){
    avatarRed = 192 + (3* 255  * lifeDiff) / currentLife / 4;
    if (avatarRed < 0) avatarRed = 0;
  }
  currentLife = life;

  r->FillRect(actX+2, actY+2, Width * actZ, Height *actZ, ARGB((int)(actA / 2), 0, 0, 0));

  JQuad * quad = player->mAvatar;
  if (quad)
    {
      switch (corner)
	{
	case TOP_LEFT : quad->SetHotSpot(0, 0); break;
	case BOTTOM_RIGHT : quad->SetHotSpot(35, 50); break;
	}
      quad->SetColor(ARGB((int)actA, 255, avatarRed, avatarRed));
      r->RenderQuad(quad, actX, actY, actT, actZ, actZ);
      if (mHasFocus){
        switch (corner)
	        {
	        case TOP_LEFT : 
            r->FillRect(actX,actY,quad->mWidth * actZ,quad->mHeight  * actZ, ARGB(abs(128 - wave),255,255,255));
            break;
	        case BOTTOM_RIGHT : 
            r->FillRect(actX - quad->mWidth * actZ,actY - quad->mHeight  * actZ,quad->mWidth * actZ,quad->mHeight  * actZ, ARGB(abs(128 - wave),255,255,255)); 
            break;
	        }
      }
    }

  if (avatarRed < 255){
    avatarRed += 3;
    if (avatarRed > 255)
      avatarRed = 255;
  }
  if (game->currentPlayer == player)
    r->DrawRect(actX-1, actY-1, 37 * actZ, 52*actZ, ARGB((int)actA, 0, 255, 0));
  else if (game->currentActionPlayer == player)
    r->DrawRect(actX, actY, 35 *actZ, 50 * actZ, ARGB((int)actA, 0, 0, 255));
  if (game->isInterrupting == player)
    r->DrawRect(actX, actY, 35 * actZ, 50*actZ, ARGB((int)actA, 255, 0, 0));



  //Life
  char buffer[5];
  sprintf(buffer, "%i",life);
  switch (corner)
    {
    case TOP_LEFT :
      mFont->SetColor(ARGB((int)actA / 4, 0, 0, 0));
      mFont->DrawString(buffer, actX+2, actY+2);
      mFont->SetColor(ARGB((int)actA, 255, 255, 255));
      mFont->DrawString(buffer, actX+1, actY+1);
      break;
    case BOTTOM_RIGHT :
      mFont->SetColor(ARGB((int)actA, 255, 255, 255));
      mFont->DrawString(buffer, actX, actY-10, JGETEXT_RIGHT);
      break;
    }
  PlayGuiObject::Render();
}

ostream& GuiAvatar::toString(ostream& out) const
{
  return out << "GuiAvatar ::: avatarRed : " << avatarRed
	     << " ; currentLife : " << currentLife
	     << " ; player : " << player;
}


void GuiGameZone::toggleDisplay(){
  if (showCards)
    showCards = 0;
  else
    {
      showCards = 1;
      cd->init(zone);
    }
}


void GuiGameZone::Render(){
  //Texture
  JQuad * quad = resources.GetQuad("back_thumb");
  float scale = defaultHeight / quad->mHeight;
  quad->SetColor(ARGB((int)(actA),255,255,255));

  JRenderer::GetInstance()->RenderQuad(quad, actX, actY, 0.0, scale * actZ, scale * actZ);

  float x0 = actX;
  if (x0 < SCREEN_WIDTH/2) {
    x0+=7;
  }

  if (mHasFocus)
    JRenderer::GetInstance()->FillRect(actX,actY,quad->mWidth * scale * actZ,quad->mHeight *scale * actZ, ARGB(abs(128 - wave),255,255,255));

  //Number of cards
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[11];
  sprintf(buffer,"%i", zone->nb_cards);
  mFont->SetColor(ARGB((int)(actA),0,0,0));
  mFont->DrawString(buffer, x0+1, actY+1);
  mFont->SetColor(ARGB(actA > 120 ? 255: (int)(actA),255,255,255));
  mFont->DrawString(buffer, x0, actY);

  

  if (showCards) cd->Render();
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    (*it)->Render();
  PlayGuiObject::Render();
}

void GuiGameZone::ButtonPressed(int controllerId, int controlId){
  GameObserver::GetInstance()->ButtonPressed(this);
}

bool GuiGameZone::CheckUserInput(u32 key){
  if (showCards) return cd->CheckUserInput(key);
  return false;
}

void GuiGameZone::Update(float dt){
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    (*it)->Update(dt);
  if (showCards) cd->Update(dt);
  PlayGuiObject::Update(dt);
}

GuiGameZone::GuiGameZone(float x, float y, bool hasFocus, MTGGameZone* zone, GuiAvatars* parent): GuiStatic(GuiGameZone::Height, x, y, hasFocus, parent), zone(zone){
  cd = NEW CardDisplay(0, GameObserver::GetInstance(), x, y, this);
  cd->zone = zone;
  showCards = 0;
}

GuiGameZone::~GuiGameZone(){
  if (cd) delete cd;
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    delete(*it);
}

ostream& GuiGameZone::toString(ostream& out) const
{
  return out << "GuiGameZone ::: zone : " << zone
	     << " ; cd : " << cd
	     << " ; showCards : " << showCards;
}

GuiGraveyard::GuiGraveyard(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) : GuiGameZone(x, y, hasFocus, player->game->graveyard, parent), player(player) {
  type = GUI_GRAVEYARD;
}

int GuiGraveyard::receiveEventPlus(WEvent* e)
{
  if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    if (event->to == zone)
      {
	CardView* t;
	if (event->card->view)
	  t = NEW CardView(CardSelector::nullZone, event->card, *(event->card->view));
	else
	  t = NEW CardView(CardSelector::nullZone, event->card, x, y);
	t->x = x + Width / 2; t->y = y + Height / 2; t->zoom = 0.6; t->alpha = 0;
	cards.push_back(t);
	return 1;
      }
  return 0;
}

int GuiGraveyard::receiveEventMinus(WEvent* e)
{
  if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    if (event->from == zone)
      for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
	if (event->card->previous == (*it)->card)
	  {
	    CardView* cv = *it;
	    cards.erase(it);
            trash(cv);
	    return 1;
	  }
  return 0;
}

ostream& GuiGraveyard::toString(ostream& out) const
{
  return out << "GuiGraveyard :::";
}

GuiLibrary::GuiLibrary(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) : GuiGameZone(x, y, hasFocus,player->game->library, parent), player(player) {
  type = GUI_LIBRARY;
}


ostream& GuiLibrary::toString(ostream& out) const
{
  return out << "GuiLibrary :::";
}
