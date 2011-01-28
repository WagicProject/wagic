//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------


#include "../include/JSprite.h"

JRenderer* JSprite::mRenderer = NULL;

//------------------------------------------------------------------------------------------------
JSprite::JSprite(JTexture *tex, float x, float y, float width, float height, bool flipped)
{
	mRenderer = JRenderer::GetInstance();
	
	//mQuad = new JQuad(tex, x, y, width, height);
	//Rect *rect = new Rect((int)x, (int)y, (int)width, (int)height);
	//mFrames[0] = rect;
	mFrameCount = 0;
	mCurrentFrame = 0;
	mX = 0.0f;
	mY = 0.0f;
	mTimer = 0;
	
	mAnimationType = ANIMATION_TYPE_LOOPING;
	mDelta = 1;
	mAnimating = false;
	mDuration = 100.0f;
	mAlpha = 255.0f;
	mHScale = 1.0f;
	mVScale = 1.0f;
	mRotation = 0.0f;
	mDirection = 0.0f;
	mSpeed = 0.0f;
	
	mColor = ARGB(0,255,255,255);

	mFrames.clear();
	mFrames.reserve(8);

	mTex = tex;

	if (mTex != NULL)
	{
		mActive = true;
		AddFrame(x, y, width, height, flipped);
	}
	else
		mActive = false;
}


//------------------------------------------------------------------------------------------------
JSprite::~JSprite()
{
// 	for (int i=0;i<mFrameCount;i++)
// 	{
// 		if (mFrames[i])
// 			delete mFrames[i];
// 	}


	while (mFrames.size()>0)
	{
		JQuad* quad = mFrames.back();
		mFrames.pop_back();
		if (quad != NULL)
			delete quad;
	}

}


//------------------------------------------------------------------------------------------------
void JSprite::Update(float dt)
{
	if (!mAnimating) return;

	mTimer += dt;
	if (mTimer > mDuration)
	{
		mTimer = 0;
		mCurrentFrame+=mDelta;
		
		if  (mCurrentFrame >= mFrameCount)
		{
			if (mAnimationType == ANIMATION_TYPE_LOOPING)
				mCurrentFrame = 0;
			else if (mAnimationType == ANIMATION_TYPE_ONCE_AND_GONE)
				mActive = false;
			else if (mAnimationType == ANIMATION_TYPE_ONCE_AND_STAY)
			{
				mCurrentFrame = mFrameCount-1;
				mAnimating = false;
			}
			else if (mAnimationType == ANIMATION_TYPE_ONCE_AND_BACK)
			{
				mCurrentFrame = 0;
				mAnimating = false;
			}
			else	// ping pong
			{
				mDelta *= -1;
				mCurrentFrame += mDelta;
			}
		}
		else if (mCurrentFrame < 0)
		{
			if (mAnimationType == ANIMATION_TYPE_PINGPONG)
			{
				mDelta *= -1;
				mCurrentFrame += mDelta;
			}
		}
	}


	
}


//------------------------------------------------------------------------------------------------
void JSprite::SetAnimationType(int type)
{
	mAnimationType = type;
}


//------------------------------------------------------------------------------------------------
void JSprite::Render()
{
	if (mActive)
	{
		//mQuad->SetTextureRect((float)mFrames[mCurrentFrame]->x, (float)mFrames[mCurrentFrame]->y, 
		//					   (float)mFrames[mCurrentFrame]->width, (float)mFrames[mCurrentFrame]->height);
		//if (mDoAlpha)

//		{
			int alpha = (int)mAlpha;
//			//mQuad->SetColor(ARGB(alpha,255,255,255));
	

#if defined (WIN32) || defined (LINUX) || defined (IOS)
			mFrames[mCurrentFrame]->SetColor(alpha<<24 | mColor);
//			mQuad->SetFlip(mFlipped[mCurrentFrame], false);
#else
			mFrames[mCurrentFrame]->SetColor(MAKE_COLOR(alpha, mColor));
#endif
//			mQuad->SetHFlip(mFlipped[mCurrentFrame]);
//		}
		//if (mDoRotation != 0.0f || mHScale != 1.0f || mVScale != 1.0f)
			mRenderer->RenderQuad(mFrames[mCurrentFrame], mX, mY, mRotation, mHScale, mVScale);
		//else
		//	mRenderer->RenderQuad(mFrames[mCurrentFrame], mX, mY);
	}
}


