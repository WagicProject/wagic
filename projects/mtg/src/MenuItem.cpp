#include "../include/config.h"
#include "../include/MenuItem.h"
#include "../include/GameOptions.h"
#include "../include/Translate.h"

MenuItem::MenuItem(int id, JLBFont *font, string text, int x, int y, JQuad * _off, JQuad * _on, const char * particle, JQuad * particleTex, bool hasFocus): JGuiObject(id), mFont(font), mX(x), mY(y)
{
  mText = _(text);
  updatedSinceLastRender = 1;
  mParticleSys = NEW hgeParticleSystem(resources.RetrievePSI(particle, particleTex));
  mParticleSys->MoveTo(mX, mY);

  mHasFocus = hasFocus;
  lastDt = 0.001f;
  mScale = 1.0f;
  mTargetScale = 1.0f;

  onQuad = _on;
  offQuad = _off;

  if (hasFocus)
    Entering();
}


void MenuItem::Render()
{
  JRenderer * renderer = JRenderer::GetInstance();


  if (mHasFocus)
    {
      PIXEL_TYPE start = ARGB(46,255,255,200);
      if(mParticleSys)
        start = mParticleSys->info.colColorStart.GetHWColor();

      PIXEL_TYPE colors[] =
      {
        ARGB(0,0,0,0),
        start,
        ARGB(0,0,0,0),
        start,
      }; 
      renderer->FillRect(255,0,SCREEN_WIDTH-155,SCREEN_HEIGHT,colors); 
      // set additive blending
      renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
      mParticleSys->Render();
      // set normal blending
      renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
      mFont->SetColor(ARGB(255,255,255,255));
      onQuad->SetColor(ARGB(70,255,255,255));
      renderer->RenderQuad(onQuad, SCREEN_WIDTH  , SCREEN_HEIGHT/2 , 0,8,8);
      onQuad->SetColor(ARGB(255,255,255,255));
      mFont->DrawString(mText.c_str(), SCREEN_WIDTH/2, 20 + 3*SCREEN_HEIGHT/4, JGETEXT_CENTER);
      renderer->RenderQuad(onQuad, mX  , mY , 0,mScale,mScale);

    }
  else
    {
      renderer->RenderQuad(offQuad, mX  , mY , 0,mScale,mScale);
    }
  updatedSinceLastRender= 0;
}

void MenuItem::Update(float dt)
{
  updatedSinceLastRender = 1;
  lastDt = dt;
  if (mScale < mTargetScale)
    {
      mScale += 8.0f*dt;
      if (mScale > mTargetScale)
	mScale = mTargetScale;
    }
  else if (mScale > mTargetScale)
    {
      mScale -= 8.0f*dt;
      if (mScale < mTargetScale)
	mScale = mTargetScale;
    }

    mParticleSys->Update(dt);
}




void MenuItem::Entering()
{

  mParticleSys->Fire();
  mHasFocus = true;
  mTargetScale = 1.3f;
}


bool MenuItem::Leaving(JButton key)
{
  mParticleSys->Stop(true);
  mHasFocus = false;
  mTargetScale = 1.0f;
  return true;
}


bool MenuItem::ButtonPressed()
{
  return true;
}


MenuItem::~MenuItem(){
  if (mParticleSys) delete mParticleSys;
}

ostream& MenuItem::toString(ostream& out) const
{
  return out << "MenuItem ::: mHasFocus : " << mHasFocus
	     << " ; mFont : " << mFont
	     << " ; mText : " << mText
	     << " ; mX,mY : " << mX << "," << mY
	     << " ; updatedSinceLastRender : " << updatedSinceLastRender
	     << " ; lastDt : " << lastDt
	     << " ; mScale : " << mScale
	     << " ; mTargetScale : " << mTargetScale
	     << " ; onQuad : " << onQuad
	     << " ; offQuad : " << offQuad
	     << " ; mParticleSys : " << mParticleSys;
}
