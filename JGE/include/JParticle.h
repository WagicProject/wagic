//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "JApp.h"
#include "JRenderer.h"
#include "Vector2D.h"


#define MAX_KEYS	8

class JParticleData
{
public:
	JParticleData();
	void Init();
	void Clear();
	void AddKey(float keyTime, float keyValue);
	void Update(float dt);
	void SetScale(float scale);

	float mCurr;
	float mTarget;
	float mDelta;
	float mTimer;
	int mKeyCount;
	int mKeyIndex;
	float mKeyTime[MAX_KEYS];
	float mKeyValue[MAX_KEYS];

	float mScale;		
};


enum ParticleField
{
	FIELD_SPEED,
	FIELD_SIZE,
	FIELD_ROTATION,
	FIELD_ALPHA,
	FIELD_RED,
	FIELD_GREEN,
	FIELD_BLUE,
	FIELD_RADIAL_ACCEL,
	FIELD_TANGENTIAL_ACCEL,
	FIELD_GRAVITY,
	FIELD_COUNT
};

class JParticle
{
public:

	bool mActive;

	JParticle();
	~JParticle();

	bool Update(float dt);
	void Render();
	void Init(float lifeTime);

	void InitPosition(float ox, float oy, float xoffset, float yoffset);
	void SetPosition(float x, float y);
	void SetQuad(JQuad *quad);

	JParticleData* GetField(int index);
	JParticleData* GetDataPtr();

	void Move(float x, float y);
	void SetVelocity(float x, float y);
	void SetSize(float size);


private:
	static JRenderer*	mRenderer;
	JQuad* mQuad;

	Vector2D mOrigin;
	Vector2D mPos;
	Vector2D mVelocity;

	//float mSpeed;
	float mSize;

	float mLifetime;

	JParticleData mData[FIELD_COUNT];




};


#endif
