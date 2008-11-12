#include "../include/debug.h"
#include "../include/SimpleMenu.h"
#include "../include/SimpleMenuItem.h"

SimpleMenu::SimpleMenu(int id, JGuiListener* listener, JLBFont* font, int x, int y, int width, const char * _title): JGuiController(id, listener){
  mHeight = 0;
  mWidth = width;
  mX = x;
  mY = y;
  mFont = font;
  if (_title){
    displaytitle = 1;
    title = _title;
    mHeight = 20;
  }else{
    displaytitle = 0;
  }
}

void SimpleMenu::Render(){
  mFont->SetColor(ARGB(255,255,255,255));
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(70,0,0,0));
  renderer->FillRoundRect(mX,mY,mWidth,mHeight,10,ARGB(255,17,17,17));
  renderer->FillRoundRect(mX+2,mY+2,mWidth - 4,mHeight-4,10,ARGB(255,62,62,62));
  if (displaytitle)
    mFont->DrawString(title.c_str(), mX+10, mY+10);
  JGuiController::Render();
}

void SimpleMenu::Add(int id, const char * text){
  JGuiController::Add(NEW SimpleMenuItem(id, mFont, text, mWidth/2 + mX + 10, mY + 10 + mHeight,  (mCount == 0)));
  mHeight += 20;
}
