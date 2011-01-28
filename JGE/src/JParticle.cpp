//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------


//#include "JApp.h"
#include "../include/JGE.h"

#include "../include/JParticle.h"



JParticleData::JParticleData()
{
	Clear();
}

void JParticleData::Clear()
{
	mCurr = 0.0f;
	mTarget = 0.0f;
	mDelta = 0.0f;
	mKeyCount = 0;
	mKeyIndex = 0;
	mTimer = 0.0f;
	mScale = 1.0f;
}

void JParticleData::Init()
{
	mKeyIndex = 0;
	mDelta = 0.0f;
	mTimer = 0.0f;
	if (mKeyCount > 0)
	{
		mCurr = mKeyValue[mKeyIndex++];

		if (mKeyIndex < mKeyCount)
		{
			mTimer = mKeyTime[mKeyIndex]*mScale-mKeyTime[mKeyIndex-1]*mScale;
			mTarget = mKeyValue[mKeyIndex];
			mDelta = (mTarget-mCurr)/mTimer;
			mKeyIndex++;
		}
	}
}

void JParticleData::AddKey(float keyTime, float keyValue)
{
	if (mKeyCount < MAX_KEYS)
	{
		mKeyTime[mKeyCount] = keyTime;
		mKeyValue[mKeyCount] = keyValue;
		mKeyCount++;
	}
}

void JParticleData::Update(float dt)
{
	mCurr += mDelta*dt;
	if (mDelta < 0.0f)
	{
		if (mCurr < mTarget)
			mCurr = mTarget;

	} 
	else if (mDelta > 0.0f)
	{
		if (mCurr > mTarget)
			mCurr = mTarget;
	}

	mTimer -= dt;

	if (mTimer <= 0.0f)
	{
		if (mKeyIndex < mKeyCount)
		{
			mTimer = mKeyTime[mKeyIndex]*mScale-mKeyTime[mKeyIndex-1]*mScale;
			mTarget = mKeyValue[mKeyIndex];
			mDelta = (mTarget-mCurr)/mTimer;
			mKeyIndex++;
		}
		else
			mDelta = 0.0f;
	}
}

void JParticleData::SetScale(float scale)
{
	mScale = scale;
}


JRenderer* JParticle::mRenderer = NULL;

JParticleData* JParticle::GetField(int index)
{
	if (index < FIELD_COUNT)
		return &mData[index];
	else
		return NULL;
}


JParticleData* JParticle::GetDataPtr()
{
	return mData;
}

JParticle::JParticle()//JQuad* texture, float x, float y)
{
	mRenderer = JRenderer::GetInstance();
	
	//mTexture = texture;
	mOrigin = Vector2D(0.0f, 0.0f);
	mPos = Vector2D(0.0f, 0.0f);
	mVelocity = Vector2D(0.0f, 0.0f);
	
	mActive = false;
	mQuad = NULL;

//	mNext = NULL;
//	mPrev = NULL;
}
	

JParticle::~JParticle()
{
	//JGERelease();
}


bool JParticle::Update(float dt)
{
	
	for (int i=0;i<FIELD_COUNT;i++)
		mData[i].Update(dt);
	

	// the radial and tangential acceleration code was taken from HGE's particle source
	Vector2D vecAccel = mPos - mOrigin;					//par->vecLocation-vecLocation;
	vecAccel.Normalize();
	Vector2D vecAccel2 = vecAccel;
	vecAccel *= mData[FIELD_RADIAL_ACCEL].mCurr;		//par->fRadialAccel;

	// vecAccel2.Rotate(M_PI_2);
	// the following is faster
	float ang = vecAccel2.x;
	vecAccel2.x = -vecAccel2.y;
	vecAccel2.y = ang;

	vecAccel2 *= mData[FIELD_TANGENTIAL_ACCEL].mCurr;	//par->fTangentialAccel;
	mVelocity += (vecAccel+vecAccel2)*dt;				//par->vecVelocity += (vecAccel+vecAccel2)*fDeltaTime;
	mVelocity.y += mData[FIELD_GRAVITY].mCurr*dt;		//par->vecVelocity.y += par->fGravity*fDeltaTime;

	//par->vecLocation += par->vecVelocity*fDeltaTime;

	////mPos.x += mData[FIELD_SPEED].mCurr * mVelocity.x * dt;
	////mPos.y += mData[FIELD_SPEED].mCurr * mVelocity.y * dt;
	mPos += mVelocity*mData[FIELD_SPEED].mCurr*dt;


	mLifetime -= dt;
	if (mLifetime <= 0.0f)
	{
		mActive = false;
		return false;
	}
	
		
	return true;
}


void JParticle::Render()
{
	if (mQuad)
	{
		int a = (int)(mData[FIELD_ALPHA].mCurr*255.0f);
		int r = (int)(mData[FIELD_RED].mCurr*255.0f);
		int g = (int)(mData[FIELD_GREEN].mCurr*255.0f);
		int b = (int)(mData[FIELD_BLUE].mCurr*255.0f);
		PIXEL_TYPE color = ARGB(a, r, g, b);
		mQuad->SetColor(color);
		
		mRenderer->RenderQuad(mQuad, mPos.x, mPos.y, mData[FIELD_ROTATION].mCurr, mData[FIELD_SIZE].mCurr*mSize, mData[FIELD_SIZE].mCurr*mSize);
		
	}
}


void JParticle::Init(float lifeTime)
{
	mLifetime = lifeTime;

	for (int i=0;i<FIELD_COUNT;i++)
	{
		mData[i].SetScale(lifeTime);
		mData[i].Init();
	}

	mActive = true;
}


void JParticle::InitPosition(float ox, float oy, float xoffset, float yoffset)
{ 
	mOrigin = Vector2D(ox, oy);
	mPos = Vector2D(ox+xoffset, oy+yoffset);
}


void JParticle::SetPosition(float x, float y) 
{ 
	mPos = Vector2D(x, y); 
	mOrigin = Vector2D(x, y);
}

void JParticle::SetQuad(JQuad *quad) { mQuad = quad; }

void JParticle::Move(float x, float y)
{
	mPos.x += x;
	mPos.y += y;

	mOrigin.x += x;
	mOrigin.y += y;
}

//void JParticle::ResetVelocity()
//{
//	float xx =  mSpeed * cosf(mAngle);
//	float yy =  mSpeed * sinf(mAngle);
//	mVelocity = Vector2D(xx, yy);
//}

void JParticle::SetVelocity(float x, float y) { mVelocity = Vector2D(x, y); }
void JParticle::SetSize(float size) { mSize = size;}

