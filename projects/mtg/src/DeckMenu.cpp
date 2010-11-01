#include "PrecompiledHeader.h"

#include "DeckMenu.h"
#include "DeckMenuItem.h"
#include "DeckMetaData.h"
#include "JTypes.h"
#include "GameApp.h"
#include "Translate.h"

namespace
{
  const unsigned int kPoleWidth = 7;
  const unsigned int kVerticalMargin = 16;
  const unsigned int kHorizontalMargin = 30;
  const signed int kLineHeight = 20;
}

WFont* DeckMenu::titleFont = NULL;
hgeParticleSystem* DeckMenu::stars = NULL;
unsigned int DeckMenu::refCount = 0;
// Here comes the magic of jewel graphics
PIXEL_TYPE DeckMenu::jewelGraphics[9] = {0x3FFFFFFF,0x63645AEA,0x610D0D98,
					   0x63645AEA,0xFF635AD5,0xFF110F67,
					   0x610D0D98,0xFF110F67,0xFD030330};

//
//  For the additional info window, maximum characters per line is roughly 30 characters across.
//  TODO: figure a way to get incoming text to wrap.
//
// used fixed locations where the menu, title and descriptive text are located.
//    * menu at (125, 60 )
//    * descriptive information 125

DeckMenu::DeckMenu(int id, JGuiListener* listener, int fontId, const char * _title)
: JGuiController(id, listener), 
fontId(fontId) {

  background = NULL;
  autoTranslate = true;
  maxItems = 7;
  mHeight = 2 * kVerticalMargin + ( maxItems * kLineHeight );
  mWidth = 0;
  mX = 125;
  mY = 60;

  // where to place the title of the menu
  titleX = mX;
  titleY = mY - 30;
  title = _(_title);

  // where stats information goes
  statsX = 280;
  statsY = 8 + kVerticalMargin;
  statsHeight = 50;
  statsWidth = SCREEN_WIDTH / 2 - 40; // 40 is the width of the right border 

  // where to place the descripiton information
  descX = 229;
  descY = 70;

  startId = 0;
  selectionT = 0;
  timeOpen = 0;
  closed = false;
  ++refCount;
  selectionTargetY = selectionY = kVerticalMargin;
      
  if (NULL == stars)
    stars = NEW hgeParticleSystem(resources.RetrievePSI("stars.psi", resources.GetQuad("stars")));

  stars->FireAt(mX, mY);
}

// TODO: Make this configurable, perhaps by user as part of the theme options.
JQuad* getBackground()
{
  resources.RetrieveTexture("DeckMenuBackdrop.png", RETRIEVE_MANAGE );
  return resources.RetrieveQuad("DeckMenuBackdrop.png", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, "DualPaneBG" );

}


