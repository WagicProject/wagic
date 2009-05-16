#include "../include/config.h"
#include "../include/SimpleMenuItem.h"
#include "../include/Translate.h"

SimpleMenuItem::SimpleMenuItem(SimpleMenu* _parent, int id, JLBFont *font, string text, int x, int y, bool hasFocus): JGuiObject(id), parent(_parent), mFont(font), mX(x), mY(y)
{
  mText = _(text);
  mHasFocus = hasFocus;

  mScale = 1.0f;
  mTargetScale = 1.0f;

  if (hasFocus)
    Entering();
}


void SimpleMenuItem::RenderWithOffset(float yOffset)
{
  //mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(mText.c_str(), mX, mY + yOffset, JGETEXT_CENTER);
}

void SimpleMenuItem::Render()
{
  RenderWithOffset(0);
}

void SimpleMenuItem::Update(float dt)
{
    if (mScale < mTargetScale)
    {
      mScale += 8.0f*dt;
      if (mScale > mTargetScale)
	mScale = mTargetScale;
    }
  else if (mScale > mTargetScale)
    {
      mScale -= 8.0f*dt;
      if (mScale < mTargetScale)
	mScale = mTargetScale;
	}
}


void SimpleMenuItem::Entering()
{
  mHasFocus = true;
  parent->selectionTargetY = mY;
}


bool SimpleMenuItem::Leaving(u32 key)
{
  mHasFocus = false;
  return true;
}


bool SimpleMenuItem::ButtonPressed()
{
  return true;
}

void SimpleMenuItem::Relocate(int x, int y)
{
  mX = x;
  mY = y;
}

int SimpleMenuItem::GetWidth()
{
  mFont->SetScale(1.0);
  return mFont->GetStringWidth(mText.c_str());
}

bool SimpleMenuItem::hasFocus()
{
  return mHasFocus;
}
