#include "PrecompiledHeader.h"

#include "TextScroller.h"
#include "WResourceManager.h"
#include "utils.h"
#include "WFont.h"


TextScroller::TextScroller(int fontId, float x, float y, float width, float scrollSpeed) :
    JGuiObject(0), fontId(fontId)
{
    mWidth = width;
    mScrollSpeed = scrollSpeed;
    mX = x;
    mY = y;
    start = -width;
    timer = 0;
    currentId = 0;
    mRandom = 0;
}

void TextScroller::setRandom(int mode)
{
    mRandom = mode;
    if (mRandom && strings.size())
    {
        currentId = (rand() % strings.size());
        mText = strings[currentId];
    }
}

void TextScroller::Add(string text)
{
    if (!strings.size()) 
        mText = text;
    strings.push_back(text);
}

void TextScroller::Reset()
{
    strings.clear();
    currentId = 0;
}

void TextScroller::Update(float dt)
{
    if (!strings.size()) return;

    start += mScrollSpeed * dt;
    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    if (start > mFont->GetStringWidth(mText.c_str()))
    {
            start = -mWidth;
            if (mRandom)
            {
                    currentId = (rand() % strings.size());
            }
            else
            {
                    currentId++;
                    if (currentId >= strings.size()) currentId = 0;
            }
            mText = strings[currentId];
    }
}

void TextScroller::Render()
{
    
    JQuadPtr fakebar;
    JTexture * tex = WResourceManager::Instance()->RetrieveTexture("phaseinfo.png");
        if (tex)
        {
            fakebar = WResourceManager::Instance()->RetrieveQuad("phaseinfo.png", 0.0f, 0.0f, tex->mWidth - 3.5f, tex->mHeight - 2.0f); //avoids weird rectangle around the texture because of bilinear filtering
        }
    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    mFont->SetColor(ARGB(128,255,255,255));
    if(fakebar.get())
    {
        if(mText.length() > 1)
        {
            float xscale = (SCREEN_WIDTH_F/2.6f) / fakebar->mWidth;
            float yscale = (mFont->GetHeight()+(mFont->GetHeight()/3.5f)) / fakebar->mHeight;
            fakebar->SetHotSpot(fakebar->mWidth-8.f,0);
            JRenderer::GetInstance()->RenderQuad(fakebar.get(),SCREEN_WIDTH_F, 4,0,xscale,yscale);
        }
    }
    mFont->DrawString(mText.c_str(), mX, mY, JGETEXT_LEFT, start, mWidth);
}

ostream& TextScroller::toString(ostream& out) const
{
    return out << "TextScroller ::: mText : " << mText 
                << " ; tempText : " << tempText 
                << " ; mWidth : " << mWidth
                << " ; mSpeed : " << mScrollSpeed 
                << " ; mX,mY : " << mX << "," << mY 
                << " ; start : " << start 
                << " ; timer : " << timer 
                << " ; strings : ?" 
                << " ; currentId : " << currentId 
                << " ; mRandom : " << mRandom;
}







VerticalTextScroller::VerticalTextScroller(int fontId, float x, float y, float width, float height, float scrollSpeed, size_t numItemsShown) :
TextScroller( fontId, x, y, width, scrollSpeed)
{
    mHeight = height;
    mNbItemsShown = numItemsShown;
    mMarginX = 0;
    timer=0;
    WFont *mFont = WResourceManager::Instance()->GetWFont(fontId);
    mOriginalY = mY;
    mMarginY = mY - mFont->GetHeight();
    Add("\n"); // initialize the scroller with a blank line

}


void VerticalTextScroller::Add( string text )
{
    strings.push_back( text );
    string wrappedText =  wordWrap(text, mWidth, fontId);
    mText.append(wrappedText);
}

/*
    Updates happen everytime the top line disappears from view.  
    The top line is then moved to the end of the file and the scrolling resumes where it left off

*/
void VerticalTextScroller::Update(float dt)
{
    if (!strings.size()) return;

    float currentYOffset = mScrollSpeed * dt;

    if ( mY <= mMarginY ) // top line has disappeared
    {
            timer = 0;
            // now readjust mText
            size_t nbLines = 1;
            vector<string> displayText = split( mText, '\n');
            vector<string> newDisplayText;
            for ( size_t i = nbLines; i < displayText.size(); ++i )
                    newDisplayText.push_back( displayText[i] );
            for ( size_t i = 0; i < nbLines; ++i )
                    newDisplayText.push_back( displayText[i] );

            mText = join( newDisplayText, "\n" );
            mY = mOriginalY;
    }
    ++timer;
    mY -= currentYOffset;
}

void VerticalTextScroller::Render()
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(fontId);
    JQuadPtr textscroller;
    JQuadPtr textscrollershadow;
#if !defined (PSP)
    textscroller = WResourceManager::Instance()->RetrieveTempQuad("textscroller.png");//new graphics textscroller
    textscrollershadow = WResourceManager::Instance()->RetrieveTempQuad("textscrollershadow.png");//new graphics textscroller shadow
    if(!mText.empty() && mText.length() > 1)
        if (textscrollershadow.get())
            JRenderer::GetInstance()->RenderQuad(textscrollershadow.get(), 0, 0, 0 ,SCREEN_WIDTH_F / textscrollershadow->mWidth, SCREEN_HEIGHT_F / textscrollershadow->mHeight);

    mFont->DrawString(mText.c_str(), mX, mY);

    if(!mText.empty() && mText.length() > 1)
        if (textscroller.get())
            JRenderer::GetInstance()->RenderQuad(textscroller.get(), 0, 0, 0 ,SCREEN_WIDTH_F / textscroller->mWidth, SCREEN_HEIGHT_F / textscroller->mHeight);
#else
    textscroller = WResourceManager::Instance()->RetrieveTempQuad("psptextscroller.png");//new graphics textscroller
    textscrollershadow = WResourceManager::Instance()->RetrieveTempQuad("psptextscrollershadow.png");//new graphics textscroller shadow
    if(!mText.empty() && mText.length() > 1)
        if (textscrollershadow.get())
            JRenderer::GetInstance()->RenderQuad(textscrollershadow.get(), 0, 0, 0 ,SCREEN_WIDTH_F / textscrollershadow->mWidth, SCREEN_HEIGHT_F / textscrollershadow->mHeight);

    mFont->DrawString(mText.c_str(), mX, mY);

    if(!mText.empty() && mText.length() > 1)
        if (textscroller.get())
            JRenderer::GetInstance()->RenderQuad(textscroller.get(), 0, 0, 0 ,SCREEN_WIDTH_F / textscroller->mWidth, SCREEN_HEIGHT_F / textscroller->mHeight);
#endif
}
