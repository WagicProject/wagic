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
    JGuiObject(id), parent(_parent), mX(x), mY(y), mFontId(fontId)
{
    if (autoTranslate)
        mText = _(text);
    else
        mText = text;
    
    mDescription = "";
    
    mHasFocus = hasFocus;

    mScale = 1.0f;
    mTargetScale = 1.0f;
    
    mXOffset = mX;

	if (hasFocus)
    {
        setIsSelectionValid(true);
        Entering();
    }
    else
    {
        setIsSelectionValid(false);
    }
}

void SimpleMenuItem::RenderWithOffset(float yOffset)
{
  mYOffset = yOffset;
  WFont * mFont = WResourceManager::Instance()->GetWFont(mFontId);
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
    setIsSelectionValid(true);
}

void SimpleMenuItem::Entering()
{
    checkUserClick();
    mHasFocus = true;
    if (parent != NULL)
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


/* Accessors */
float SimpleMenuItem::getX() const
{
    return mX;
}

float SimpleMenuItem::getY() const
{
    return mY;
}

void SimpleMenuItem::setFontId(const int &fontId)
{
    mFontId = fontId;
}

int SimpleMenuItem::getFontId() const
{
    return mFontId;
}

void SimpleMenuItem::setIsSelectionValid( bool validSelection )
{
    mIsValidSelection = validSelection;
}

bool SimpleMenuItem::isSelectionValid() const
{
    return mIsValidSelection;
}

void SimpleMenuItem::setFocus(bool value)
{
    mHasFocus = value;
}

bool SimpleMenuItem::hasFocus() const
{
    return mHasFocus;
}

string SimpleMenuItem::getDescription() const
{
    return mDescription;
}

void SimpleMenuItem::setDescription( const string& desc )
{
    mDescription = desc;
}

string SimpleMenuItem::getText() const
{
    return mText;
}

void SimpleMenuItem::setText( const string& text)
{
    mText = text;
}

float SimpleMenuItem::GetWidth() const
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(mFontId);
    mFont->SetScale(1.0);
    return mFont->GetStringWidth(mText.c_str());
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
