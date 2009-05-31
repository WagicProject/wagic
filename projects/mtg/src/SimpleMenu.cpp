#include "../include/config.h"
#include "../include/SimpleMenu.h"
#include "../include/SimpleMenuItem.h"
#include "JTypes.h"
#include "../include/GameApp.h"
#include "../include/Translate.h"

const unsigned SimpleMenu::SIDE_SIZE = 7;
const unsigned SimpleMenu::VMARGIN = 16;
const unsigned SimpleMenu::HMARGIN = 30;
const signed SimpleMenu::LINE_HEIGHT = 20;
const char* SimpleMenu::spadeLPath = "graphics/spade_ul.png";
const char* SimpleMenu::spadeRPath = "graphics/spade_ur.png";
const char* SimpleMenu::jewelPath = "graphics/jewel.png";
const char* SimpleMenu::sidePath = "graphics/menuside.png";
const char* SimpleMenu::titleFontPath = "graphics/smallface";

JQuad* SimpleMenu::spadeR = NULL;
JQuad* SimpleMenu::spadeL = NULL;
JQuad* SimpleMenu::jewel = NULL;
JQuad* SimpleMenu::side = NULL;
JTexture* SimpleMenu::spadeRTex = NULL;
JTexture* SimpleMenu::spadeLTex = NULL;
JTexture* SimpleMenu::jewelTex = NULL;
JTexture* SimpleMenu::sideTex = NULL;
JLBFont* SimpleMenu::titleFont = NULL;
hgeParticleSystem* SimpleMenu::stars = NULL;
unsigned int SimpleMenu::refCount = 0;
// Here comes the magic of jewel graphics
PIXEL_TYPE SimpleMenu::jewelGraphics[9] = {0x3FFFFFFF,0x63645AEA,0x610D0D98,
					   0x63645AEA,0xFF635AD5,0xFF110F67,
					   0x610D0D98,0xFF110F67,0xFD030330};


SimpleMenu::SimpleMenu(int id, JGuiListener* listener, JLBFont* font, int x, int y, const char * _title, int _maxItems): JGuiController(id, listener){
  mHeight = 2 * VMARGIN;
  mWidth = 0;
  mX = x;
  mY = y;
  mFont = font;
  title = _(_title);
  startId = 0;
  maxItems = _maxItems;
  selectionT = 0;
  timeOpen = 0;
  closed = false;
  ++refCount;

  JRenderer* renderer = JRenderer::GetInstance();
  
  if (!spadeLTex)  spadeLTex= renderer->LoadTexture(spadeLPath, TEX_TYPE_USE_VRAM);
  if (!spadeRTex) spadeRTex = renderer->LoadTexture(spadeRPath, TEX_TYPE_USE_VRAM);
  if (!jewelTex)   jewelTex= renderer->CreateTexture(5, 5, TEX_TYPE_USE_VRAM);
  if (!sideTex)   sideTex = renderer->LoadTexture(sidePath, TEX_TYPE_USE_VRAM);
if (NULL == spadeL) spadeL = NEW JQuad(spadeLTex, 2, 1, 16, 13);
  if (NULL == spadeR) spadeR = NEW JQuad(spadeRTex, 2, 1, 16, 13);
  if (NULL == jewel)  jewel  = NEW JQuad(jewelTex, 1, 1, 3, 3);
  if (NULL == side)   side   = NEW JQuad(sideTex, 1, 1, 1, 7);

  if (NULL == titleFont)
    {
      GameApp::CommonRes->LoadJLBFont(titleFontPath, 7);
      titleFont = GameApp::CommonRes->GetJLBFont(titleFontPath);
    }
  if (NULL == stars) stars = NEW hgeParticleSystem("graphics/stars.psi", GameApp::CommonRes->GetQuad("stars"));

  stars->MoveTo(mX, mY);
}

void SimpleMenu::drawHorzPole(int x, int y, int width)
{
  JRenderer* renderer = JRenderer::GetInstance();

  renderer->RenderQuad(side, x + 5 , y - SIDE_SIZE / 2, 0, width - 10);
  spadeR->SetHFlip(true);
  spadeL->SetHFlip(false);
  renderer->RenderQuad(spadeR, x - 9, y - 6);
  renderer->RenderQuad(spadeL, x + width - 5, y - 6);

  renderer->RenderQuad(jewel, x, y - 1);
  renderer->RenderQuad(jewel, x + width - 1, y - 1);
}
void SimpleMenu::drawVertPole(int x, int y, int height)
{
  JRenderer* renderer = JRenderer::GetInstance();

  renderer->RenderQuad(side, x - SIDE_SIZE / 2, y + height - 5, -M_PI/2, height - 10);
  spadeR->SetHFlip(false);
  spadeL->SetHFlip(true);
  renderer->RenderQuad(spadeR, x - 6, y + 7, -M_PI/2);
  renderer->RenderQuad(spadeL, x - 6, y + height + 11, -M_PI/2);

  renderer->RenderQuad(jewel, x - 1, y - 1);
  renderer->RenderQuad(jewel, x - 1, y + height - 1);
}

