#include "PrecompiledHeader.h"

#include "DeckMenu.h"
#include "DeckMenuItem.h"
#include "DeckMetaData.h"
#include "DeckManager.h"
#include "InteractiveButton.h"
#include "JTypes.h"
#include "GameApp.h"
#include "Translate.h"
#include "TextScroller.h"
#include "Tasks.h"

#include <iomanip>

namespace DeckMenuConst
{
    const float kVerticalMargin = 16;
    const float kHorizontalMargin = 20;
    const float kLineHeight = 25;
    const float kDescriptionVerticalBoxPadding = -5;
    const float kDescriptionHorizontalBoxPadding = 5;
    
    const float kVerticalScrollSpeed = 7.0f;

    const int DETAILED_INFO_THRESHOLD = 20;
    const int kDetailedInfoButtonId = 10000;
}

hgeParticleSystem* DeckMenu::stars = NULL;

//
//  For the additional info window, maximum characters per line is roughly 30 characters across.
//  TODO:        
//    *** Need to make this configurable in a file somewhere to allow for class reuse

DeckMenu::DeckMenu(int id, JGuiListener* listener, int fontId, const string _title, const int&, bool showDetailsOverride, bool chooseOpponent) :
JGuiController(JGE::GetInstance(), id, listener), fontId(fontId), mShowDetailsScreen( showDetailsOverride )
{

    backgroundName = "DeckMenuBackdrop";
    mAlwaysShowDetailsButton = false;
    mSelectedDeck = NULL;
    mY = 50;
    mWidth = 176;
    mX = 115;

    titleX = 110; // center point in title box
    titleY = 15;
    titleWidth = 180; // width of inner box of title

    descX = 260 + DeckMenuConst::kDescriptionHorizontalBoxPadding;
    descY = 100 + DeckMenuConst::kDescriptionVerticalBoxPadding;
    descHeight = 145;
    descWidth = 195;

    detailedInfoBoxX = 400;
    detailedInfoBoxY = 235;
    starsOffsetX = 50;

    statsX = 300;
    statsY = 15;
    statsHeight = 50;
    statsWidth = 227;

    avatarX = 232;
    avatarY = 11;

    menuInitialized = false;

    float scrollerWidth = 200.0f;
    float scrollerHeight = 28.0f;
    mScroller = NEW VerticalTextScroller(Fonts::MAIN_FONT, 14, 235, scrollerWidth, scrollerHeight, DeckMenuConst::kVerticalScrollSpeed);

    mAutoTranslate = true;
    maxItems = 6;
    mHeight = 2 * DeckMenuConst::kVerticalMargin + (maxItems * DeckMenuConst::kLineHeight);

    // we want to cap the deck titles to 15 characters to avoid overflowing deck names
    title = _(_title);
    displayTitle = title;
    mFont = WResourceManager::Instance()->GetWFont(fontId);

    startId = 0;
    selectionT = 0;
    timeOpen = 0;
    mClosed = false;
    isOpponent = chooseOpponent;

    if (mFont->GetStringWidth(title.c_str()) > titleWidth)
        titleFontScale = SCALE_SHRINK;
    else
        titleFontScale = SCALE_NORMAL;

    mSelectionTargetY = selectionY = DeckMenuConst::kVerticalMargin;

    if (NULL == stars)
        stars = NEW hgeParticleSystem(WResourceManager::Instance()->RetrievePSI("stars.psi", WResourceManager::Instance()->GetQuad("stars").get()));
    stars->FireAt(mX, mY);
    
    const string detailedInfoString = _("info");
    WFont *descriptionFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);

    float stringWidth = descriptionFont->GetStringWidth(detailedInfoString.c_str());
    float boxStartX = detailedInfoBoxX - stringWidth / 2 + 20;
    //dismiss button?
#if defined PSP   
    dismissButton = NEW InteractiveButton( this, DeckMenuConst::kDetailedInfoButtonId, Fonts::MAIN_FONT, detailedInfoString, boxStartX+25, detailedInfoBoxY-10.0f, JGE_BTN_CANCEL);