//------------------------------------------------------------------------------------------------
void JSprite::AddFrame(float x, float y, float width, float height, bool flipped)
{ 
	//mFlipped[mFrameCount] = flipped;
	//mFrames[mFrameCount++] = new JQuad(mTex, x, y, width, height); 

	JQuad* quad = new JQuad(mTex, x, y, width, height);
	if (quad != NULL)
	{
		quad->SetHFlip(flipped);
		mFrames.push_back(quad);

		mFrameCount = mFrames.size();
	}
}

//-------------------------------------------------------------------------------
void JSprite::AddFrame(JTexture* tex, float x, float y, float width, float height, bool flipped)
{
	mTex = tex;
	AddFrame(x, y, width, height, flipped);
}



//------------------------------------------------------------------------------------------------

void JSprite::SetDuration(float duration) 
{ 
	mDuration = duration; 
}


//------------------------------------------------------------------------------------------------
void JSprite::SetPosition(float x, float y) 
{ 
	mX = x; 
	mY = y; 

}



void JSprite::RestartAnimation()
{
	mAlpha = 255.0f;
	mHScale = 1.0f;
	mVScale = 1.0f;
	mRotation = 0.0f;
	
	mCurrentFrame = 0;
	StartAnimation();
}


void JSprite::SetDirection(float x, float y)
{
	float dx = x - mX;
	float dy = y - mY;

	//from atan2:
	//	    -pi/2
	//		   |
	//		   |
	// pi  ----+----  0
	//		   |
	//		   |
	//	      pi/2          

	SetDirection(atan2f(dy, dx));

}


void JSprite::SetFlip(bool flip, int index)
{
	int size = (int)mFrames.size();
	if (index == -1)
	{
		for (int i=0;i<size;i++)
			if (mFrames[i] != NULL)
				mFrames[i]->SetHFlip(flip);
	}
	else
	{
		if (index >= 0 && index < size)
			mFrames[index]->SetHFlip(flip);
	}

}


// void JSprite::SetBBox(int x, int y, int width, int height)
// { 
// 	mBBoxX = x; 
// 	mBBoxY = y; 
// 	mBBoxWidth = width; 
// 	mBBoxHeight = height; 
// }



void JSprite::StartAnimation() 
{ 
	mActive = true;
	mAnimating = true; 
	mCurrentFrame = 0;
}

void JSprite::StopAnimation() { mAnimating = false; }
bool JSprite::IsAnimating() { return mAnimating; }

void JSprite::SetAlpha(float alpha) { mAlpha = alpha; }
void JSprite::SetScale(float hscale, float vscale) { mHScale = hscale; mVScale = vscale; }
void JSprite::SetScale(float scale) { mHScale = scale; mVScale = scale; }
float JSprite::GetScale() { return mHScale; }
void JSprite::SetRotation(float rot) { mRotation = rot; }

//JQuad* JSprite::GetQuad() { return mQuad; }

float JSprite::GetXVelocity() { return cosf(mDirection)*mSpeed; }
float JSprite::GetYVelocity() { return sinf(mDirection)*mSpeed; }

void JSprite::SetSpeed(float speed) { mSpeed = speed; }
float JSprite::GetSpeed() { return mSpeed; }
void JSprite::SetDirection(float angle) { mDirection = angle; }

void JSprite::SetHotSpot(float x, float y, int index)
{ 
	int size = (int)mFrames.size();

	if (index == -1)
	{
		for (int i=0;i<size;i++)
			if (mFrames[i] != NULL)
				mFrames[i]->SetHotSpot(x, y); 
	}
	else
	{
		if (index >= 0 && index < size)
			mFrames[index]->SetHotSpot(x, y); 
	}

}

#if defined (WIN32) || defined (LINUX) || defined (IOS)
void JSprite::SetColor(PIXEL_TYPE color) { mColor = (color&0x00ffffff); }
#else
void JSprite::SetColor(PIXEL_TYPE color) { mColor = (color&~MASK_ALPHA); }
#endif

