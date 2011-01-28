//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#include <math.h>

#include "../include/JGE.h"
#include "../include/JParticleSystem.h"
#include "../include/JParticleEffect.h"
#include "../include/JParticleEmitter.h"


//-------------------------------------------------------------------------------------
JParticleEmitter::JParticleEmitter(JParticleEffect* parent)
{
	mParent = parent;
	mType = TYPE_POINT;
	mSrcBlending = BLEND_SRC_ALPHA;
	mDestBlending = BLEND_ONE;

	mQuad = NULL;

	mQuadIndex = 0;
	mWidth = 8;
	mHeight = 8;

	mEmitTimer = 0.0f;
	mActive = false;

	mMaxParticleCount = MAX_PARTICLE_COUNT;

	mParticles.clear();
	mParticles.reserve(INIT_PARTICLE_COUNT);
}

JParticleEmitter::~JParticleEmitter()
{

	while (mParticles.size()>0)
	{
		JParticle* par = mParticles.back();
		mParticles.pop_back();
		delete par;
	}
}

JParticle* JParticleEmitter::GetIdleParticle()
{
	int size = mParticles.size();
	for (int i=0;i<size;i++)
	{
		if (!mParticles[i]->mActive)
			return mParticles[i];
	}

	if (size < mMaxParticleCount)
	{
		
		JParticle*par = new JParticle();
		if (par != NULL)
		{
			mParticles.push_back(par);
			return par;
		}
	}
	
	return NULL;
}

void JParticleEmitter::Start()
{
	mActive = true;
	mActiveParticleCount = 0;
	mRepeatCounter = mRepeatTimes;

	ReStart();
}


void JParticleEmitter::ReStart()
{
	mQuantity.Init();
	mEmitTimer = 0.0f;

// 	if (mQuad == NULL)
// 	{
// 		JParticleSystem* particleSys = mParent->GetParticleSystem();
// 		mQuad = particleSys->GetParticleQuad(mQuadIndex);
// 	}

	int count = (int) mQuantity.mCurr;
	if (count > 0)
		EmitParticles(count);
}


void JParticleEmitter::SetQuad(JQuad *quad)
{
	mQuad = quad;
}



void JParticleEmitter::Update(float dt)
{
  //	JParticleSystem* particleSys = mParent->GetParticleSystem();

	mActiveParticleCount = 0;

	if (!mParticles.empty())
	{
		int count = 0;
		int size = mParticles.size();
		for (int i=0;i<size;i++)
		{
			if (mParticles[i]->mActive)
			{
				count++;
				mParticles[i]->Update(dt);
			}
		}

		mActiveParticleCount = count;
	}

	if (!mActive) return;			// don't generate more

	mEmitTimer += dt;
	if (mEmitTimer > mLife)
	{
		mEmitTimer = 0.0f;
		
		if (mEmitterMode == MODE_ONCE)
		{
			mActive = false;
		}
		else if (mEmitterMode == MODE_REPEAT)
		{
			ReStart();
			return;
		}
		else if (mEmitterMode == MODE_NTIMES)
		{
			mRepeatCounter--;
			if (mRepeatCounter > 0)
			{
				ReStart();
				return;
			}
			else
				mActive = false;
		}
	}

	if (!mActive) return;			// don't generate more

	// more particles...
	int count = 0;

	if (mQuantity.mCurr != 0.0f)
	{
		
		float timeForOneParticle = 1.0f/mQuantity.mCurr;

		float potentialParticles = (float) ((int)(mEmitTimer/timeForOneParticle));
		if (potentialParticles >= 1.0f)
			mEmitTimer -= (potentialParticles*timeForOneParticle);

		count = (int)potentialParticles;
	}

	//int count = (int)(mQuantity.mCurr * dt);		// number of particles for this dt
	
	mQuantity.Update(dt);

	EmitParticles(count);
}


void JParticleEmitter::EmitParticles(int count)
{
  //	JParticleSystem* particleSys = mParent->GetParticleSystem();

	JParticleData *dataPtr;
	JParticle* particle;

	float x, y;

	float xOrigin = mParent->GetX();
	float yOrigin = mParent->GetY();

	for (int i=0;i<count;i++)
	{
		particle = GetIdleParticle();
		if (particle != NULL)
		{
			particle->SetQuad(mQuad);

			dataPtr = particle->GetDataPtr();
			memcpy(dataPtr, mData, sizeof(JParticleData)*FIELD_COUNT);

			float angle = 0.0f;

			switch (mType)
			{
			case TYPE_POINT:
				//particle->SetPosition(mParent->GetX(),mParent->GetY());
				particle->InitPosition(xOrigin, yOrigin, 0, 0);
				break;
			case TYPE_AREA:
				x = (float)((rand()%mWidth) - (mWidth>>1));
				y = (float)((rand()%mHeight) - (mHeight>>1));
				//particle->SetPosition(mParent->GetX()+x,mParent->GetY()+y);
				particle->InitPosition(xOrigin, yOrigin, x, y);
				break;
			case TYPE_HORIZONTAL:
				x = (float)((rand()%mWidth) - (mWidth>>1));
				//particle->SetPosition(mParent->GetX()+x,mParent->GetY());
				particle->InitPosition(xOrigin, yOrigin, x, 0);
				break;
			case TYPE_VERTICAL:
				y = (float)((rand()%mHeight) - (mHeight>>1));
				//particle->SetPosition(mParent->GetX(),mParent->GetY()+y);
				particle->InitPosition(xOrigin, yOrigin, 0, y);
				break;
			case TYPE_CIRCLE:
				angle = M_PI* 2 * (rand()%1001)/1000.0f;
				x = cosf(angle)*mWidth;
				y = sinf(angle)*mHeight;
				particle->InitPosition(xOrigin, yOrigin, x, y);
				break;

			}

			//particle->mSpeed 
			float speed = mSpeedBase + (mSpeedMax-mSpeedBase) * (rand()%1001)/1000.0f;

			if (mType != TYPE_CIRCLE)
				angle = mAngleBase + (mAngleMax-mAngleBase) * (rand()%1001)/1000.0f;
			//float x = cosf(angle);
			//float y = sinf(angle);
			// speed itself is not changing
			particle->SetVelocity(cosf(angle)*speed, sinf(angle)*speed);

			particle->SetSize(mSizeBase + (mSizeMax-mSizeBase) * (rand()%1001)/1000.0f);
			
			float life = mLifeBase + (mLifeMax-mLifeBase) * (rand()%1001)/1000.0f;
			particle->Init(life);

			
		}
		
	}

		
}


void JParticleEmitter::Render()
{
	JRenderer* renderer = JRenderer::GetInstance();

	renderer->SetTexBlend(mSrcBlending, mDestBlending);

	int size = mParticles.size();
	for (int i=0;i<size;i++)
	{
		if (mParticles[i]->mActive)
		{
			mParticles[i]->Render();
		}
	}

	renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);


}


void JParticleEmitter::SetBlending(int srcBlend, int destBlend)
{
	mSrcBlending = srcBlend;
	mDestBlending = destBlend;
}


bool JParticleEmitter::Done()
{
	return !mActive && mActiveParticleCount==0;
}


void JParticleEmitter::SetActive(bool flag)
{
	mActive = flag;
}


void JParticleEmitter::MoveAllParticles(float x, float y)
{
	int size = mParticles.size();
	for (int i=0;i<size;i++)
	{
		if (mParticles[i]->mActive)
		{
			mParticles[i]->Move(x, y);
		}
	}


}

void JParticleEmitter::SetMaxParticleCount(int count)
{
	mMaxParticleCount = count;
}