#else
    dismissButton = NEW InteractiveButton( this, DeckMenuConst::kDetailedInfoButtonId, Fonts::MAIN_FONT, detailedInfoString, boxStartX+30, detailedInfoBoxY+4.5f, JGE_BTN_CANCEL);
#endif
    JGuiController::Add(dismissButton, true);

    updateScroller();
}

void DeckMenu::RenderDeckManaColors()
{
    // display the deck mana colors if known
    // We only want to display the mana symbols on the game screens and not the Deck Editor screens.  
    // Need a better way to determine where/when to display the mana symbols.  Perhaps make it a property setting.
    bool displayDeckMana = backgroundName.find("DeckMenuBackdrop") != string::npos;
    // current set of coordinates puts the mana symbols to the right of the last stat info in the upper right
    // box of the deck selection screen.
    //float manaIconX = 398;
    //float manaIconY = 55;
    float manaIconX = 288;
    float manaIconY = 248;
    if (mSelectedDeck &&displayDeckMana)
    {
        string deckManaColors = mSelectedDeck->getColorIndex().c_str();
        if (deckManaColors.size() >= 6)
        {   
            // due to space constraints don't display icons for colorless mana.
            for( int colorIdx = Constants::MTG_COLOR_ARTIFACT; colorIdx < Constants::MTG_COLOR_WASTE; ++colorIdx )
            {               
                if ( (deckManaColors.at(colorIdx) == '1') != 0)
                {
                    //JRenderer::GetInstance()->FillCircle(manaIconX,manaIconY,8.5f,ARGB(240,255,255,5));
                    JRenderer::GetInstance()->RenderQuad(manaIcons[colorIdx].get(), manaIconX, manaIconY, 0, 0.6f, 0.6f);
                    manaIconX += 26;
                }
            }
        }
        else if (deckManaColors.compare("") != 0 )
            DebugTrace("Error with color index string for "<< mSelectedDeck->getName() << ". [" << deckManaColors << "].");
    }
}

void DeckMenu::RenderBackground()
{
    ostringstream bgFilename;
#if !defined (PSP)
    if(backgroundName.find("menubgdeckeditor") != string::npos) //Now it's possibile to randomly use up to 10 background images for deck editor selection.
        bgFilename << backgroundName << ".jpg";
    else
        bgFilename << backgroundName << ".png";
#else
    if(backgroundName == "menubgdeckeditor")
        bgFilename << "pspmenubgdeckeditor.jpg";
    else
        bgFilename << "pspmenubgdeckeditor.png";
#endif

    static bool loadBackground = true;
    if (loadBackground)
    {
        //Erwan 2010/07/11 the TEXTURE_SUB_5551 below is useless, JGE doesn't support it for png. I'm letting it here to avoid causing a bug, but it should be removed
        JQuadPtr background = WResourceManager::Instance()->RetrieveTempQuad(bgFilename.str(), TEXTURE_SUB_5551);
        if (background.get())
        {
            float scaleX = SCREEN_WIDTH_F / background.get()->mWidth;
            float scaleY = SCREEN_HEIGHT_F / background.get()->mHeight;
            JRenderer::GetInstance()->RenderQuad(background.get(), 0, 0,0,scaleX, scaleY);
        }
        else
            loadBackground = false;
    }
}

DeckMetaData * DeckMenu::getSelectedDeck()
{
    if (mSelectedDeck) return mSelectedDeck;

    return NULL;
}

void DeckMenu::enableDisplayDetailsOverride()
{
    mAlwaysShowDetailsButton = true;
}

bool DeckMenu::showDetailsScreen()
{
    DeckMetaData * currentMenuItem = getSelectedDeck();
    if (currentMenuItem)
    {
        if (mAlwaysShowDetailsButton) return true;
        if (mShowDetailsScreen && currentMenuItem->getVictories() > DeckMenuConst::DETAILED_INFO_THRESHOLD) return true;
    }

    return false;
}

