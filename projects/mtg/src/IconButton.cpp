#include "PrecompiledHeader.h"

#include "IconButton.h"
#include "WResourceManager.h"
#include "WFont.h"

#define SCALE_SELECTED		1.2f
#define SCALE_NORMAL		1.0f

 IconButtonsController::IconButtonsController(float x, float y): JGuiController(0, NULL), mX(x), mY(y)
 {
     mListener = this;

 }
    
void IconButtonsController::SetColor(PIXEL_TYPE color)
{
    for (int i = 0; i < mCount; ++i)
        if (mObjects[i])
            ((IconButton *)mObjects[i])->SetColor(color);
}

IconButton::IconButton(int id, IconButtonsController * parent, string texture, float x, float y, float scale, int fontId, string text, float textRelativeX, float textRelativeY, bool hasFocus): JGuiObject(id)
{
    mQuad = NULL;
    mTex = WResourceManager::Instance()->RetrieveTexture(texture, RETRIEVE_LOCK);
    if (mTex)
    {
        mQuad = NEW JQuad(mTex, 0, 0, (float) mTex->mWidth, (float) mTex->mHeight);
    }
    init(parent, mQuad, x, y, scale, fontId, text, textRelativeX, textRelativeY, hasFocus);
}

IconButton::IconButton(int id, IconButtonsController * parent, JQuad * quad, float x, float y, float scale, int fontId, string text, float textRelativeX, float textRelativeY, bool hasFocus): JGuiObject(id)
{
    mTex = NULL;
    init(parent, quad, x, y, scale, fontId, text, textRelativeX, textRelativeY, hasFocus);
}

void IconButton::init(IconButtonsController * parent, JQuad * quad, float x, float y, float scale, int fontId, string text, float textRelativeX, float textRelativeY, bool hasFocus)
{
    mParent = parent;
    mQuad = quad;
    mX = x;
    mY = y;
    mScale = scale;
    mFontId = fontId;
    mText = text;
    mTextRelativeX = textRelativeX;
    mTextRelativeY = textRelativeY;
    mHasFocus = hasFocus;

    mCurrentScale = scale;
    mTargetScale =  mHasFocus ? SCALE_SELECTED * mScale : SCALE_NORMAL * mScale;
    SetColor(ARGB(255,255,255,255));
}

void IconButton::SetColor(PIXEL_TYPE color)
{
    mColor = color;
}

bool IconButton::hasFocus()
{
    return mHasFocus;
}

void IconButton::Render()
{
    JRenderer * r = JRenderer::GetInstance();

    float relX = mX + mParent->mX;
    float relY = mY + mParent->mY;

    if (mQuad)
    {
        mQuad->SetColor(mColor);
        r->RenderQuad(mQuad, relX, relY, 0, mCurrentScale, mCurrentScale);
    }
    if (mText.size())
    {
        WFont * mFont = WResourceManager::Instance()->GetWFont(mFontId);
        PIXEL_TYPE backup = mFont->GetColor();
        mFont->SetColor(ARGB(255,0,0,0));
        //TODO adapt if mTextRelativeX/Y/align are negative/positive
        mFont->DrawString(mText.c_str(), relX + mTextRelativeX  , relY + mTextRelativeY , JGETEXT_CENTER);
        mFont->SetColor(backup);
    }

}

void IconButton::Update(float dt)
{
    if (mCurrentScale < mTargetScale)
    {
        mCurrentScale += 8.0f * dt;
        if (mCurrentScale > mTargetScale) mCurrentScale = mTargetScale;
    }
    else if (mCurrentScale > mTargetScale)
    {
        mCurrentScale -= 8.0f * dt;
        if (mCurrentScale < mTargetScale) mCurrentScale = mTargetScale;
    }
}

void IconButton::Entering()
{
    mHasFocus = true;
    mTargetScale = SCALE_SELECTED * mScale;
}

bool IconButton::Leaving(JButton key)
{
    mHasFocus = false;
    mTargetScale = SCALE_NORMAL * mScale;
    return true;
}

bool IconButton::ButtonPressed()
{
    return true;
}

IconButton::~IconButton()
{
    if (mTex)
    {
        WResourceManager::Instance()->Release(mTex);
        SAFE_DELETE(mQuad);
    }

}

ostream& IconButton::toString(ostream& out) const
{
    return out << "IconButton ::: mHasFocus : " << mHasFocus;
}