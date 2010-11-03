#include "PrecompiledHeader.h"

#include "DeckMenu.h"
#include "DeckMenuItem.h"
#include "DeckMetaData.h"
#include "JTypes.h"
#include "GameApp.h"
#include "Translate.h"
#include "TextScroller.h"
#include "Tasks.h"
#include <iomanip>
namespace
{
  const unsigned int kVerticalMargin = 16;
  const unsigned int kHorizontalMargin = 30;
  const signed int kLineHeight = 20;
  const signed int kDescriptionVerticalBoxPadding = 5;
  const signed int kDescriptionHorizontalBoxPadding = 5;
}

hgeParticleSystem* DeckMenu::stars = NULL; 
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
//    *** Need to make this configurable in a file somewhere to allow for class reuse

DeckMenu::DeckMenu(int id, JGuiListener* listener, int fontId, const string _title)
: JGuiController(id, listener), 
fontId(fontId) {

 backgroundName = "DeckMenuBackdrop";

  mX = 125;
  mY = 55;
  
  titleX = 125; // center point in title box
  titleY = 28;
  titleWidth = 180; // width of inner box of title

  descX = 230 + kDescriptionVerticalBoxPadding;
  descY = 65 + kDescriptionHorizontalBoxPadding;
  descHeight = 145;
  descWidth = 220;
   
  statsX = 280;
  statsY = 8;
  statsHeight = 50;
  statsWidth = 227;


  avatarX = 230;
  avatarY = 8;

  int scrollerWidth = 80;

  scroller = NEW TextScroller(Fonts::MAIN_FONT, 40 , 230, scrollerWidth, 100, 1, 1);

  autoTranslate = true;
  maxItems = 7;
  
  mHeight = 2 * kVerticalMargin + ( maxItems * kLineHeight );
  mWidth = 0;

  // we want to cap the deck titles to 15 characters to avoid overflowing deck names
  title = _(_title);

  titleFont = resources.GetWFont(Fonts::MAGIC_FONT);
  startId = 0;
  selectionT = 0;
  timeOpen = 0;
  closed = false;

  selectionTargetY = selectionY = kVerticalMargin;
      
  if (NULL == stars)
    stars = NEW hgeParticleSystem(resources.RetrievePSI("stars.psi", resources.GetQuad("stars")));
  stars->FireAt(mX, mY);

  updateScroller();
}

// TODO: Make this configurable, perhaps by user as part of the theme options.
JQuad* DeckMenu::getBackground()
{
  ostringstream bgFilename;
  bgFilename << backgroundName << ".png";
  resources.RetrieveTexture( bgFilename.str(), RETRIEVE_MANAGE );
  return resources.RetrieveQuad(bgFilename.str(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, backgroundName );

}

void DeckMenu::initMenuItems()
{
  float sY = mY + kVerticalMargin;
  for (int i = startId; i < startId + mCount; ++i) {
    DeckMenuItem *menuItem = static_cast<DeckMenuItem *> (mObjects[i]);
    int width = menuItem->GetWidth();
    if (mWidth < width) mWidth = width;
  }
  titleWidth = titleFont->GetStringWidth(title.c_str());
  if ((!title.empty()) && (mWidth < titleWidth)) 
    mWidth = titleWidth;

  mWidth += 2*kHorizontalMargin;
  for (int i = startId; i < startId + mCount; ++i) {
    float y = mY + kVerticalMargin + i * kLineHeight;
    DeckMenuItem * currentMenuItem = static_cast<DeckMenuItem*>(mObjects[i]);
    currentMenuItem->Relocate( mX, y);
    if (currentMenuItem->hasFocus()) 
      sY = y;
  }
  selectionTargetY = selectionY = sY;
}

void DeckMenu::Render() 
{
  JRenderer * renderer = JRenderer::GetInstance();
  WFont * mFont = resources.GetWFont(fontId);
  float height = mHeight;

  // figure out where to place the stars initially
  if (0 == mWidth) {
    initMenuItems();
    stars->Fire();
    timeOpen = 0;
  }

  if (timeOpen < 1) height *= timeOpen > 0 ? timeOpen : -timeOpen;

  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
  stars->Render();
  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

  mFont->SetScale(1.0f);

  for (int i = startId; i < startId + maxItems ; i++){
    if (i > mCount-1) break;
    DeckMenuItem *currentMenuItem = static_cast<DeckMenuItem*>(mObjects[i]);
    if ( currentMenuItem->mY - kLineHeight * startId < mY + height - kLineHeight + 7) {
      if ( currentMenuItem->hasFocus()){
        // display the avatar image
        if ( currentMenuItem->imageFilename.size() > 0 )
        {
          JQuad * quad = resources.RetrieveTempQuad( currentMenuItem->imageFilename, TEXTURE_SUB_AVATAR ); 
          if (quad) 
            renderer->RenderQuad(quad, avatarX, avatarY);
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

          mainFont->DrawString( oss.str(), statsX, statsY );
        }
      } 
      else {
        mFont->SetColor(ARGB(150,255,255,255));
      }
      currentMenuItem->RenderWithOffset(-kLineHeight*startId);
    }

    renderer->RenderQuad(getBackground(), 0, 0 );

    if (!title.empty())
      titleFont->DrawString(title.c_str(), titleX, titleY, JGETEXT_CENTER);
    
    scroller->Render();
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

  scroller->Update(dt);
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


void DeckMenu::updateScroller()
{
  // add all the items from the Tasks db.
  TaskList *taskList = NEW TaskList();
  scroller->Reset();
  for (vector<Task*>::iterator it = taskList->tasks.begin(); it!=taskList->tasks.end(); it++)
  {
    ostringstream taskDescription;
    taskDescription << "[ " << setw(4) << (*it)->getReward() << " / " << (*it)->getExpiration() << " ]   " << (*it)->getDesc() << ". "  << endl;
    scroller->Add( taskDescription.str() );
  }
  SAFE_DELETE(taskList);
}


void DeckMenu::Close()
{
  timeOpen = -1.0;
  stars->Stop(true);
}


void DeckMenu::destroy(){
  SAFE_DELETE(DeckMenu::stars);
 }

DeckMenu::~DeckMenu()
{
  SAFE_DELETE(scroller);

}