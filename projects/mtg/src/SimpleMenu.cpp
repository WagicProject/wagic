#include "PrecompiledHeader.h"

#include "SimpleMenu.h"
#include "SimpleMenuItem.h"
#include "JTypes.h"
#include "GameApp.h"
#include "Translate.h"

namespace SimpleMenuConst
{
    const float kPoleWidth = 7;
    const float kVerticalMargin = 20;
    const float kHorizontalMargin = 30;

    
#ifdef TOUCH_ENABLED
    const float kLineHeight = 30;
#else
    const float kLineHeight = 20;
#endif
    //For the "Classic" Theme
    const float kSpadeHeightOffset = 4;
    const float kSpadeWidthOffset = 9;
    const float kSpadeRightBottomOffset = 3;
}

JQuadPtr SimpleMenu::spadeR;
JQuadPtr SimpleMenu::spadeL;
JQuadPtr SimpleMenu::jewel;
JQuadPtr SimpleMenu::side;
JTexture* SimpleMenu::spadeRTex = NULL;
JTexture* SimpleMenu::spadeLTex = NULL;
JTexture* SimpleMenu::jewelTex = NULL;
JTexture* SimpleMenu::sideTex = NULL;

SimpleMenu::SimpleMenu(JGE* jge, WResourceManager* resourceManager, int id, JGuiListener* listener, int fontId, float x, float y, const char * _title, int _maxItems, bool centerHorizontal, bool centerVertical)
    : JGuiController(jge, id, listener), fontId(fontId), mCenterHorizontal(centerHorizontal), mCenterVertical(centerVertical), stars(0)
{
    autoTranslate = true;
    isMultipleChoice = false;
    mHeight = 2 * SimpleMenuConst::kVerticalMargin;
    mWidth = 0;
    mX = x;
    mY = y;
    title = _(_title);
    startId = 0;
    maxItems = _maxItems;
    selectionT = 0;
    timeOpen = 0;
    mClosed = false;
    selectionTargetY = selectionY = y + SimpleMenuConst::kVerticalMargin;

    if(resourceManager)
    {
        JRenderer* renderer = JRenderer::GetInstance();

        if (!spadeLTex) spadeLTex = resourceManager->RetrieveTexture("spade_ul.png", RETRIEVE_MANAGE);
        if (!spadeRTex) spadeRTex = resourceManager->RetrieveTexture("spade_ur.png", RETRIEVE_MANAGE);
        if (!jewelTex) jewelTex = renderer->CreateTexture(5, 5, TEX_TYPE_USE_VRAM);
        if (!sideTex) sideTex = resourceManager->RetrieveTexture("menuside.png", RETRIEVE_MANAGE);
        spadeL = resourceManager->RetrieveQuad("spade_ul.png", 0, 0, 0, 0, "spade_ul", RETRIEVE_MANAGE);
        spadeR = resourceManager->RetrieveQuad("spade_ur.png", 0, 0, 0, 0, "spade_ur", RETRIEVE_MANAGE);
        jewel.reset(NEW JQuad(jewelTex, 1, 1, 3, 3));
        side = resourceManager->RetrieveQuad("menuside.png", 1, 1, 1, SimpleMenuConst::kPoleWidth, "menuside", RETRIEVE_MANAGE);

        stars = NEW hgeParticleSystem(resourceManager->RetrievePSI("stars.psi", resourceManager->GetQuad("stars").get()));

        stars->FireAt(mX, mY);
    }
}

SimpleMenu::~SimpleMenu()
{
    SAFE_DELETE(stars);
}

void SimpleMenu::drawHorzPole(float x, float y, float width)
{
    JRenderer* renderer = JRenderer::GetInstance();

    float leftXOffset = (spadeR->mWidth - SimpleMenuConst::kPoleWidth) / 2;
    float rightXOffset = leftXOffset;
    float yOffset = leftXOffset;
    if (spadeR->mWidth != spadeR->mHeight) 
    {
        //We have a weird case to deal with in the "Classic" theme, the spades graphics need to be aligned specifically,
        // While the ones in the "Final Saga" theme need to be centered
        leftXOffset =  SimpleMenuConst::kSpadeWidthOffset;
        yOffset = SimpleMenuConst::kSpadeHeightOffset;
        rightXOffset = SimpleMenuConst::kSpadeRightBottomOffset;
    }

    renderer->RenderQuad(side.get(), x, y, 0, width);
    spadeR->SetHFlip(true);
    spadeL->SetHFlip(false);
    renderer->RenderQuad(spadeR.get(), x - leftXOffset, y - yOffset );
    renderer->RenderQuad(spadeL.get(), x + width - rightXOffset, y - yOffset);

    renderer->RenderQuad(jewel.get(), x, y - 1);
    renderer->RenderQuad(jewel.get(), x + width - 1, y - 1);
}

