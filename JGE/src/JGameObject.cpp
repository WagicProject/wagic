//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------


#include "../include/JGE.h"
#include "../include/JApp.h"

#include "../include/JGameObject.h"


//JRenderer* JGameObject::mRenderer = NULL;



JGameObject::JGameObject(JTexture *tex, float x, float y, float width, float height)
	:JSprite(tex, x, y, width, height)
{
	//mRenderer = JRenderer::GetInstance();

	//mX = 0.0f;
	//mY = 0.0f;

	mRenderFlags = 0;
	//mSize = 1.0f;
	//mAngle = 0.0f;
	
	mOriginalBlood = 1;
	mBlood = 1;
	mHitPoint = 1;

	mCollided = false;
	mCollisionTarget = NULL;
	mFlashing = false;

	//mActive = false;

	mRotationDelta = 0.0f;
	mDoRotation = false;

	mAlphaDelta = 0.0f;
	mDoAlpha = false;

	mDoScaling = false;
	mScaleDelta = 0.0f;

	SetBBox(x, y, width, height);

}
	

JGameObject::~JGameObject()
{
//	JGERelease();
}


void JGameObject::Update(float dt)
{
	JSprite::Update(dt);

	if (mFlashing)
	{
		mFlashTimer += dt;
		if (mFlashTimer > FLASH_TIME)
		{
			mFlashTimer = 0;
			mFlashCounter++;
			if (mFlashCounter > FLASHING_COUNT)
				mFlashing = false;
		}
	}

	if (mDoAlpha)
	{
		mAlpha += mAlphaDelta*dt;
		if (mAlpha < 0.0f)
		{
			mAlpha = 0.0f;
			if (mAnimationType == ANIMATION_TYPE_ONCE_AND_GONE)
				mActive = false;
		}
		else if (mAlpha > 255.0f)
		{
			mAlpha = 255.0f;
		}
		
	}
	
	if (mDoRotation || mDoScaling)
	{
		mRotation += mRotationDelta*dt;
		mHScale += mScaleDelta*dt;
		mVScale += mScaleDelta*dt;
	}

}


void JGameObject::Render()
{
	/*
	if (mQuad != NULL)
	{
		//mEngine->RenderQuad(mQuad, mX, mY);
		if (mFlashing && (mFlashCounter&1)==0)
		{
			mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		}

		float angle = 0.0f;
		if ((mRenderFlags & RENDER_FLAG_ANGLE)==RENDER_FLAG_ANGLE)
			angle = mAngle;
		if ((mRenderFlags & RENDER_FLAG_ROTATION)==RENDER_FLAG_ROTATION)
			angle = mRotation;
		float scale = 1.0f;
		if ((mRenderFlags & RENDER_FLAG_SIZE)==RENDER_FLAG_SIZE)
			scale = mSize;
	
		mRenderer->RenderQuad(mQuad, mX, mY, angle, scale, scale);
		
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	}
	*/

	if (!mActive) return;

	if ((mRenderFlags & RENDER_FLAG_ANGLE)==RENDER_FLAG_ANGLE)
		mRotation = mDirection;

	if (mFlashing && (mFlashCounter&1)==0)
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);

// 	if ((mRenderFlags & RENDER_FLAG_SIZE)==RENDER_FLAG_SIZE)
// 	{
// 		mHScale = mSize;
// 		mVScale = mSize;
// 	}
// 	else
// 	{
// 		mHScale = 1.0f;
// 		mVScale = 1.0f;
// 	}

	JSprite::Render();

	if (mFlashing)
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

}


// void JGameObject::SetPosition(float x, float y) 
// { 
// //	mPos = Vector2D(x, y); 
// 	mX = x;
// 	mY = y;
// }


//void JGameObject::SetQuad(JQuad *quad) { mQuad = quad; }




//void JParticle::ResetVelocity()
//{
//	float xx =  mSpeed * cosf(mAngle);
//	float yy =  mSpeed * sinf(mAngle);
//	mVelocity = Vector2D(xx, yy);
//}


void JGameObject::SetBBox(float x, float y, float width, float height)
{
	mUseBoundingBox = true;

	mBBoxX = x;
	mBBoxY = y;
	mBBoxWidth = width;
	mBBoxHeight = height;
}

void JGameObject::GetBBox(float x, float y, float* xNow, float* yNow, float* width, float *height)
{
	*xNow = x + mBBoxX;
	*yNow = y + mBBoxY;
	*width = mBBoxWidth;
	*height = mBBoxHeight;
}


bool JGameObject::Collide(JGameObject *target)
{
	if (mUseBoundingBox)
	{
		// bounding box collision detection
		if ((target->mX+target->mBBoxX)-(mX+mBBoxX) < -target->mBBoxWidth) return false;
		if ((target->mX+target->mBBoxX)-(mX+mBBoxX) > mBBoxWidth) return false;
		if ((target->mY+target->mBBoxY)-(mY+mBBoxY) < -target->mBBoxHeight) return false;
		if ((target->mY+target->mBBoxY)-(mY+mBBoxY) > mBBoxHeight) return false;
	
	}
	else
	{
		// Circle-Circle collision detection
		float dx = (mX+mCenterX)-(target->mX+target->mCenterX);
		float dy = (mY+mCenterY)-(target->mY+target->mCenterY);
		float dr = mRadius+target->mRadius;
		if (dx*dx + dy*dy > dr*dr)
			return false;
	}

	//mCollided = true;
	//mCollisionTarget = target;
	SetCollisionTarget(target);
	target->SetCollisionTarget(this);

	return true;	// collision!!!
}


void JGameObject::SetCollisionTarget(JGameObject *target)
{
	mBlood -= target->GetHitPoint();
	if (mBlood < 0)
	{
//		mActive = false;
		mBlood = 0;
	}
	
	mCollided = true;
	mCollisionTarget = target;
}



int JGameObject::GetHitPoint()
{
	return mHitPoint;
}


void JGameObject::SetHitPoint(int pt)
{
	mHitPoint = pt;
}


void JGameObject::SetBlood(int pt)
{
	mOriginalBlood = pt;
	mBlood = pt;
}

int JGameObject::GetBlood()
{
	return mBlood;
}


void JGameObject::OnCollide()
{
	
}


void JGameObject::StartFlashing()
{
	mFlashing = true;
	mFlashTimer = 0.0f;
	mFlashCounter = 0;
}


void JGameObject::StopFlashing()
{
	mFlashing = false;
}


bool JGameObject::IsFlashing()
{
	return mFlashing;
}

void JGameObject::SetRenderFlags(int flags)
{
	mRenderFlags = flags;
}

// void JGameObject::SetSize(float size)
// { 
// 	mSize = size;	
// }
// 
// 
// void JGameObject::SetAngle(float angle)
// {
// 	mAngle = angle;
// }

void JGameObject::EnableAlpha(bool flag, float delta) { mDoAlpha = flag; mAlphaDelta = delta; }
void JGameObject::EnableScaling(bool flag, float delta) { mDoScaling = flag; mScaleDelta = delta; }
void JGameObject::EnableRotation(bool flag, float delta) { mDoRotation = flag; mRotationDelta = delta; }



void JGameObject::SetCollisionCircle(float cx, float cy, float radius)
{
	mUseBoundingBox = false;

	mCenterX = cx;
	mCenterY = cy;
	mRadius = radius;
}

JGameObject *JGameObject::GetCollisionTarget()
{
	return mCollisionTarget;
}

// void JGameObject::SetSize(float size)
// {
// 	mSize = size;
// 	//mHScale = size;
// 	//mVScale = size;
// }
