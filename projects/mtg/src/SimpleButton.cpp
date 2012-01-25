//
//  SimpleButton.cpp
//  wagic
//
//  Created by Michael Nguyen on 1/23/12.
//  Copyright (c) 2012 Wagic the Homebrew. All rights reserved.
//

#include "PrecompiledHeader.h"

#include "SimpleButton.h"
#include "Translate.h"
#include "WFont.h"
#include "WResourceManager.h"

SimpleButton::SimpleButton(int id): JGuiObject(id)
{
    mIsValidSelection = false;
}

SimpleButton::SimpleButton(JGuiController* _parent, int id, int fontId, string text, float x, float y, bool hasFocus, bool autoTranslate) :
JGuiObject(id), parent(_parent), mX(x), mY(y), mFontId(fontId)
{
    mYOffset = 0;
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
        setIsSelectionValid(true);
        Entering();
    }
    else
    {
        setIsSelectionValid(false);
    }
}


void SimpleButton::RenderWithOffset(float yOffset)
{
    mYOffset = yOffset;
    WFont * mFont = WResourceManager::Instance()->GetWFont(mFontId);
    mFont->DrawString(mText.c_str(), mX, mY + yOffset, JGETEXT_CENTER);
}

void SimpleButton::Render()
{
    RenderWithOffset(0);
}

void SimpleButton::Update(float dt)
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

void SimpleButton::checkUserClick()
{
    setIsSelectionValid(true);
}

void SimpleButton::Entering()
{
    checkUserClick();
    mHasFocus = true;
}

bool SimpleButton::Leaving(JButton key)
{
    checkUserClick();
    mHasFocus = false;
    return true;
}

bool SimpleButton::ButtonPressed()
{
    return mIsValidSelection;
}


void SimpleButton::Relocate(float x, float y)
{
    mXOffset = 0;
    mX = x;
    mY = y;
}

/* Accessors */
JGuiController* SimpleButton::getParent() const
{
    return parent;
}

float SimpleButton::getScale() const
{
    return mScale;
}

float SimpleButton::getTargetScale() const
{
    return mTargetScale;
}

float SimpleButton::getX() const
{
    return mX;
}

float SimpleButton::getY() const
{
    return mY;
}

void SimpleButton::setFontId(const int &fontId)
{
    mFontId = fontId;
}

int SimpleButton::getFontId() const
{
    return mFontId;
}

void SimpleButton::setIsSelectionValid( bool validSelection )
{
    mIsValidSelection = validSelection;
}

bool SimpleButton::isSelectionValid() const
{
    return mIsValidSelection;
}

void SimpleButton::setFocus(bool value)
{
    mHasFocus = value;
}

bool SimpleButton::hasFocus() const
{
    return mHasFocus;
}


string SimpleButton::getText() const
{
    return mText;
}

void SimpleButton::setText( const string& text)
{
    mText = text;
}

float SimpleButton::GetWidth()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(mFontId);
    mFont->SetScale(1.0);
    return mFont->GetStringWidth(mText.c_str());
}

ostream& SimpleButton::toString(ostream& out) const
{
    return out << "SimpleButton ::: mHasFocus : " << hasFocus()
    << " ; parent : " << getParent()
    << " ; mText : " << getText()
    << " ; mScale : " << getScale()
    << " ; mTargetScale : " << getTargetScale()
    << " ; mX,mY : " << getX() << "," << getY();
}