void SimpleMenu::Render(){
  if (0 == mWidth)
    {
      for (int i = startId; i < startId + mCount; ++i)
	{
	  int width = (static_cast<SimpleMenuItem*>(mObjects[i]))->GetWidth();
	  if (mWidth < width) mWidth = width;
	}
      if ((!title.empty()) && (mWidth < titleFont->GetStringWidth(title.c_str()))) mWidth = titleFont->GetStringWidth(title.c_str());
      mWidth += 2*HMARGIN;
      for (int i = startId; i < startId + mCount; ++i)
	static_cast<SimpleMenuItem*>(mObjects[i])->Relocate(mX + mWidth / 2, mY + VMARGIN + i * LINE_HEIGHT);
      stars->Fire();
      selectionTargetY = selectionY = mY + VMARGIN;
      timeOpen = 0;
    }
  mFont->SetColor(ARGB(255,255,255,255));
  JRenderer * renderer = JRenderer::GetInstance();

  float height = mHeight;
  if (timeOpen < 1)
    height *= timeOpen > 0 ? timeOpen : -timeOpen;

  renderer->FillRect(mX, mY, mWidth, height, ARGB(180,0,0,0));

  drawVertPole(mX, mY - 16, height + 32);
  drawVertPole(mX + mWidth, mY - 16, height + 32);
  drawHorzPole(mX - 16, mY, mWidth + 32);

  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
  stars->Render();
  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

  mFont->SetScale(1.0f);
  if (!title.empty())
    titleFont->DrawString(title.c_str(), mX+mWidth/2, mY - 3, JGETEXT_CENTER);
  for (int i = startId; i < startId + maxItems ; i++){
    if (i > mCount-1) break;
    if ((static_cast<SimpleMenuItem*>(mObjects[i]))->mY - LINE_HEIGHT * startId < mY + height - LINE_HEIGHT + 7)
      {
        if (static_cast<SimpleMenuItem*>(mObjects[i])->hasFocus()){
          GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT)->DrawString(static_cast<SimpleMenuItem*>(mObjects[i])->desc.c_str(),mX+mWidth+10,mY+15);
	        mFont->SetColor(ARGB(255,255,255,0));
        }
	else
	  mFont->SetColor(ARGB(255,255,255,255));
	(static_cast<SimpleMenuItem*>(mObjects[i]))->RenderWithOffset(-LINE_HEIGHT*startId);
      }
  }

  drawHorzPole(mX - 25, mY + height, mWidth + 50);
}

void SimpleMenu::Update(float dt){
  JGuiController::Update(dt);
  if (mCurr > startId + maxItems-1){
    startId = mCurr - maxItems +1;
  }else if (mCurr < startId){
    startId = mCurr;
  }
  stars->Update(dt);
  selectionT += 3*dt;
  selectionY += (selectionTargetY - selectionY) * 8 * dt;
  stars->MoveTo(mX + HMARGIN + ((mWidth-2*HMARGIN)*(1+cos(selectionT))/2), selectionY + 5 * cos(selectionT*2.35) + LINE_HEIGHT / 2 - LINE_HEIGHT * startId);
  if (timeOpen < 0)
    {
      timeOpen += dt * 10;
      if (timeOpen >= 0) { timeOpen = 0; closed = true; }
    }
  else
    {
      closed = false;
      timeOpen += dt * 10;
    }
}

void SimpleMenu::Add(int id, const char * text,string desc){
  SimpleMenuItem * smi = NEW SimpleMenuItem(this, id, mFont, text, 0, mY + VMARGIN + mCount*LINE_HEIGHT, (mCount == 0));
  smi->desc = desc;
  JGuiController::Add(smi);
  if (mCount <= maxItems) mHeight += LINE_HEIGHT;
}

void SimpleMenu::Close()
{
  timeOpen = -1.0;
  stars->Stop(true);
}

void SimpleMenu::destroy(){
  SAFE_DELETE(SimpleMenu::spadeR);
  SAFE_DELETE(SimpleMenu::spadeL);
  SAFE_DELETE(SimpleMenu::jewel);
  SAFE_DELETE(SimpleMenu::side);
  SAFE_DELETE(SimpleMenu::spadeRTex);
  SAFE_DELETE(SimpleMenu::spadeLTex);
  SAFE_DELETE(SimpleMenu::jewelTex);
  SAFE_DELETE(SimpleMenu::sideTex);
  SAFE_DELETE(SimpleMenu::stars);
}