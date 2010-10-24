#include "PrecompiledHeader.h"

#include "PlayGuiObject.h"

using namespace std;

PlayGuiObject::PlayGuiObject(float desiredHeight, float x, float y, bool hasFocus) : JGuiObject(0), Pos(x, y, 1.0, 0.0, 255) {
  defaultHeight = desiredHeight;
  mHeight = desiredHeight;
  mHasFocus = hasFocus;
  type = 0;
  wave = 0;
}
PlayGuiObject::PlayGuiObject(float desiredHeight, const Pos& ref, bool hasFocus) : JGuiObject(0), Pos(ref) {
  defaultHeight = desiredHeight;
  mHeight = desiredHeight;
  mHasFocus = hasFocus;
  type = 0;
  wave = 0;
}


void PlayGuiObject::Update(float dt){
  if (mHasFocus && mHeight < defaultHeight * 1.2)
    {
      mHeight += defaultHeight*0.8f*dt;
      //      fprintf(stderr, "increasing size to %f - %d", mHeight, GetId() );

      if (mHeight > defaultHeight * 1.2)
	mHeight = defaultHeight * 1.2;
    }
  else if (!mHasFocus && mHeight > defaultHeight)
    {
      mHeight -= defaultHeight*0.8f*dt;
      if (mHeight < defaultHeight)
	mHeight = defaultHeight;
    }
  wave = (wave +2 * (int) (100 * dt) ) % 255;
  for (vector<Effect*>::iterator it = effects.begin(); it != effects.end(); ++it)
    (*it)->Update(dt);
  Pos::Update(dt);
}

void PlayGuiObject::Render()
{
  for (vector<Effect*>::iterator it = effects.begin(); it != effects.end(); ++it)
    (*it)->Render();
}
