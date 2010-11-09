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
  const float kVerticalMargin = 16;
  const float kHorizontalMargin = 20;
  const float kLineHeight = 20;
  const float kDescriptionVerticalBoxPadding = 5;
  const float kDescriptionHorizontalBoxPadding = 5;
}

hgeParticleSystem* DeckMenu::stars = NULL; 

//
//  For the additional info window, maximum characters per line is roughly 30 characters across.
//  TODO: figure a way to get incoming text to wrap.
//        
// used fixed locations where the menu, title and descriptive text are located.
//    * menu at (125, 60 )
//    * descriptive information 125
//    *** Need to make this configurable in a file somewhere to allow for class reuse

DeckMenu::DeckMenu(int id, JGuiListener* listener, int fontId, const string _title, const float& mFontScale): JGuiController(id, listener), fontId(fontId), menuFontScale( mFontScale ) {

 backgroundName = "DeckMenuBackdrop";

  mY = 55;
  mWidth = 176;
  mX = 125;

  titleX = 130; // center point in title box
  titleY = 28;
  titleWidth = 180; // width of inner box of title

  descX = 230 + kDescriptionVerticalBoxPadding;
  descY = 65 + kDescriptionHorizontalBoxPadding;
  descHeight = 145;
  descWidth = 220;

  starsOffsetX = 50;

  statsX = 280;
  statsY = 8;
  statsHeight = 50;
  statsWidth = 227;
  
  avatarX = 230;
  avatarY = 8;

  menuInitialized = false;

  float scrollerWidth = 80;
  scroller = NEW TextScroller(Fonts::MAIN_FONT, 40 , 230, scrollerWidth, 100, 1, 1);

  autoTranslate = true;
  maxItems = 7;
  mHeight = 2 * kVerticalMargin + ( maxItems * kLineHeight );

  // we want to cap the deck titles to 15 characters to avoid overflowing deck names
  title = _(_title);
  displayTitle = title;
  mFont = resources.GetWFont(fontId);
  
  startId = 0;
  selectionT = 0;
  timeOpen = 0;
  closed = false;

  if ( mFont->GetStringWidth( title.c_str() ) > titleWidth )
    titleFontScale = 0.75f;
  else
    titleFontScale = 1.0f;

  selectionTargetY = selectionY = kVerticalMargin;
      
  if (NULL == stars)
    stars = NEW hgeParticleSystem(resources.RetrievePSI("stars.psi", resources.GetQuad("stars")));
  stars->FireAt(mX, mY);

  updateScroller();
}


void DeckMenu::RenderBackground()
{
  ostringstream bgFilename;
  bgFilename << backgroundName << ".png";
  JQuad *background = resources.RetrieveTempQuad(bgFilename.str(), TEXTURE_SUB_5551);
  if ( background )
    JRenderer::GetInstance()->RenderQuad( background, 0, 0 );

}

void DeckMenu::initMenuItems()
{
  float sY = mY + kVerticalMargin;
  for (int i = startId; i < startId + mCount; ++i) {
    float y = mY + kVerticalMargin + i * kLineHeight;
    DeckMenuItem * currentMenuItem = static_cast<DeckMenuItem*>(mObjects[i]);
    currentMenuItem->Relocate( mX, y );
    if (currentMenuItem->hasFocus()) 
      sY = y;
  }
  selectionTargetY = selectionY = sY;
}

void DeckMenu::Render() 
{
  JRenderer * renderer = JRenderer::GetInstance();
  float height = mHeight;
  
  if (!menuInitialized)
  {
    initMenuItems();
    stars->Fire();
    timeOpen = 0;
    menuInitialized = true;
  }
  if (timeOpen < 1) height *= timeOpen > 0 ? timeOpen : -timeOpen;

  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
  stars->Render();
  renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

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
        mFont->SetColor(ARGB(255,255,255,255));

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
      mFont->SetScale( menuFontScale );
      currentMenuItem->RenderWithOffset(-kLineHeight*startId);
    }

    RenderBackground();

    if (!title.empty())
    {
      mFont->SetScale( titleFontScale );
      mFont->DrawString(title.c_str(), titleX, titleY, JGETEXT_CENTER);
    }
    mFont->SetScale( 1.0f );
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

  float starsX = starsOffsetX + ((mWidth - 2 * kHorizontalMargin)*(1+cos(selectionT))/2);
  float starsY = selectionY + 5 * cos(selectionT*2.35f) + kLineHeight / 2 - kLineHeight * startId;
  stars->MoveTo( starsX, starsY);
  if (timeOpen < 0) 
  {
    timeOpen += dt * 10;
    if (timeOpen >= 0) 
    { 
      timeOpen = 0; 
      closed = true; 
      stars->FireAt(mX, mY); 
    }
  } 
  else 
  {
    closed = false;
    timeOpen += dt * 10;
  }

  scroller->Update(dt);
}

void DeckMenu::Add(int id, const char * text,string desc, bool forceFocus, DeckMetaData * deckMetaData) {
  DeckMenuItem * menuItem = NEW DeckMenuItem(this, id, fontId, text, 0, mY + kVerticalMargin + mCount*kLineHeight, (mCount == 0), autoTranslate, deckMetaData);
  Translator * t = Translator::GetInstance();
  map<string,string>::iterator it = t->deckValues.find(text);
  if (it != t->deckValues.end()) //translate decks desc
    menuItem->desc = it->second;
  else
    menuItem->desc = deckMetaData ? deckMetaData->getDescription() : desc;

  JGuiController::Add(menuItem);
  if (mCount <= maxItems) 
    mHeight += kLineHeight;
  if (forceFocus){
    mObjects[mCurr]->Leaving(JGE_BTN_DOWN);
    mCurr = mCount-1;
    menuItem->Entering();
  }
}

void DeckMenu::updateScroller()
{
  // add all the items from the Tasks db.
  TaskList taskList;
  scroller->Reset();
  for (vector<Task*>::iterator it = taskList.tasks.begin(); it!=taskList.tasks.end(); it++)
  {
    ostringstream taskDescription;
    taskDescription << "[ " << setw(4) << (*it)->getReward() << " / " << (*it)->getExpiration() << " ]   " << (*it)->getDesc() << endl;
    scroller->Add( taskDescription.str() );
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

DeckMenu::~DeckMenu()
{
  SAFE_DELETE(scroller);
}