void DeckMenu::initMenuItems()
{
    float sY = mY + DeckMenuConst::kVerticalMargin;
    for (int i = startId; i < mCount; ++i)
    {
        float y = mY + DeckMenuConst::kVerticalMargin + i * DeckMenuConst::kLineHeight;
        DeckMenuItem * currentMenuItem = static_cast<DeckMenuItem*> (mObjects[i]);
        currentMenuItem->Relocate(mX, y);
        if (currentMenuItem->hasFocus()) sY = y;
    }
    mSelectionTargetY = selectionY = sY;

#ifndef TOUCH_ENABLED
    //Grab a texture in VRAM.
    pspIconsTexture = WResourceManager::Instance()->RetrieveTexture("iconspsp.png", RETRIEVE_MANAGE);

    char buf[512];
    for (int i = 0; i < 8; i++)
    {
        sprintf(buf, "iconspsp%d", i);
        pspIcons[i] = WResourceManager::Instance()->RetrieveQuad("iconspsp.png", (float) i * 32, 0, 32, 32, buf);
        pspIcons[i]->SetHotSpot(16, 16);
    }
    dismissButton->setImage(pspIcons[5]);
#endif
}

void DeckMenu::selectRandomDeck(bool isAi)
{
    DeckManager *deckManager = DeckManager::GetInstance();
    vector<DeckMetaData *> *deckList = isAi ? deckManager->getAIDeckOrderList() : deckManager->getPlayerDeckOrderList();
    int random = rand() % (int)deckList->size();
    selectDeck( random, isAi );
}

void DeckMenu::selectDeck(int deckId, bool isAi)
{
    DeckManager *deckManager = DeckManager::GetInstance();
    vector<DeckMetaData *> *deckList = isAi ? deckManager->getAIDeckOrderList() : deckManager->getPlayerDeckOrderList();
    mSelectedDeck = deckList->at(deckId);
}

