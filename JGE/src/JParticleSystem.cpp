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



//-------------------------------------------------------------------------------------
JParticleSystem::JParticleSystem()
{

	mActive = false;
	
}


JParticleSystem::~JParticleSystem()
{
	mEffects.clear();
	

}


void JParticleSystem::ClearAll()
{
	mEffects.clear();
}


void JParticleSystem::Update(float dt)
{
	if (!mEffects.empty())
	{
		JParticleEffect* effect;
		std::list<JParticleEffect*>::iterator curr = mEffects.begin();
		while (curr != mEffects.end())
		{
			effect = *curr;
			effect->Update(dt);
			if (effect->Done())
			{
				mEffects.erase(curr++);
			}
			else	
				++curr;
		}
	}
	
}


void JParticleSystem::Render()
{

	if (!mEffects.empty())
	{
		JParticleEffect* effect;
		std::list<JParticleEffect*>::iterator curr = mEffects.begin();
		while (curr != mEffects.end())
		{
			effect = *curr;
			effect->Render();
			
			++curr;
		}
	}
}




void JParticleSystem::StartEffect(JParticleEffect* effect)
{
	std::list<JParticleEffect*>::iterator curr = mEffects.begin();
	while (curr != mEffects.end())
	{
		if (effect == *curr && effect->GetParticleSystem() == this)
		{
			effect->Start();
			return;
		}
		++curr;
	}
	mEffects.push_back(effect);
	effect->SetParticleSystem(this);
	effect->Start();
}


void JParticleSystem::StopAllEffects()
{

	if (!mEffects.empty())
	{
		JParticleEffect* effect;
		std::list<JParticleEffect*>::iterator curr = mEffects.begin();
		while (curr != mEffects.end())
		{
			effect = *curr;
			effect->Stop();
			++curr;
		}
	}
	
	
}

bool JParticleSystem::IsActive() { return mActive; }

void JParticleSystem::SetActive(bool flag) { mActive = flag; }