// void JSprite::SetBlendMode(int blend, int index) 
// { 
// 	if (index == -1)
// 	{
// 		for (int i=0;i<MAX_FRAMES;i++)
// 			if (mFrames[i] != NULL)
// 				mFrames[i]->mBlend = blend;
// 	}
// 	else
// 	{
// 		if (index >= 0 && index < MAX_FRAMES)
// 			mFrames[index]->mBlend = blend;
// 	}
// 
// }

float JSprite::GetAlpha() { return mAlpha; }
float JSprite::GetDirection() { return mDirection; }
float JSprite::GetRotation() { return mRotation; }

void JSprite::SetId(int id) { mId = id; }
int JSprite::GetId() { return mId; }

int JSprite::GetCurrentFrameIndex() { return mCurrentFrame; }
void JSprite::SetCurrentFrameIndex(int frame)
{ 
	if (frame < (int)mFrames.size())
		mCurrentFrame = frame; 
}


JQuad* JSprite::GetCurrentFrame() 
{ 
        if (mCurrentFrame < (signed int)mFrames.size())
		return mFrames[mCurrentFrame]; 
	else
		return NULL;
}

int JSprite::GetFrameCount()
{
	return (int) mFrames.size();
}


JQuad* JSprite::GetFrame(int index) 
{ 
        if (index < (signed int)mFrames.size())
		return mFrames[index]; 
	else
		return NULL;
}


float JSprite::GetX() { return mX; }
float JSprite::GetY() { return mY; }

void JSprite::SetX(float x) { mX = x; }
void JSprite::SetY(float y) { mY = y; }

void JSprite::SetActive(bool f)
{ 
	mActive = f; 
	if (mActive)
	{
		if (mFrames.size()==0)
			mActive = false;
	}
}

bool JSprite::IsActive() { return mActive; }

void JSprite::Move(float dx, float dy)
{
	mX += dx;
	mY += dy;
}


// ------------------------------------------------------------------------------------------------
JSpriteList::JSpriteList(int count)
{
	
	mCount = count;
	mList = new JSprite*[count];
	//mVictims = new JSpriteList*[count];

	for (int i=0;i<mCount;i++)
	{
		mList[i] = NULL;
		//mVictims[i] = NULL;
	}
}
 

JSpriteList::~JSpriteList()
{
	for (int i=0;i<mCount;i++)
		if (mList[i])
			delete mList[i];

	delete[] mList;
	//delete[] mVictims;

}
 

void JSpriteList::Update(float dt)
{
	for (int i=0;i<mCount;i++)
	{
		if (mList[i] && mList[i]->IsActive())
		{
			mList[i]->Update(dt);
//			if (mVictims[i]!= NULL)
//				mVictims[i]->CheckCollision(mList[i]);
		}
	}
}
 

void JSpriteList::Render()
{
	for (int i=0;i<mCount;i++)
		if (mList[i] && mList[i]->IsActive())
			mList[i]->Render();	
}


void JSpriteList::AddSprite(JSprite *sprite)//, JSpriteList* victim)
{
	for (int i=0;i<mCount;i++)
	{
		if (mList[i] == NULL)
		{
			mList[i] = sprite;
			//mVictims[i] = victim;
			return;
		}
	}
}



JSprite* JSpriteList::Activate(float x, float y)
{
	for (int i=0;i<mCount;i++)
	{
		if (mList[i] != NULL && !mList[i]->IsActive())
		{
			mList[i]->SetActive(true);
			mList[i]->SetPosition(x, y);
			mList[i]->RestartAnimation();
			
			return mList[i];
		}
	}

	return NULL;
}


void JSpriteList::Activate(float x, float y, int index)
{
	if (index < mCount)
	{
		if (mList[index] != NULL)
		{
			mList[index]->SetActive(true);
			mList[index]->SetPosition(x, y);
			mList[index]->RestartAnimation();
			
		}
	}
}

JSprite* JSpriteList::GetSprite(int index)
{
	if (index < mCount)
	{
		return mList[index];
	}

	return NULL;
}

void JSpriteList::EnableAll(bool flag)
{
	for (int i=0;i<mCount;i++)
	{
		if (mList[i] != NULL)
		{
			mList[i]->SetActive(flag);
			mList[i]->RestartAnimation();
		}
	}
}