void DeckMenu::Render()
{
    JRenderer * renderer = JRenderer::GetInstance();
    float height = mHeight;
    JQuadPtr avatarholder;
    JQuadPtr menupanel;
    JQuadPtr menuholder;
#if defined (PSP)
    avatarholder = WResourceManager::Instance()->RetrieveTempQuad("pspavatarholder.png");//new graphics avatarholder for PSP
    menupanel = WResourceManager::Instance()->RetrieveTempQuad("pspmenupanel.jpg");//new graphics menupanel for PSP
    menuholder = WResourceManager::Instance()->RetrieveTempQuad("pspmenuholder.png");//new graphics menuholder for PSP
#else
    avatarholder = WResourceManager::Instance()->RetrieveTempQuad("avatarholder.png");//new graphics avatarholder
    if(isOpponent){
        menupanel = WResourceManager::Instance()->RetrieveTempQuad("menupanel2.jpg");//try to load the new graphics menupanel for opponent.
        if(!menupanel.get())
            menupanel = WResourceManager::Instance()->RetrieveTempQuad("menupanel.jpg");//fallback to new graphics menupanel for player
    } else
        menupanel = WResourceManager::Instance()->RetrieveTempQuad("menupanel.jpg");//new graphics menupanel for player
    menuholder = WResourceManager::Instance()->RetrieveTempQuad("menuholder.png");//new graphics menuholder
#endif
    bool inDeckMenu = backgroundName.find("DeckMenuBackdrop") != string::npos;
    float modAvatarX = 0.f;
    float modAvatarY = 0.f;

    if(inDeckMenu)
    {
        modAvatarX =26.f;
        modAvatarY =1.f;
    }
    else
    {
        modAvatarX =-76.f;
        modAvatarY =-1.5f;
    }
    if (!menuInitialized)
    {
        initMenuItems();
        stars->Fire();
        timeOpen = 0;
        menuInitialized = true;
    }

    if (avatarholder.get() && menupanel.get() && inDeckMenu)//bg panel
         renderer->RenderQuad(menupanel.get(), 225.f, 0, 0 ,SCREEN_WIDTH_F / avatarholder.get()->mWidth, SCREEN_HEIGHT_F / avatarholder.get()->mHeight);

    RenderBackground();//background deck menu
    mScroller->Render();

    if (menuholder.get() && inDeckMenu)//menuholder
         renderer->RenderQuad(menuholder.get(), 0, 0, 0 ,SCREEN_WIDTH_F / menuholder.get()->mWidth, SCREEN_HEIGHT_F / menuholder.get()->mHeight);

    if (timeOpen < 1) height *= timeOpen > 0 ? timeOpen : -timeOpen;
    
    for (int i = startId; i < startId + maxItems; i++)
    {
        if (i > mCount - 1) break;        
        DeckMenuItem *currentMenuItem = static_cast<DeckMenuItem*> (mObjects[i]);
        if (currentMenuItem->getY() - DeckMenuConst::kLineHeight * startId < mY + height - DeckMenuConst::kLineHeight + 7)
        {
            // only load stats for visible items in the list
            DeckMetaData* metaData = currentMenuItem->getMetaData();
            if (metaData && !metaData->mStatsLoaded)
            {
                metaData->LoadStats();
            }

            if (currentMenuItem->hasFocus())
            {
                mSelectedDeck = metaData;
                WFont *descriptionFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);

                // display the "more info" button if special condition is met
                if (showDetailsScreen())
                {                    
                    dismissButton->setIsSelectionValid(true);
                    dismissButton->Render();
                }
                else
                {
                    dismissButton->setIsSelectionValid(false);
                }//detailed info???
                // display the avatar image
                string currentAvatarImageName = currentMenuItem->getImageFilename();
                if (currentAvatarImageName.size() > 0)
                {
                    JQuadPtr quad = WResourceManager::Instance()->RetrieveTempQuad(currentAvatarImageName, TEXTURE_SUB_AVATAR);
                    if(quad.get())
                    {
                        float xscale = 37.f / quad->mWidth;//orig 35.f
                        float yscale = 50.f / quad->mHeight;
                        if (currentMenuItem->getText() == "Evil Twin")
                        {
                            JQuad * evil = quad.get();
                            evil->SetHFlip(true);

                            if (avatarholder.get() && inDeckMenu)
                                renderer->RenderQuad(avatarholder.get(), 0, 0, 0 ,SCREEN_WIDTH_F / avatarholder.get()->mWidth, SCREEN_HEIGHT_F / avatarholder.get()->mHeight);

                            renderer->RenderQuad(quad.get(), avatarX+modAvatarX, avatarY+modAvatarY, 0, xscale, yscale);
                            renderer->DrawRect(avatarX+modAvatarX, avatarY+modAvatarY,37.f,50.f,ARGB(200,3,3,3));
                            evil = NULL;
                        }
                        else
                        {

                            if (avatarholder.get() && inDeckMenu)
                                renderer->RenderQuad(avatarholder.get(), 0, 0, 0 ,SCREEN_WIDTH_F / avatarholder.get()->mWidth, SCREEN_HEIGHT_F / avatarholder.get()->mHeight);

                            renderer->RenderQuad(quad.get(), avatarX+modAvatarX, avatarY+modAvatarY, 0, xscale, yscale);
                            renderer->DrawRect(avatarX+modAvatarX, avatarY+modAvatarY,37.f,50.f,ARGB(200,3,3,3));
                        }
                    }
                }
                
                // fill in the description part of the screen
                string text = wordWrap(_(currentMenuItem->getDescription()), descWidth, descriptionFont->mFontID );
                descriptionFont->SetColor(ARGB(255,255,255,255));
                descriptionFont->DrawString(text.c_str(), descX, descY);
                
                // fill in the statistical portion
                if (currentMenuItem->hasMetaData())
                {
                    ostringstream oss;
                    oss << _("Deck: ") << currentMenuItem->getDeckName() << endl;
                    oss << currentMenuItem->getDeckStatsSummary();
                    descriptionFont->SetColor(ARGB(255,255,255,255));
                    if(inDeckMenu)
                        descriptionFont->DrawString(oss.str(), statsX+2, statsY-2);
                    else
                        descriptionFont->DrawString(oss.str(), statsX-86, statsY-4);
                }
                
                // change the font color of the current menu item
                mFont->SetColor(ARGB(255,255,255,255));
            }
            else // reset the font color to be slightly muted
                mFont->SetColor(ARGB(150,255,255,255));
            currentMenuItem->RenderWithOffset(-DeckMenuConst::kLineHeight * startId);
        }
    }
    //psp
    RenderDeckManaColors();

    if (!title.empty())
    {
        mFont->SetColor(ARGB(255,255,255,255));
        mFont->DrawString(title.c_str(), titleX, titleY, JGETEXT_CENTER);
    }
    
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    stars->Render();
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

}

