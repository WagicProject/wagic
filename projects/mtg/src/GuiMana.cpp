#include <iostream>
#include "../include/GuiMana.h"

using std::cout;
using std::endl;


const float ManaIcon::DESTX = 440;
const float ManaIcon::DESTY = 20;

ManaIcon::ManaIcon(int color, float x, float y) : Pos(x, y, 0.5, 0.0, 255), f(-1), mode(ALIVE), color(color)
{
  hgeParticleSystemInfo * psi = NULL; 
  JQuad * mq = resources.GetQuad("stars");

  if(!mq){
    particleSys = NULL;
    return;
  }

  switch (color)
    {
    case Constants::MTG_COLOR_RED :
      psi = resources.RetrievePSI("manared.psi",mq);
      break;
    case Constants::MTG_COLOR_BLUE :
      psi = resources.RetrievePSI("manablue.psi",mq);
      break;
    case Constants::MTG_COLOR_GREEN :
      psi = resources.RetrievePSI("managreen.psi",mq);
      break;
    case Constants::MTG_COLOR_BLACK :
      psi = resources.RetrievePSI("manablack.psi",mq);
      break;
    case Constants::MTG_COLOR_WHITE :
      psi = resources.RetrievePSI("manawhite.psi",mq);
      break;
    default :
      psi = resources.RetrievePSI("mana.psi",mq);
    }

  if(!psi){
    psi = NEW hgeParticleSystemInfo();
    if(!psi)
      return;
    hgeParticleSystemInfo * defaults = resources.RetrievePSI("mana.psi",mq);
    if(defaults){
      memcpy(psi,defaults,sizeof(hgeParticleSystemInfo));
    }
    else{
      memset(psi,0,sizeof(hgeParticleSystemInfo));

      //Default values for particle system! Cribbed from mana.psi
      //Really, we should just be loading that and then changing colors...
      psi->nEmission = 114;
      psi->fLifetime = -1;
      psi->fParticleLifeMin = 1.1507937;
      psi->fParticleLifeMax = 1.4682540;
      psi->fSpeedMin = 0.0099999998;
      psi->fSizeStart = 0.5;
      psi->fSizeEnd = 0.69999999;
      psi->fSizeVar = 0.25396827;
      psi->fSpinStart = -5.5555553;
      psi->fAlphaVar = 0.77777779;
      psi->sprite = mq;
    }

    switch(color){
      case Constants::MTG_COLOR_RED :
        psi->colColorStart.SetHWColor(ARGB(161,240,40,44));
        psi->colColorEnd.SetHWColor(ARGB(14,242,155,153));
        break;
      case Constants::MTG_COLOR_BLUE :
        psi->colColorStart.SetHWColor(ARGB(161,28,40,224));
        psi->colColorEnd.SetHWColor(ARGB(14,255,255,255));
        break;
      case Constants::MTG_COLOR_GREEN :
        psi->colColorStart.SetHWColor(ARGB(161,36,242,44));
        psi->colColorEnd.SetHWColor(ARGB(14,129,244,153));
        break;
      case Constants::MTG_COLOR_BLACK :
        psi->colColorStart.SetHWColor(ARGB(161,210,117,210));
        psi->colColorEnd.SetHWColor(ARGB(14,80,56,80));
        break;
      case Constants::MTG_COLOR_WHITE :
        psi->colColorStart.SetHWColor(ARGB(151,151,127,38));
        psi->colColorEnd.SetHWColor(ARGB(8,255,255,255));
        break;
      default:
        psi->colColorStart.SetHWColor(ARGB(161,236,242,232));
        psi->colColorEnd.SetHWColor(ARGB(14,238,244,204));
        break;
    }
    
    particleSys = NEW hgeParticleSystem(psi);
    SAFE_DELETE(psi); //This version of psi is not handled by cache, so kill it here.
  }
  else
    particleSys = NEW hgeParticleSystem(psi); //Cache will clean psi up later.
  

  icon = manaIcons[color];

  particleSys->FireAt(x, y);

  zoomP1 = 0.2 + 0.1 * ((float)rand() / (float)RAND_MAX);
  zoomP2 = 0.2 + 0.1 * ((float)rand() / (float)RAND_MAX);
  zoomP3 = 2 * M_PI * ((float)rand() / (float)RAND_MAX);
  zoomP4 = 2 * M_PI * ((float)rand() / (float)RAND_MAX);
  zoomP5 = 0.5 + ((float)rand() / (float)RAND_MAX);
  zoomP6 = 0.5 + ((float)rand() / (float)RAND_MAX);

  xP1 = 2 * M_PI * ((float)rand() / (float)RAND_MAX);
  xP2 = 5 + 30 * ((float)rand() / (float)RAND_MAX);
  xP3 = 0.5 + ((float)rand() / (float)RAND_MAX);
  yP1 = 2 * M_PI * ((float)rand() / (float)RAND_MAX);
  yP2 = 5 + 10 * ((float)rand() / (float)RAND_MAX);
  yP3 = 0.5 + ((float)rand() / (float)RAND_MAX);

  actT = 0;
  tP1 = 0;
}

