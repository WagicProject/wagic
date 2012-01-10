#include "PrecompiledHeader.h"

#include "SimpleMenuItem.h"
#include "Translate.h"
#include "WResourceManager.h"

float SimpleMenuItem::mYOffset = 0;

SimpleMenuItem::SimpleMenuItem(int id): JGuiObject(id)
{
    mIsValidSelection = false;
}

SimpleMenuItem::SimpleMenuItem(SimpleMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus, bool autoTranslate) :
    JGuiObject(id), parent(_parent), fontId(fontId), mX(x), mY(y)
{
    if (autoTranslate)
        mText = _(text);
    else
        mText = text;
    mHasFocus = hasFocus;

    mScale = 1.0f;
    mTargetScale = 1.0f;
    
    mXOffset = mX;

	if (hasFocus)
    {
        mIsValidSelection = true;
        Entering();
    }
}

void SimpleMenuItem::RenderWithOffset(float yOffset)
{
  mYOffset = yOffset;
  WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
  mFont->DrawString(mText.c_str(), mX, mY + yOffset, JGETEXT_CENTER);
}

void SimpleMenuItem::Render()
{
    RenderWithOffset(0);
}

void SimpleMenuItem::Update(float dt)
{
    if (mScale < mTargetScale)
    {
        mScale += 8.0f * dt;
        if (mScale > mTargetScale) mScale = mTargetScale;
    }
    else if (mScale > mTargetScale)
    {
        mScale -= 8.0f * dt;
        if (mScale < mTargetScale) mScale = mTargetScale;
    }
}

void SimpleMenuItem::checkUserClick()
{
    mIsValidSelection = true;
}

void SimpleMenuItem::Entering()
{
    checkUserClick();
    mHasFocus = true;
    parent->selectionTargetY = mY;
}

bool SimpleMenuItem::Leaving(JButton key)
{
    checkUserClick();
    mHasFocus = false;
    return true;
}

bool SimpleMenuItem::ButtonPressed()
{
    return mIsValidSelection;
}


void SimpleMenuItem::Relocate(float x, float y)
{
    mXOffset = x - (parent->getWidth()/2); // determines the leftmost point of the text;
    mX = x;
    mY = y;
}

float SimpleMenuItem::GetWidth()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    mFont->SetScale(1.0);
    return mFont->GetStringWidth(mText.c_str());
}

bool SimpleMenuItem::hasFocus()
{
    return mHasFocus;
}

ostream& SimpleMenuItem::toString(ostream& out) const
{
    return out << "SimpleMenuItem ::: mHasFocus : " << mHasFocus
                << " ; parent : " << parent
                << " ; mText : " << mText
                << " ; mScale : " << mScale
                << " ; mTargetScale : " << mTargetScale
                << " ; mX,mY : " << mX << "," << mY;
}