void DeckMenu::Update(float dt)
{
    JGuiController::Update(dt);
    
    if (mCurr > startId + maxItems - 1)
        startId = mCurr - maxItems + 1;
    else if (mCurr < startId) 
        startId = mCurr;

    stars->Update(dt);
    selectionT += 3 * dt;
    selectionY += (mSelectionTargetY - selectionY) * 8 * dt;

    float starsX = starsOffsetX + ((mWidth - 2 * DeckMenuConst::kHorizontalMargin) * (1 + cos(selectionT)) / 2);
    float starsY = selectionY + 5 * cos(selectionT * 2.35f) + DeckMenuConst::kLineHeight / 2 - DeckMenuConst::kLineHeight * startId;
    stars->MoveTo(starsX, starsY);
    
    // 
    
    if (timeOpen < 0)
    {
        timeOpen += dt * 10;
        if (timeOpen >= 0)
        {
            timeOpen = 0;
            mClosed = true;
            stars->FireAt(mX, mY);
        }
    }
    else
    {
        mClosed = false;
        timeOpen += dt * 10;
    }
    if (mScroller) 
        mScroller->Update(dt);
    
}

void DeckMenu::Add(int id, const string& text, const string& desc, bool forceFocus, DeckMetaData * deckMetaData)
{
    DeckMenuItem * menuItem = NEW DeckMenuItem(this, id, fontId, text, 0, 
            mY + DeckMenuConst::kVerticalMargin + mCount * DeckMenuConst::kLineHeight, (mCount == 0), mAutoTranslate, deckMetaData);
    Translator * t = Translator::GetInstance();
    map<string, string>::iterator it = t->deckValues.find(text);
    string deckDescription = "";
    
    if (it != t->deckValues.end()) //translate decks desc
        deckDescription = it->second;
    else
        deckDescription = deckMetaData ? deckMetaData->getDescription() : desc;

    if(deckMetaData && deckMetaData->isCommanderDeck())
        deckDescription = deckDescription + " (" + _("CMD") + ")"; // It will show a CMD suffix for Commander Decks.

    menuItem->setDescription(deckDescription);

    JGuiController::Add(menuItem);
    if (mCount <= maxItems) mHeight += DeckMenuConst::kLineHeight;

    if (forceFocus)
    {
        mObjects[mCurr]->Leaving(JGE_BTN_DOWN);
        mCurr = mCount - 1;
        menuItem->Entering();
    }
}

void DeckMenu::updateScroller()
{
    // add all the items from the Tasks db.
    TaskList taskList;
    mScroller->Reset();

    for (vector<Task*>::iterator it = taskList.tasks.begin(); it != taskList.tasks.end(); it++)
    {
        ostringstream taskDescription;
        taskDescription << "Credits: " << setw(4) << (*it)->getReward() << " / "
            << "Days Left: " << (*it)->getExpiration() << endl 
            << (*it)->getDesc() << endl << endl;
        mScroller->Add(taskDescription.str());
    }
    
}

void DeckMenu::Close()
{
    timeOpen = -1.0;
    stars->Stop(true);
}

void DeckMenu::destroy()
{
    SAFE_DELETE(DeckMenu::stars);
}

DeckMenu::~DeckMenu()
{
    WResourceManager::Instance()->Release(pspIconsTexture);
    SAFE_DELETE(mScroller);
    dismissButton = NULL;
}