ManaIcon::~ManaIcon()
{
  SAFE_DELETE(particleSys);
}

void ManaIcon::Render()
{
  if(!particleSys)
    return; 

  JRenderer* renderer = JRenderer::GetInstance();

  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
  particleSys->Render();
  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
  renderer->RenderQuad(icon, actX, actY, actT, actZ + zoomP1 * sinf(M_PI * zoomP3), actZ + zoomP2 * cosf(M_PI * zoomP4));
}
void ManaIcon::Update(float dt)
{
  xP1 += xP3 * dt;
  actX = x + xP2 * sinf(M_PI * xP1);
  zoomP3 += zoomP5 * dt;
  zoomP4 += zoomP6 * dt;

  switch (mode)
    {
    case DROPPING :
      f += dt * 700;
      actY += f * dt;
      if (actY > SCREEN_HEIGHT * 2) mode = DEAD;
      break;
    case WITHERING :
      actT += dt * 4;
      actZ /= f; zoomP1 /= f; zoomP2 /= f;
      f -= dt;
      actZ *= f; zoomP1 *= f; zoomP2 *= f;
      yP1 += yP3 * dt;
      actY = y + yP2 * sinf(M_PI * yP1);
      if (f < 0) mode = DEAD;
      break;
    case ALIVE :
      x += 10 * dt * (DESTX - x);
      y += 10 * dt * (DESTY - y);
      yP1 += yP3 * dt;
      actY = y + yP2 * sinf(M_PI * yP1);
      break;
    case DEAD :
      break;
    }

  particleSys->MoveTo(actX, actY);
  particleSys->Update(dt);
}

void ManaIcon::Wither()
{
  mode = WITHERING;
  f = 1.0;
  particleSys->Stop();
}
void ManaIcon::Drop()
{
  mode = DROPPING;
  if (f < 0) f = 0;
  particleSys->Stop();
}

GuiMana::GuiMana()
{
}

GuiMana::~GuiMana(){
  for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it){
    delete(*it);
  }
}

void GuiMana::Render()
{
  for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
    (*it)->Render();
}
bool remove_dead(ManaIcon* m) { return ManaIcon::DEAD != m->mode; }
void GuiMana::Update(float dt)
{
  {
    for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
      (*it)->Update(dt);
  }
  vector<ManaIcon*>::iterator it = partition(manas.begin(), manas.end(), &remove_dead);
  if (it != manas.end())
    {
      for (vector<ManaIcon*>::iterator q = it; q != manas.end(); ++q)
	      SAFE_DELETE(*q);
      manas.erase(it, manas.end());
    }
}

int GuiMana::receiveEventPlus(WEvent* e)
{
  if (WEventEngageMana *event = dynamic_cast<WEventEngageMana*>(e))
    {
      if (event->card && event->card->view)
	      manas.push_back(NEW ManaIcon(event->color, event->card->view->actX, event->card->view->actY));
      else
	      manas.push_back(NEW ManaIcon(event->color, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
      return 1;
    }
  else return 0;
}

int GuiMana::receiveEventMinus(WEvent* e)
{
  if (WEventConsumeMana *event = dynamic_cast<WEventConsumeMana*>(e))
    {
      for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
	if ((event->color == (*it)->color) && (ManaIcon::ALIVE == (*it)->mode)) { (*it)->Wither(); return 1; }
      return 1;
    }
  else if (dynamic_cast<WEventEmptyManaPool*>(e))
    {
      for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
	      (*it)->Drop(); //TODO: split according to which manapool was emptied...
      return 1;
    }
  return 0;
}
