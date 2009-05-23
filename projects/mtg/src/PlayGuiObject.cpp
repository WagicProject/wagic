#include "../include/config.h"
#include "../include/PlayGuiObject.h"

#include "../include/Player.h"
#include "../include/MTGGameZones.h"
#include "../include/CardDisplay.h"

PlayGuiObject::PlayGuiObject(int id, float desiredHeight,float _x, float _y, bool hasFocus): JGuiObject(id){
  defaultHeight = desiredHeight;
  mHeight = desiredHeight;
  x = _x;
  y = _y;
  mHasFocus = hasFocus;
  type = 0;
  wave = 0;
}


void PlayGuiObject::Update(float dt){
  if (mHasFocus && mHeight < defaultHeight * 1.2)
    {
      mHeight += defaultHeight*0.8f*dt;
      //      fprintf(stderr, "increasing size to %f - %d", mHeight, GetId() );

      if (mHeight > defaultHeight * 1.2)
	mHeight = defaultHeight * 1.2;
    }
  else if (!mHasFocus && mHeight > defaultHeight)
    {
      mHeight -= defaultHeight*0.8f*dt;
      if (mHeight < defaultHeight)
	mHeight = defaultHeight;
    }
  wave = (wave +2) % 255;
}

GuiAvatar::GuiAvatar(int id, float desiredHeight,float _x, float _y, bool hasFocus, Player * _player): PlayGuiObject(id, desiredHeight, _x,  _y,  hasFocus){
  player= _player;
  avatarRed = 255;
  currentLife = player->life;
  type = GUI_AVATAR;
}

void GuiAvatar::Render(){
  GameObserver * game = GameObserver::GetInstance();
  JRenderer * r = JRenderer::GetInstance();
  int life = player->life;
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  //Avatar
  int lifeDiff = life - currentLife;
  if (lifeDiff < 0 && currentLife >0 ){
    avatarRed = 192 + (3* 255  * lifeDiff)/ currentLife / 4;
    if (avatarRed<0)
      avatarRed = 0;
  }
  currentLife= life;

  JQuad * quad = player->mAvatar;
  if(quad){
    quad->SetColor(ARGB(255,255,avatarRed,avatarRed));
    r->RenderQuad(quad,x,y);
    if (mHasFocus){
      r->FillRect(x,y,quad->mWidth,quad->mHeight,ARGB(abs(wave-128), 255,255,255));
    }
  }

  if (avatarRed < 255){
    avatarRed+=3;
    if (avatarRed >255)
      avatarRed = 255;
  }
  if(game->currentPlayer == player){
    r->DrawRect(x-1,y-1 ,37,52,ARGB(255,0,255,0));
  }else if (game->currentActionPlayer == player){
    r->DrawRect(x,y,35,50,ARGB(255,0,0,255));
  }
  if(game->isInterrupting == player){
    r->DrawRect(x,y ,35,50,ARGB(255,255,0,0));
  }

  //Life
  char buffer[5];
  sprintf(buffer, "%i",life);
  mFont->SetColor(ARGB(128,0,0,0));
  mFont->DrawString(buffer, x+3,y+40);
  mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(buffer, x+1,y+38);
}

ostream& GuiAvatar::toString(ostream& out) const
{
  return out << "GuiAvatar ::: avatarRed : " << avatarRed
	     << " ; currentLife : " << currentLife
	     << " ; player : " << player;
}


void GuiGameZone::toggleDisplay(){
  if (showCards){
    showCards = 0;
  }else{
    showCards = 1;
    cd->init(zone);
  }
}


void GuiGameZone::Render(){
  //Texture
  JQuad * quad = GameApp::CommonRes->GetQuad("back_thumb");
  float scale = defaultHeight / quad->mHeight;
  quad->SetColor(ARGB(255,255,255,255));

  JRenderer::GetInstance()->RenderQuad(quad,x,y,0.0,scale, scale);
  if (mHasFocus){
    JRenderer::GetInstance()->FillRect(x,y,quad->mWidth*scale,quad->mHeight*scale,ARGB(abs(wave-128), 255,255,255));
  }
  //Number of cards
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[512];
  sprintf(buffer,"%i", zone->nb_cards);
  mFont->SetColor(ARGB(128,0,0,0));
  mFont->DrawString(buffer, x+2, y+2);
  mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(buffer, x, y);

  if (showCards) cd->Render();
}

void GuiGameZone::ButtonPressed(int controllerId, int controlId){
  toggleDisplay();
}

void GuiGameZone::Update(float dt){
  if (showCards) cd->Update(dt);
  PlayGuiObject::Update(dt);
}

GuiGameZone::GuiGameZone(int id, float desiredHeight,float _x, float _y, bool hasFocus,MTGGameZone * _zone): PlayGuiObject(id, desiredHeight, _x,  _y,  hasFocus), zone(_zone){
  cd = NEW CardDisplay(id, GameObserver::GetInstance(), _x,  _y, this);
  showCards = 0;
}

GuiGameZone::~GuiGameZone(){
  if(cd) delete cd;
}

ostream& GuiGameZone::toString(ostream& out) const
{
  return out << "GuiGameZone ::: zone : " << zone
	     << " ; cd : " << cd
	     << " ; showCards : " << showCards;
}

GuiGraveyard::GuiGraveyard(int id, float desiredHeight,float _x, float _y, bool hasFocus,Player * player):GuiGameZone(id, desiredHeight, _x,  _y,  hasFocus,player->game->graveyard){
  type= GUI_GRAVEYARD;
}

ostream& GuiGraveyard::toString(ostream& out) const
{
  return out << "GuiGraveyard :::";
}

GuiLibrary::GuiLibrary(int id, float desiredHeight,float _x, float _y, bool hasFocus,Player * player):GuiGameZone(id, desiredHeight, _x,  _y,  hasFocus,player->game->library){
  type = GUI_LIBRARY;
}


ostream& GuiLibrary::toString(ostream& out) const
{
  return out << "GuiLibrary :::";
}
