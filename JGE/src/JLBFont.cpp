//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------



#include "../include/JLBFont.h"
#include "../include/JFileSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


JRenderer* JLBFont::mRenderer = NULL;

JLBFont::JLBFont(const char *fontname, int lineheight, bool useVideoRAM)
{
	mRenderer = JRenderer::GetInstance();

	mHeight = 0;
	mScale = 1.0f;
	mRotation = mSpacing = 0.0f;
	mTracking = 0.0f;
	mColor = ARGB(255,255,255,255);

	mTexture = NULL;
	mQuad = NULL;
	mBase = 0;

	char filename[256];
	short buffer[1024];


	sprintf(filename, "%s.dat", fontname);

	//FILE *file;
	JFileSystem *fileSys = JFileSystem::GetInstance();
	if (!fileSys->OpenFile(filename)) return;

	fileSys->ReadFile((u8 *)buffer, 2048);
	fileSys->CloseFile();

    sprintf(filename, "%s.png", fontname);
	mTexture = mRenderer->LoadTexture(filename, useVideoRAM);

	if (mTexture == NULL) return;

	mHeight = (float) lineheight;

	mQuad = new JQuad(mTexture, 0.0f, 0.0f, 16.0f, mHeight);

	float a, b, c;

    float y = 0.0f;
	float x;

	// there are 16x16 characters in the texture map.
	float cellHeight = mTexture->mHeight/16.0f;

	int n = 0;
	for (int i=0;i<16;i++)
	{
		for (int j=0;j<16;j++)
		{
			x = (float)buffer[n*4];			// x offset
			a = (float)buffer[n*4+1];		// character width
			b = (float)buffer[n*4+2];
			c = (float)buffer[n*4+3];

			mXPos[n] = x;
			mYPos[n] = y;
			mCharWidth[n] = a+b+c;

			n++;
		}

		y += cellHeight;
	}

}


JLBFont::~JLBFont()
{
	if (mQuad)
		delete mQuad;

	if (mTexture)
		delete mTexture;

//	JGERelease();
}


void JLBFont::DrawString(const char *string, float x, float y, int align, float leftOffset, float displayWidth)
{
	char *p = (char*)string;
	float dx = x, dy = y;

	if (mQuad == NULL) return;

	float width = GetStringWidth(string);

	if (align == JGETEXT_RIGHT)
		dx -= width;
	else if (align == JGETEXT_CENTER)
		dx -= width/2;

	dx = floorf(dx);
	dy = floorf(dy);
  float x0 = dx;
	int index;
	while (*p)
	{
		index = (*p - 32)+mBase;
    float charWidth = mCharWidth[index];
    float delta = (charWidth + mTracking) * mScale;
    float xPos =  mXPos[index];
    if (leftOffset){
      if (leftOffset < 0){
        dx-=leftOffset;
        leftOffset = 0;
        continue;
      }else if (leftOffset - delta > 0){
        leftOffset -= delta;
        p++;
        continue;
      }else{
        xPos = mXPos[index] + (leftOffset);
         delta -= leftOffset;
        leftOffset = 0;
       
        charWidth = (delta/mScale) - mTracking;
      }
    }
    else if (displayWidth){
      if (dx > x0+displayWidth)  return;
      if (dx+delta > x0+displayWidth) {
        delta = x0 + displayWidth - dx;
        charWidth = (delta/mScale) - mTracking;
      }
    }
		mQuad->SetTextureRect(xPos, mYPos[index], charWidth , mHeight);
		mRenderer->RenderQuad(mQuad, dx, dy, mRotation, mScale, mScale);
		dx += delta;
		p++;

   }

}


void JLBFont::printf(float x, float y, const char *format, ...)
{
	char buffer[PRINTF_BUFFER_SIZE];
	va_list list;

	va_start(list, format);
	vsprintf(buffer, format, list);
	va_end(list);

	DrawString(buffer, x, y);
}


void JLBFont::SetColor(PIXEL_TYPE color)
{
    mColor = color;
    mQuad->SetColor(mColor);
}



float JLBFont::GetStringWidth(const char *string) const
{
  float len = 0.0f;

  const char *p = string;
  char ch;

  while (*p)
    {
      ch = *p - 32;
      p++;
      if (ch < 0) continue;
      len += mCharWidth[ch+mBase] + mTracking;
    }
  len -= mTracking;
  return len*mScale;
}

void JLBFont::SetScale(float scale) { mScale = scale; }
void JLBFont::SetRotation(float rot) { mRotation = rot; }
void JLBFont::SetTracking(float tracking) { mTracking = tracking; }
PIXEL_TYPE JLBFont::GetColor() const { return mColor; }
float JLBFont::GetScale() const { return mScale; }
float JLBFont::GetRotation() const { return mRotation; }
float JLBFont::GetTracking() const { return mTracking; }
float JLBFont::GetHeight() const { return mHeight; }

void JLBFont::SetBase(int base)
{
	mBase = (base==0)?0:128;
}
