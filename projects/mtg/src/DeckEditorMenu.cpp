#include "PrecompiledHeader.h"
#include "DeckEditorMenu.h"
#include "JTypes.h"

DeckEditorMenu::DeckEditorMenu(int id, JGuiListener* listener, int fontId, const char * _title)
: DeckMenu( id, listener, fontId, _title )
{
  backgroundName = "DeckEditorMenuBackdrop";

  mX = 120;
  mY = 70;
  starsOffsetX = 50;
  titleX = 110; // center point in title box
  titleY = 34;
  titleWidth = 180; // width of inner box of title

  descX = 275;
  descY = 80;
  descHeight = 154;
  descWidth = 175;
   
  statsX = 290;
  statsY = 15;
  statsHeight = 40;
  statsWidth = 180;

  avatarX = 222;
  avatarY = 8;

  float scrollerWidth = 80;
  SAFE_DELETE(scroller); // need to delete the scroller init in the base class
  scroller = NEW TextScroller(Fonts::MAIN_FONT, 40 , 230, scrollerWidth, 100, 1, 1);

}

void DeckEditorMenu::Render()
{
  JRenderer *r = JRenderer::GetInstance();
  r->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(200,0,0,0));
  DeckMenu::Render();
}

DeckEditorMenu::~DeckEditorMenu()
{
  SAFE_DELETE( scroller );
}
