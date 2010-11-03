#include "PrecompiledHeader.h"

#include "TextScroller.h"
#include "WResourceManager.h"
#include "utils.h"
#include "WFont.h"

enum {
  HORIZONTAL_SCROLLER = 0,
  VERTICAL_SCROLLER = 1
};


TextScroller::TextScroller(int fontId, float x, float y, float width, float speed, int scrollerType, int numItems ): JGuiObject(0), fontId(fontId){
  mWidth = width;
  mSpeed = speed;
  minimumItems = numItems;
  mX = x;
  mY = y;
  start = -width;
  timer = 0;
  currentId = 0;
  mRandom = 0;
  scrollDirection = scrollerType;
}

void TextScroller::setRandom(int mode){
  mRandom = mode;
  if (mRandom && strings.size()){
    currentId = (rand() % strings.size());
    mText = strings[currentId];
  }
}

void TextScroller::Add(string text){
  if (!strings.size()) mText = text;
  strings.push_back(text);
}

void TextScroller::Reset(){
  strings.clear();
}

void TextScroller::Update(float dt){
  if(!strings.size())
    return;
  if ( scrollDirection == HORIZONTAL_SCROLLER )
  {
    start+=mSpeed*dt;
    WFont * mFont = resources.GetWFont(fontId);
    if (start > mFont->GetStringWidth(mText.c_str())){
      start = -mWidth;
      if (mRandom){
        currentId = (rand() % strings.size());
      }else{
        currentId++;
        if (currentId > strings.size()-1)currentId = 0;
      }
      mText = strings[currentId];
    }
  }
  else
  {
    // we want to display 2 at a time
    ostringstream scrollerText;
    if ( timer == 0 )
    {
      size_t nbItemsToDisplay = ( static_cast <unsigned> (minimumItems) < strings.size() ? minimumItems : strings.size()); //MIN(minimumItems, strings.size())
      for ( size_t idx = 0; idx < nbItemsToDisplay; idx ++ )
      {
        scrollerText << strings[currentId + idx];
      }
      currentId++;
      if ( currentId >= (strings.size()-1) )
        currentId = 0;
      mText = wordWrap( scrollerText.str(), mWidth );
    }
    timer = ++timer % ((int) mSpeed);
  }
}

void TextScroller::Render(){
  WFont * mFont = resources.GetWFont(fontId);
  if ( scrollDirection == HORIZONTAL_SCROLLER )
    mFont->DrawString(mText.c_str(),mX,mY,JGETEXT_LEFT,start,mWidth);
  else
    mFont->DrawString(mText.c_str(), mX, mY );
}

ostream& TextScroller::toString(ostream& out) const
{
  return out << "TextScroller ::: mText : " << mText
	     << " ; tempText : " << tempText
	     << " ; mWidth : " << mWidth
	     << " ; mSpeed : " << mSpeed
	     << " ; mX,mY : " << mX << "," << mY
	     << " ; start : " << start
	     << " ; timer : " << timer
	     << " ; strings : ?" // << strings
	     << " ; currentId : " << currentId
	     << " ; mRandom : " << mRandom;
}
