#include "../include/config.h"
#include "../include/SimpleMenuItem.h"


SimpleMenuItem::SimpleMenuItem(SimpleMenu* _parent, int id, JLBFont *font, const char* text, int x, int y, bool hasFocus): JGuiObject(id), parent(_parent), mFont(font), mX(x), mY(y)
{
  mText = text;
  mHasFocus = hasFocus;

  mScale = 1.0f;
  mTargetScale = 1.0f;

  if (hasFocus)
    Entering();
  mFont->SetScale(1.2f);
}


void SimpleMenuItem::RenderWithOffset(float yOffset)
{
  mFont->SetScale(mScale);
  mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(mText.c_str(), mX, mY + yOffset, JGETEXT_CENTER);
  mFont->SetScale(1.0f);
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
  return mFont->GetStringWidth(mText.c_str());
}