void SimpleMenu::drawVertPole(float x, float y, float height)
{
    JRenderer* renderer = JRenderer::GetInstance();

    float xOffset = (spadeR->mWidth - SimpleMenuConst::kPoleWidth) / 2;
    float topYOffset = xOffset;
    float bottomYOffset = xOffset;
    if (spadeR->mWidth != spadeR->mHeight) 
    {
        //We have a weird case to deal with in the "Classic" theme, the spades graphics need to be aligned specifically,
        // While the ones in the "Final Saga" theme need to be centered
        xOffset = SimpleMenuConst::kSpadeHeightOffset;
        topYOffset = SimpleMenuConst::kSpadeWidthOffset;
        bottomYOffset = SimpleMenuConst::kSpadeRightBottomOffset;
    }

    renderer->RenderQuad(side.get(), x + SimpleMenuConst::kPoleWidth, y, M_PI / 2, height);
    spadeR->SetHFlip(true);
    spadeL->SetHFlip(false);
    renderer->RenderQuad(spadeR.get(), x + SimpleMenuConst::kPoleWidth + xOffset, y - topYOffset, M_PI / 2);
    renderer->RenderQuad(spadeL.get(), x + SimpleMenuConst::kPoleWidth + xOffset, y + height - bottomYOffset, M_PI / 2);

    renderer->RenderQuad(jewel.get(), x - 1, y - 1);
    renderer->RenderQuad(jewel.get(), x - 1, y + height - 1);
}

void SimpleMenu::Render()
{
    WFont * titleFont = WResourceManager::Instance()->GetWFont(fontId);
    titleFont->SetColor(ARGB(250,255,255,255));//reseting color on passes as this is a shared font now.
    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    if (0 == mWidth)
    {
        float sY = mY + SimpleMenuConst::kVerticalMargin;

        for (int i = 0; i < mCount; ++i)
        {
            float width = (static_cast<SimpleMenuItem*> (mObjects[i]))->GetEnlargedWidth() + 15;
            if (mWidth < width) mWidth = width;
        }

        float scaleFactor = titleFont->GetScale();
        titleFont->SetScale(SCALE_NORMAL);
        if ((!title.empty()) && (mWidth < titleFont->GetStringWidth(title.c_str()))) 
			mWidth = titleFont->GetStringWidth(title.c_str());
         titleFont->SetScale(scaleFactor);
        mWidth += 2 * SimpleMenuConst::kHorizontalMargin;

        if (mCenterHorizontal)
            mX = (SCREEN_WIDTH_F - mWidth) / 2;

        if (mCenterVertical)
            mY = (SCREEN_HEIGHT_F - mHeight) / 2;

        for (int i = 0; i < mCount; ++i)
        {
            float y = mY + SimpleMenuConst::kVerticalMargin + i * SimpleMenuConst::kLineHeight;
            SimpleMenuItem * smi = static_cast<SimpleMenuItem*> (mObjects[i]);
            smi->Relocate(mX + mWidth / 2, y);
            if (smi->hasFocus()) sY = y;
        }
        stars->Fire();
        selectionTargetY = selectionY = sY;
        timeOpen = 0;
    }

    JRenderer * renderer = JRenderer::GetInstance();

    float height = mHeight;
    if (timeOpen < 1) height *= timeOpen > 0 ? timeOpen : -timeOpen;

    float heightPadding = SimpleMenuConst::kLineHeight/2; // this to reduce the bottom padding of the menu
    renderer->FillRect(mX, mY, mWidth, height - heightPadding, ARGB(180,0,0,0));

    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    
    drawVertPole(mX, mY, height - heightPadding);
    drawVertPole(mX + mWidth, mY, height - heightPadding);
    drawHorzPole(mX, mY, mWidth);
    drawHorzPole(mX, mY + height - heightPadding, mWidth);
    //drawVertPole(mX, mY - 16, height + 32);
    //drawVertPole(mX + mWidth, mY - 16, height + 32);
    //drawHorzPole(mX - 16, mY, mWidth + 32);
    //drawHorzPole(mX - 25, mY + height, mWidth + 50);

    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    stars->Render();
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

    if (!title.empty()) 
    {
        float scaleFactor = titleFont->GetScale();
        titleFont->SetScale(SCALE_NORMAL);
        titleFont->DrawString(title.c_str(), mX + mWidth / 2, mY - 3, JGETEXT_CENTER);
        titleFont->SetScale(scaleFactor);
    }
    for (int i = startId; i < startId + maxItems; i++)
    {
        if (i > mCount - 1) break;
        SimpleMenuItem *currentMenuItem = static_cast<SimpleMenuItem*>(mObjects[i]);
        float currentY = currentMenuItem->getY() - SimpleMenuConst::kLineHeight * startId;
        float menuBottomEdge = mY + height - SimpleMenuConst::kLineHeight + 7;
        if (currentY < menuBottomEdge)
        {
            if (currentMenuItem->hasFocus())
            {
                WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT)->DrawString(currentMenuItem->getDescription().c_str(), mX
                                + mWidth + 10, mY + 15);
                mFont->SetColor(ARGB(255,255,255,0));
            }
            else
            {
                mFont->SetColor(ARGB(150,255,255,255));
            }
            (static_cast<SimpleMenuItem*> (mObjects[i]))->RenderWithOffset(-SimpleMenuConst::kLineHeight * startId);
        }
        mFont->SetScale(SCALE_NORMAL);
    }
    mFont->SetScale(SCALE_NORMAL);
}

