#include "PrecompiledHeader.h"

#include "DeckMenuItem.h"
#include "Translate.h"
#include "WResourceManager.h"
#include <algorithm>

#define ITEM_PX_WIDTH 190.0f
#define kItemXOffset 22
#define kItemYHeight 30

const int kHorizontalScrollSpeed = 30; // higher numbers mean faster scrolling

DeckMenuItem::DeckMenuItem(DeckMenu* _parent, int id, int fontId, string text, float x, float y, bool hasFocus, bool autoTranslate, DeckMetaData *deckMetaData): SimpleMenuItem(NULL, id, fontId, text, x, y, hasFocus, autoTranslate)
{
    mEngine = JGE::GetInstance();
    deckController = _parent;

	WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    meta = deckMetaData;
	mText = trim(text);
    
	if (autoTranslate)
		mText = _(mText);	
    
    mXOffset = kItemXOffset;

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
        setIsSelectionValid( true );
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

    WFont * mFont = WResourceManager::Instance()->GetWFont(getFontId());
	
	if (!( hasFocus() && mScrollEnabled ))
		mScrollerOffset = 0;
	if (!hasFocus() && mScrollEnabled)
		mScrollerOffset = -1 * ( GetWidth() - ITEM_PX_WIDTH )/2;
	float offSet = mScrollerOffset;

	mFont->DrawString( getText().c_str(), getX(), getY() + yOffset, JGETEXT_CENTER, offSet, ITEM_PX_WIDTH);
	mDisplayInitialized = true;
	//Render a "new" icon for decks that have never been played yet
    if (meta && !meta->getGamesPlayed())
    {
        JTexture * tex = WResourceManager::Instance()->RetrieveTexture("new.png");
        if (tex)
        {
            JQuadPtr quad = WResourceManager::Instance()->RetrieveQuad("new.png", 2.0f, 2.0f, tex->mWidth - 4.0f, tex->mHeight - 4.0f); //avoids weird rectangle aroudn the texture because of bilinear filtering
            quad->SetHotSpot(quad->mWidth/2.0f, quad->mHeight/2.0f);
            float x = getX() + min(ITEM_PX_WIDTH - quad->mWidth, GetWidth() )/2 + quad->mWidth/2;
            if (quad) 
                JRenderer::GetInstance()->RenderQuad(quad.get(), x , getY() + yOffset + quad->mHeight/2, 0.5);
        }
    }
}

void DeckMenuItem::Render() 
{
    RenderWithOffset(0);
}

void DeckMenuItem::Relocate(float x, float y)
{
    setX( x );
    setY( y );
}

void DeckMenuItem::Entering()
{
    checkUserClick();
    setFocus(true);
    deckController->mSelectionTargetY = getY();
}


bool DeckMenuItem::Leaving(JButton key)
{
    return SimpleMenuItem::Leaving(key);
}

bool DeckMenuItem::ButtonPressed()
{
    return SimpleMenuItem::ButtonPressed();
}


void DeckMenuItem::checkUserClick()
{
    int x1 = -1, y1 = -1;
    if (mEngine->GetLeftClickCoordinates(x1, y1))
    {   
        SimpleMenuItem::setIsSelectionValid( false );
        int x2 = static_cast<int>(mXOffset), y2 = static_cast<int>(getY() + mYOffset);
        if ( (x1 >= x2) && (x1 <= (x2 + 200)) && (y1 >= y2) && (y1 < (y2 + 30)))
            setIsSelectionValid( true );
    }
    else
        setIsSelectionValid( true );
}

// Accessors
float DeckMenuItem::GetWidth()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(getFontId());
    return mFont->GetStringWidth(getText().c_str());
}

ostream& DeckMenuItem::toString(ostream& out) const
{
    return out << "DeckMenuItem ::: mHasFocus : " << hasFocus()
                 << " ; parent : " << deckController
                 << " ; mText : " << getText()
                 << " ; mX,mY : " << getX() << "," << getY();
}

JGuiController* DeckMenuItem::getParent() const
{
    return deckController;
}


DeckMenuItem::~DeckMenuItem()
{
    meta = NULL;
}
