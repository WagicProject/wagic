#include "PrecompiledHeader.h"

#include "DeckMenuItem.h"
#include "Translate.h"
#include "WResourceManager.h"

#define ITEM_PX_WIDTH 190.f
const int kHorizontalScrollSpeed = 10; // lower numbers mean faster scrolling

DeckMenuItem::DeckMenuItem(DeckMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus, bool autoTranslate, DeckMetaData *deckMetaData)
                        : JGuiObject(id), parent(_parent), fontId(fontId), mX(x), mY(y)
{
	mText = trim(text);
	displayName = text;
	if (autoTranslate)
		mText = _(mText);	
    
	WFont * mFont = resources.GetWFont(fontId);
    while (mFont->GetStringWidth(displayName.c_str()) > ITEM_PX_WIDTH)
        displayName.erase(displayName.size() - 1);

	mScrollTimer = 0;
    mHasFocus = hasFocus;
	mScrollEnabled = (displayName.length() != mText.length()) ;

	if (mScrollEnabled)
		mText.append("   "); // add padding to reduce jerkiness when text scrolls

	mRemainder = ( mText.length() - displayName.length() );


	if (hasFocus)
        Entering();

    meta = deckMetaData;
    if (meta && meta->getAvatarFilename().size() > 0)
        this->imageFilename = meta->getAvatarFilename();
    else
        this->imageFilename = "avatar.jpg";

}

void DeckMenuItem::RenderWithOffset(float yOffset)
{
    WFont * mFont = resources.GetWFont(fontId);
	string menuItemString = displayName;
	size_t offset = 0;
	
	if ( mHasFocus && mScrollEnabled )
	{
		offset = mScrollTimer / kHorizontalScrollSpeed;
		int wrapIndexEnd = mText.length() - offset;
		int nbWrapAroundChars =  displayName.length() - wrapIndexEnd;
		menuItemString = mText.substr(offset, displayName.length());
		if ( nbWrapAroundChars > 0 )
			// need to append start of title to end of menuItemString
			menuItemString.append( mText.substr(0, nbWrapAroundChars ) );
	}

	mFont->DrawString(menuItemString.c_str(), mX, mY + yOffset, JGETEXT_CENTER);
	if ( mHasFocus && mScrollEnabled && offset == mText.length())
		mScrollTimer = 0;
	else if (mHasFocus && mScrollEnabled)
		mScrollTimer++;
}

void DeckMenuItem::Render()
{
    RenderWithOffset(0);
}

void DeckMenuItem::Entering()
{
    mHasFocus = true;
    parent->mSelectionTargetY = mY;
}

bool DeckMenuItem::Leaving(JButton key)
{
    mHasFocus = false;
    return true;
}

bool DeckMenuItem::ButtonPressed()
{
    return true;
}

void DeckMenuItem::Relocate(float x, float y)
{
    mX = x;
    mY = y;
}

float DeckMenuItem::GetWidth()
{
    WFont * mFont = resources.GetWFont(fontId);
    return mFont->GetStringWidth(mText.c_str());
}

bool DeckMenuItem::hasFocus()
{
    return mHasFocus;
}

ostream& DeckMenuItem::toString(ostream& out) const
{
    return out << "DeckMenuItem ::: mHasFocus : " << mHasFocus
                 << " ; parent : " << parent
                 << " ; mText : " << mText
                 << " ; mX,mY : " << mX << "," << mY;
}

DeckMenuItem::~DeckMenuItem()
{
    meta = NULL;
}
