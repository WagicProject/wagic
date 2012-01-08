#include "PrecompiledHeader.h"

#include "DeckMenuItem.h"
#include "Translate.h"
#include "WResourceManager.h"
#include <algorithm>

#define ITEM_PX_WIDTH 190.0f
#define kItemXOffset 22
#define kItemYHeight 30

const int kHorizontalScrollSpeed = 30; // higher numbers mean faster scrolling

DeckMenuItem::DeckMenuItem(DeckMenu* _parent, int id, int iFontId, string text, float x, float y, bool hasFocus, bool autoTranslate, DeckMetaData *deckMetaData): SimpleMenuItem(id)
{
    mEngine = JGE::GetInstance();
    parent = _parent;
    fontId = iFontId;
    mY = y;
    mX = x;
	WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    meta = deckMetaData;
	mText = trim(text);
    
	if (autoTranslate)
		mText = _(mText);	
    
    mXOffset = kItemXOffset;

    mHasFocus = hasFocus;
	float newImageWidth = 0.0f;
    if (meta && !meta->getGamesPlayed())
    {
        JTexture * tex = WResourceManager::Instance()->RetrieveTexture("new.png");
        if (tex)
        {
            JQuadPtr quad = WResourceManager::Instance()->RetrieveQuad("new.png", 2.0f, 2.0f, tex->mWidth - 4.0f, tex->mHeight - 4.0f); //avoids weird rectangle around the texture because of bilinear filtering
			newImageWidth = quad->mWidth;
		}
	}

	float titleStringWidth = mFont->GetStringWidth( mText.c_str() );
	mTitleResetWidth = (titleStringWidth - newImageWidth )/ 2; 
	mScrollEnabled = titleStringWidth  > ( ITEM_PX_WIDTH - newImageWidth );
	mScrollerOffset = 0.0f;

	if (hasFocus)
    {
        mIsValidSelection = true;
        Entering();
    }
    
    if (meta && meta->getAvatarFilename().size() > 0)
        this->imageFilename = meta->getAvatarFilename();
    else 
    {
        // this is a non-deck menu item (ie "Random", "Cancel", etc
        switch(id)
        {
            case kRandomPlayerMenuID:
                this->imageFilename = "noavatar.jpg";
                break;
            case kRandomAIPlayerMenuID:
                this->imageFilename = "noavatar.jpg";
                break;
            case kEvilTwinMenuID:
                {
                    this->imageFilename = "EvilTwinAvatar";
                    break;
                }
            default:
                this->imageFilename = "noavatar.jpg";
                // do nothing.  
                break;
        }
        
    }
    
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
    SimpleMenuItem::mYOffset = yOffset;

    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
	
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
        JTexture * tex = WResourceManager::Instance()->RetrieveTexture("new.png");
        if (tex)
        {
            JQuadPtr quad = WResourceManager::Instance()->RetrieveQuad("new.png", 2.0f, 2.0f, tex->mWidth - 4.0f, tex->mHeight - 4.0f); //avoids weird rectangle aroudn the texture because of bilinear filtering
            quad->SetHotSpot(quad->mWidth/2.0f, quad->mHeight/2.0f);
            float x = mX + min(ITEM_PX_WIDTH - quad->mWidth, GetWidth() )/2 + quad->mWidth/2;
            if (quad) JRenderer::GetInstance()->RenderQuad(quad.get(), x , mY + yOffset + quad->mHeight/2, 0.5);
        }
    }
}

void DeckMenuItem::Render() 
{
    RenderWithOffset(0);
}

void DeckMenuItem::Relocate(float x, float y)
{
    mX = x;
    mY = y;
}

void DeckMenuItem::Entering()
{
    checkUserClick();
    mHasFocus = true;
    parent->mSelectionTargetY = mY;
}


bool DeckMenuItem::Leaving(JButton key)
{
    return SimpleMenuItem::Leaving(key);
}

bool DeckMenuItem::ButtonPressed()
{
    return SimpleMenuItem::ButtonPressed();
}

float DeckMenuItem::GetWidth()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
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