bool SimpleMenu::CheckUserInput(JButton key)
{
    // a dude may have clicked somewhere, we're gonna select the closest object from where he clicked
    // since we know we are in a menu, we just need to check one cardinality
    int x = -1, y = -1;
    int n = mCurr;
    
    if ((key == JGE_BTN_NONE) && mEngine->GetLeftClickCoordinates(x, y))
    {

        // first scan the buttons on the screen and then process the other gui elements
        for (size_t i = 0; i < mButtons.size(); i++)
        {
            if (mButtons[i]->ButtonPressed())
            {
                mEngine->LeftClickedProcessed();
                return true;
            }
        }
        
        if (mObjects.size())
        {
            float top, left;
            float menuTopEdge =  mY + SimpleMenuConst::kLineHeight;
            float menuBottomEdge = mY + mHeight - (SimpleMenuConst::kLineHeight/2);
            
            if (y < menuTopEdge)
                n = (mCurr - 1) > 0 ? mCurr -1 : 0;
            else if (y >= menuBottomEdge)
                n = (mCurr + 1) < mCount ? mCurr + 1 : mCurr - 1;
            else
            {
                for (int i = 0; i < mCount; i++)
                {
                    if (mObjects[i]->getTopLeft(top, left))
                    {
                        if ( (y > top) && (y <= (top + SimpleMenuConst::kLineHeight)) )
                            n = i;
                    }   
                }
            }
            
            // check bounds of n. 
            if ( n < 0 ) 
                n = 0;
            if ( n >= mCount ) 
                n = mCount - 1;
            
            // check to see if the user clicked 
            if( n-mCurr > this->maxItems+1 )
                n = mCurr+1;//we don't want to increment pages at a time.
            if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(JGE_BTN_DOWN))
            {
                mCurr = n;
                mObjects[mCurr]->Entering();
            }
            // if the same object was selected process click
            else if (n == mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(JGE_BTN_OK))
            {
                mObjects[mCurr]->Entering();
            }

            mEngine->LeftClickedProcessed();
            mEngine->ResetInput();
            return true;
        }
        mEngine->LeftClickedProcessed();
    }
    else
        return JGuiController::CheckUserInput(key);
    
    return false;
}


void SimpleMenu::Update(float dt)
{
    JGuiController::Update(dt);
    if (mCurr > startId + maxItems - 1)
        startId = mCurr - maxItems + 1;
    else if (mCurr < startId) startId = mCurr;
 
    if(stars)
        stars->Update(dt);
    selectionT += 3 * dt;
    selectionY += (selectionTargetY - selectionY) * 8 * dt;
    if(stars)
        stars->MoveTo(mX + SimpleMenuConst::kHorizontalMargin + ((mWidth - 2 * SimpleMenuConst::kHorizontalMargin) * (1 + cos(selectionT)) / 2), selectionY + 5 * cos(
                    selectionT * 2.35f) + SimpleMenuConst::kLineHeight / 2 - SimpleMenuConst::kLineHeight * startId);
    if (timeOpen < 0)
    {
        timeOpen += dt * 10;
        if (timeOpen >= 0)
        {
            timeOpen = 0;
            mClosed = true;
            if(stars)
                stars->FireAt(mX, mY);
        }
    }
    else
    {
        mClosed = false;
        timeOpen += dt * 10;
    }
}

void SimpleMenu::Add(int id, const string& text, string desc, bool forceFocus)
{
    SimpleMenuItem * smi = NEW SimpleMenuItem(this, id, fontId, text, 0, mY + SimpleMenuConst::kVerticalMargin + mCount * SimpleMenuConst::kLineHeight,
                    (mCount == 0), autoTranslate);

    smi->setDescription(desc);
    JGuiController::Add(smi);
    if (mCount <= maxItems) mHeight += SimpleMenuConst::kLineHeight;
    if (forceFocus)
    {
        mObjects[mCurr]->Leaving(JGE_BTN_DOWN);
        mCurr = mCount - 1;
        smi->Entering();
    }
}

void SimpleMenu::Close()
{
    timeOpen = -1.0;
    stars->Stop(true);
}

void SimpleMenu::destroy()
{
    SimpleMenu::jewel.reset();
    SAFE_DELETE(SimpleMenu::jewelTex);
}
