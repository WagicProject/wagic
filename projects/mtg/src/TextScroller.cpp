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
	WFont * mFont = resources.GetWFont(fontId);
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
    WFont * mFont = resources.GetWFont(fontId);
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
	mScrollerInitialized = false;
	timer=0;
	WFont *mFont = resources.GetWFont(fontId);
	mOriginalY = mY;
	mMarginY = mY - mFont->GetHeight();
	Add("\n"); // initialize the scroller with a blank line

}


void VerticalTextScroller::Add( string text )
{
	strings.push_back( text );
	string wrappedText =  wordWrap(text, mWidth);
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
    WFont * mFont = resources.GetWFont(fontId);
	mFont->DrawString(mText.c_str(), mX, mY);
}



// This is a customized word wrap based on pixel width.  It tries it's best 
// to wrap strings using spaces as delimiters.  
// Not sure how this translates into non-english fonts.
std::string VerticalTextScroller::wordWrap(std::string sentence, float width)
{
	WFont * mFont = resources.GetWFont(fontId);
	float lineWidth = mFont->GetStringWidth( sentence.c_str() );
	string retVal = sentence;
	if ( lineWidth < width ) return sentence;
   
	int numLines = 1;
	int breakIdx = 0;
	for( size_t idx = 0; idx < sentence.length(); idx ++ )
    {
		if ( sentence[idx] == ' ' )
		{
			string currentSentence = sentence.substr(breakIdx, idx - breakIdx);
			float stringLength = mFont->GetStringWidth( currentSentence.c_str() );
			if (stringLength >= width)
			{				
				if ( stringLength > width )
				{
					while ( sentence[idx-1] != ' ' )
						idx--;
				}
				retVal[idx-1] = '\n';				
				breakIdx = idx;
				numLines++;
			}
		}
    }
	if ( numLines * mFont->GetHeight() > mHeight )
		mScrollerInitialized = true;
	
    return retVal;
}
