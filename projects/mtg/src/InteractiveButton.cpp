//
//  InteractiveButton.cpp
//  wagic
//
//  Created by Michael Nguyen on 1/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "PrecompiledHeader.h"

#include "InteractiveButton.h"
#include "Translate.h"
#include "JTypes.h"
#include "WResourceManager.h"
#include "WFont.h"

const int kButtonHeight = 30;

InteractiveButton::InteractiveButton(JGuiController* _parent, int id, int fontId, string text, float x, float y, JButton actionKey, bool hasFocus, bool autoTranslate) :
SimpleButton( _parent, id, fontId, text, x, y, hasFocus, autoTranslate)
{
    setIsSelectionValid(false); // by default it's turned off since you can't auto select it.
    mActionKey = actionKey;
}

void InteractiveButton::Entering()
{  
}

void InteractiveButton::checkUserClick()
{
    int x1 = -1, y1 = -1;
    if (mEngine->GetLeftClickCoordinates(x1, y1))
    {   
        setIsSelectionValid(false);
        int buttonImageWidth = static_cast<int>(GetWidth());
        int x2 = static_cast<int>(getX()), y2 = static_cast<int>(getY() + mYOffset);
        int buttonHeight = kButtonHeight;
        if ( (x1 >= x2) && (x1 <= (x2 + buttonImageWidth)) && (y1 >= y2) && (y1 < (y2 + buttonHeight)))
            setIsSelectionValid( true );
    }
    else
        setIsSelectionValid( false );
}

bool InteractiveButton::ButtonPressed()
{
    checkUserClick();
    if (isSelectionValid())
    {
        mEngine->ReadButton();
        mEngine->LeftClickedProcessed();
        mEngine->HoldKey_NoRepeat( mActionKey );
        setIsSelectionValid(false);
        return true;
    }
    
    return false;
}

void InteractiveButton::Render()
{
    if (!isSelectionValid()) return;
    JRenderer *renderer = JRenderer::GetInstance();
    WFont *mainFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    const string detailedInfoString = _(getText());
    float stringWidth = mainFont->GetStringWidth(detailedInfoString.c_str());
    float pspIconsSize = 0.5;
    float mainFontHeight = mainFont->GetHeight();
    float boxStartX =  getX() - 5;
    mXOffset = 0;
    mYOffset = 0;

#ifndef TOUCH_ENABLED
    renderer->FillRoundRect(boxStartX, getY(), stringWidth - 3, mainFontHeight - 9, 5, ARGB(0, 0, 0, 0));
#else
    renderer->FillRoundRect(boxStartX, getY(), stringWidth - 3, mainFontHeight - 9, 5, ARGB(255, 192, 172, 119));
    renderer->DrawRoundRect(boxStartX, getY(), stringWidth - 3, mainFontHeight - 9, 5, ARGB(255, 255, 255, 255));
#endif

	float buttonXOffset = getX() - mXOffset;
    float buttonYOffset = getY() + mYOffset;
    if (buttonImage != NULL)
    {
        renderer->RenderQuad(buttonImage.get(), buttonXOffset - buttonImage.get()->mWidth/2, buttonYOffset + mainFontHeight/2, 0, pspIconsSize, pspIconsSize);
    }
    mainFont->SetColor(ARGB(255, 200, 200, 200));
    mainFont->DrawString(detailedInfoString, buttonXOffset, buttonYOffset);
}

void InteractiveButton::setImage( const JQuadPtr imagePtr )
{
    buttonImage = imagePtr;
    float imageXOffset = getX() - buttonImage.get()->mWidth;
    
    if (imageXOffset < 0)
        setX( getX() - imageXOffset/2 + 5 );
}

/* Accessors */

ostream& InteractiveButton::toString(ostream& out) const
{
    return out << "InteractiveButton ::: mHasFocus : " << hasFocus()
    << " ; parent : " << getParent()
    << " ; mText : " << getText()
    << " ; mScale : " << getScale()
    << " ; mTargetScale : " << getTargetScale()
    << " ; mX,mY : " << getX() << "," << getY();
}
