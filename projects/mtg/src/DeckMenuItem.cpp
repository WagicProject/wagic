#include "PrecompiledHeader.h"

#include "DeckMenuItem.h"
#include "Translate.h"
#include "WResourceManager.h"
#include <algorithm>

#define ITEM_PX_WIDTH 190.0f

const int kHorizontalScrollSpeed = 30; // higher numbers mean faster scrolling

DeckMenuItem::DeckMenuItem(DeckMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus, bool autoTranslate, DeckMetaData *deckMetaData)
                        : JGuiObject(id), parent(_parent), fontId(fontId), mX(x), mY(y)
{
	WFont * mFont = resources.GetWFont(fontId);
    meta = deckMetaData;
	mText = trim(text);
	if (autoTranslate)
		mText = _(mText);	
    

    mHasFocus = hasFocus;
	float newImageWidth = 0.0f;
    if (meta && !meta->getGamesPlayed())
    {
        JTexture * tex = resources.RetrieveTexture("new.png");
        if (tex)
        {
            JQuad * quad = resources.RetrieveQuad("new.png", 2.0f, 2.0f, tex->mWidth - 4.0f, tex->mHeight - 4.0f); //avoids weird rectangle around the texture because of bilinear filtering
			newImageWidth = quad->mWidth;
		}
	}

	float titleStringWidth = mFont->GetStringWidth( mText.c_str() );
	mTitleResetWidth = (titleStringWidth - newImageWidth )/ 2; 
	mScrollEnabled = titleStringWidth  > ( ITEM_PX_WIDTH - newImageWidth );
	mScrollerOffset = 0.0f;

	if (hasFocus)
        Entering();

    if (meta && meta->getAvatarFilename().size() > 0)
        this->imageFilename = meta->getAvatarFilename();
    else
        this->imageFilename = "avatar.jpg";

	mDisplayInitialized = false;

}

void DeckMenuItem::Update(float dt)
{
	mScrollerOffset += kHorizontalScrollSpeed * dt;
	if ( (mScrollerOffset) > mTitleResetWidth )
		mScrollerOffset = -ITEM_PX_WIDTH;
}


void DeckMenuItem::RenderWithOffset(float yOffset)
{
    WFont * mFont = resources.GetWFont(fontId);
	
	if (!( mHasFocus && mScrollEnabled ))
		mScrollerOffset = 0;
	if (!mHasFocus && mScrollEnabled)
		mScrollerOffset = -1 * ( GetWidth() - ITEM_PX_WIDTH )/2;
	float offSet = mScrollerOffset;

	mFont->DrawString(mText.c_str(), mX, mY + yOffset, JGETEXT_CENTER, offSet, ITEM_PX_WIDTH);
	mDisplayInitialized = true;
	//Render a "new" icon for decks that have never been played yet
    if (meta && !meta->getGamesPlayed())
    {
        JTexture * tex = resources.RetrieveTexture("new.png");
        if (tex)
        {
            JQuad * quad = resources.RetrieveQuad("new.png", 2.0f, 2.0f, tex->mWidth - 4.0f, tex->mHeight - 4.0f); //avoids weird rectangle aroudn the texture because of bilinear filtering
            quad->SetHotSpot(quad->mWidth/2.0f, quad->mHeight/2.0f);
            float x = mX + min(ITEM_PX_WIDTH - quad->mWidth, GetWidth() )/2 + quad->mWidth/2;
            if (quad) JRenderer::GetInstance()->RenderQuad(quad, x , mY + yOffset + quad->mHeight/2, 0.5);
        }
    }
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