void DeckMenu::Render() {
  JRenderer * renderer = JRenderer::GetInstance();

  WFont * titleFont = resources.GetWFont(Fonts::SMALLFACE_FONT);
  WFont * mFont = resources.GetWFont(fontId);

  // figure out where to place the stars initially
  if (0 == mWidth) {
    float sY = mY + kVerticalMargin;
    for (int i = startId; i < startId + mCount; ++i) {
      DeckMenuItem *menuItem = static_cast<DeckMenuItem *> (mObjects[i]);
      int width = menuItem->GetWidth();
      if (mWidth < width) mWidth = width;
    }
    if ((!title.empty()) && (mWidth < titleFont->GetStringWidth(title.c_str()))) 
      mWidth = titleFont->GetStringWidth(title.c_str());
    mWidth += 2*kHorizontalMargin;
    for (int i = startId; i < startId + mCount; ++i) {
      float y = mY + kVerticalMargin + i * kLineHeight;
      DeckMenuItem * currentMenuItem = static_cast<DeckMenuItem*>(mObjects[i]);
      currentMenuItem->Relocate( mX, y);

      if (currentMenuItem->hasFocus()) sY = y;
    }
    stars->Fire();
    selectionTargetY = selectionY = sY;
    timeOpen = 0;
  }


  renderer->RenderQuad(getBackground(), 0, 0 );

  float height = mHeight;
  if (timeOpen < 1) height *= timeOpen > 0 ? timeOpen : -timeOpen;

  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
  stars->Render();
  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

  mFont->SetScale(1.0f);

  for (int i = startId; i < startId + maxItems ; i++){
    if (i > mCount-1) break;
    if ( static_cast<DeckMenuItem*>(mObjects[i])->mY - kLineHeight * startId < mY + height - kLineHeight + 7) {
      DeckMenuItem *currentMenuItem = static_cast<DeckMenuItem*>(mObjects[i]);
      if ( currentMenuItem->hasFocus()){
        // display the avatar image
        if ( currentMenuItem->imageFilename.size() > 0 )
        {
          JQuad * quad = resources.RetrieveTempQuad( currentMenuItem->imageFilename, TEXTURE_SUB_AVATAR ); 
          if (quad) {
            renderer->RenderQuad(quad, 232, 10, 0, 0.9f, 0.9f);
          }      
        }
        // fill in the description part of the screen
        string text = currentMenuItem->desc;
        WFont *mainFont = resources.GetWFont(Fonts::MAIN_FONT);
        mainFont->DrawString(text.c_str(), descX, descY);
        mFont->SetColor(ARGB(255,255,255,0));

        // fill in the statistical portion
        if ( currentMenuItem->meta )
        {
          ostringstream oss;
          oss << "Deck: " << currentMenuItem->meta->getName() << endl;
          oss << currentMenuItem->meta->getStatsSummary();

          mainFont->DrawString( oss.str(), statsX, statsY - kVerticalMargin );
        }

        // fill in the bottom section of the screen.
        // TODO:  add text scroller of Tasks.
      } 
      else {
        mFont->SetColor(ARGB(150,255,255,255));
      }
      currentMenuItem->RenderWithOffset(-kLineHeight*startId);
    }
    if (!title.empty())
      titleFont->DrawString(title.c_str(), titleX, titleY, JGETEXT_CENTER);

  }
}

void DeckMenu::Update(float dt){
  JGuiController::Update(dt);
  if (mCurr > startId + maxItems-1)
    startId = mCurr - maxItems +1;
  else if (mCurr < startId)
    startId = mCurr;
  stars->Update(dt);
  selectionT += 3*dt;
  selectionY += (selectionTargetY - selectionY) * 8 * dt;
  stars->MoveTo( 40 + ((mWidth-2*kHorizontalMargin)*(1+cos(selectionT))/2), selectionY + 5 * cos(selectionT*2.35) + kLineHeight / 2 - kLineHeight * startId);
  if (timeOpen < 0) {
    timeOpen += dt * 10;
    if (timeOpen >= 0) { timeOpen = 0; closed = true; stars->FireAt(mX, mY); }
  } else {
    closed = false;
    timeOpen += dt * 10;
  }
}

void DeckMenu::Add(int id, const char * text,string desc, bool forceFocus, DeckMetaData * deckMetaData) {
  DeckMenuItem * menuItem = NEW DeckMenuItem(this, id, fontId, text, 0, mY + kVerticalMargin + mCount*kLineHeight, (mCount == 0), autoTranslate, deckMetaData);
  menuItem->desc = deckMetaData ? deckMetaData->getDescription() : desc;

  JGuiController::Add(menuItem);
  if (mCount <= maxItems) mHeight += kLineHeight;
  if (forceFocus){
    mObjects[mCurr]->Leaving(JGE_BTN_DOWN);
    mCurr = mCount-1;
    menuItem->Entering();
  }
}

void DeckMenu::Close()
{
  timeOpen = -1.0;
  stars->Stop(true);
}


void DeckMenu::destroy(){
  SAFE_DELETE(DeckMenu::stars);
}
