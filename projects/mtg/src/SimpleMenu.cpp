#include "PrecompiledHeader.h"

#include "SimpleMenu.h"
#include "SimpleMenuItem.h"
#include "JTypes.h"
#include "GameApp.h"
#include "Translate.h"

namespace
{
    const float kPoleWidth = 7;
    const float kVerticalMargin = 16;
    const float kHorizontalMargin = 30;
    const float kLineHeight = 20;

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
WFont* SimpleMenu::titleFont = NULL;
hgeParticleSystem* SimpleMenu::stars = NULL;

SimpleMenu::SimpleMenu(int id, JGuiListener* listener, int fontId, float x, float y, const char * _title, int _maxItems, bool centerHorizontal, bool centerVertical)
    : JGuiController(id, listener), fontId(fontId), mCenterHorizontal(centerHorizontal), mCenterVertical(centerVertical)
{
    autoTranslate = true;
    mHeight = 2 * kVerticalMargin;
    mWidth = 0;
    mX = x;
    mY = y;
    title = _(_title);
    startId = 0;
    maxItems = _maxItems;
    selectionT = 0;
    timeOpen = 0;
    mClosed = false;
    selectionTargetY = selectionY = y + kVerticalMargin;

    JRenderer* renderer = JRenderer::GetInstance();

    if (!spadeLTex) spadeLTex = WResourceManager::Instance()->RetrieveTexture("spade_ul.png", RETRIEVE_MANAGE);
    if (!spadeRTex) spadeRTex = WResourceManager::Instance()->RetrieveTexture("spade_ur.png", RETRIEVE_MANAGE);
    if (!jewelTex) jewelTex = renderer->CreateTexture(5, 5, TEX_TYPE_USE_VRAM);
    if (!sideTex) sideTex = WResourceManager::Instance()->RetrieveTexture("menuside.png", RETRIEVE_MANAGE);
    spadeL = WResourceManager::Instance()->RetrieveQuad("spade_ul.png", 0, 0, 0, 0, "spade_ul", RETRIEVE_MANAGE);
    spadeR = WResourceManager::Instance()->RetrieveQuad("spade_ur.png", 0, 0, 0, 0, "spade_ur", RETRIEVE_MANAGE);
    jewel.reset(NEW JQuad(jewelTex, 1, 1, 3, 3));
    side = WResourceManager::Instance()->RetrieveQuad("menuside.png", 1, 1, 1, kPoleWidth, "menuside", RETRIEVE_MANAGE);

    if (NULL == stars)
        stars = NEW hgeParticleSystem(WResourceManager::Instance()->RetrievePSI("stars.psi", WResourceManager::Instance()->GetQuad("stars").get()));

    stars->FireAt(mX, mY);
}

SimpleMenu::~SimpleMenu()
{
}

void SimpleMenu::drawHorzPole(float x, float y, float width)
{
    JRenderer* renderer = JRenderer::GetInstance();

    float leftXOffset = (spadeR->mWidth - kPoleWidth) / 2;
    float rightXOffset = leftXOffset;
    float yOffset = leftXOffset;
    if (spadeR->mWidth != spadeR->mHeight) 
    {
        //We have a weird case to deal with in the "Classic" theme, the spades graphics need to be aligned specifically,
        // While the ones in the "Final Saga" theme need to be centered
        leftXOffset =  kSpadeWidthOffset;
        yOffset = kSpadeHeightOffset;
        rightXOffset = kSpadeRightBottomOffset;
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


    float xOffset = (spadeR->mWidth - kPoleWidth) / 2;
    float topYOffset = xOffset;
    float bottomYOffset = xOffset;
    if (spadeR->mWidth != spadeR->mHeight) 
    {
        //We have a weird case to deal with in the "Classic" theme, the spades graphics need to be aligned specifically,
        // While the ones in the "Final Saga" theme need to be centered
        xOffset = kSpadeHeightOffset;
        topYOffset = kSpadeWidthOffset;
        bottomYOffset = kSpadeRightBottomOffset;
    }

    renderer->RenderQuad(side.get(), x + kPoleWidth, y, M_PI / 2, height);
    spadeR->SetHFlip(true);
    spadeL->SetHFlip(false);
    renderer->RenderQuad(spadeR.get(), x + kPoleWidth + xOffset, y - topYOffset, M_PI / 2);
    renderer->RenderQuad(spadeL.get(), x + kPoleWidth + xOffset, y + height - bottomYOffset, M_PI / 2);

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
        float sY = mY + kVerticalMargin;

        for (int i = 0; i < mCount; ++i)
        {
            float width = (static_cast<SimpleMenuItem*> (mObjects[i]))->GetWidth();
            if (mWidth < width) mWidth = width;
        }
        if ((!title.empty()) && (mWidth < titleFont->GetStringWidth(title.c_str()))) 
			mWidth = titleFont->GetStringWidth(title.c_str());
        mWidth += 2 * kHorizontalMargin;

        if (mCenterHorizontal)
            mX = (JRenderer::GetInstance()->GetActualWidth() - mWidth) / 2;

        if (mCenterVertical)
            mY = (JRenderer::GetInstance()->GetActualHeight() - mHeight) / 2;

        for (int i = 0; i < mCount; ++i)
        {
            float y = mY + kVerticalMargin + i * kLineHeight;
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

    renderer->FillRect(mX, mY, mWidth, height, ARGB(180,0,0,0));

    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    drawVertPole(mX, mY - 16, height + 32);
    drawVertPole(mX + mWidth, mY - 16, height + 32);
    drawHorzPole(mX - 16, mY, mWidth + 32);
    drawHorzPole(mX - 25, mY + height, mWidth + 50);

    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    stars->Render();
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

    mFont->SetScale(1.0f);
    if (!title.empty()) titleFont->DrawString(title.c_str(), mX + mWidth / 2, mY - 3, JGETEXT_CENTER);
    for (int i = startId; i < startId + maxItems; i++)
    {
        if (i > mCount - 1) break;
        if ((static_cast<SimpleMenuItem*> (mObjects[i]))->mY - kLineHeight * startId < mY + height - kLineHeight + 7)
        {
            if (static_cast<SimpleMenuItem*> (mObjects[i])->hasFocus())
            {
                WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT)->DrawString(static_cast<SimpleMenuItem*> (mObjects[i])->desc.c_str(), mX
                                + mWidth + 10, mY + 15);
                mFont->SetColor(ARGB(255,255,255,0));
            }
            else
                mFont->SetColor(ARGB(150,255,255,255));
            (static_cast<SimpleMenuItem*> (mObjects[i]))->RenderWithOffset(-kLineHeight * startId);
        }
    }
}

void SimpleMenu::Update(float dt)
{
    JGuiController::Update(dt);
    if (mCurr > startId + maxItems - 1)
        startId = mCurr - maxItems + 1;
    else if (mCurr < startId) startId = mCurr;
    stars->Update(dt);
    selectionT += 3 * dt;
    selectionY += (selectionTargetY - selectionY) * 8 * dt;
    stars->MoveTo(mX + kHorizontalMargin + ((mWidth - 2 * kHorizontalMargin) * (1 + cos(selectionT)) / 2), selectionY + 5 * cos(
                    selectionT * 2.35f) + kLineHeight / 2 - kLineHeight * startId);
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
}

void SimpleMenu::Add(int id, const char * text, string desc, bool forceFocus)
{
    SimpleMenuItem * smi = NEW SimpleMenuItem(this, id, fontId, text, 0, mY + kVerticalMargin + mCount * kLineHeight,
                    (mCount == 0), autoTranslate);
    smi->desc = desc;
    JGuiController::Add(smi);
    if (mCount <= maxItems) mHeight += kLineHeight;
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
    SAFE_DELETE(SimpleMenu::stars);
    SAFE_DELETE(SimpleMenu::jewelTex);
}
