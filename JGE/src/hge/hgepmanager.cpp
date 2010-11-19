/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeParticleManager helper class implementation
*/


#include "../../include/hge/hgeparticle.h"


hgeParticleManager::hgeParticleManager()
{
	nPS=0;
	tX=tY=0.0f;
}

hgeParticleManager::~hgeParticleManager()
{
	int i;
	for(i=0;i<nPS;i++) delete psList[i];
}

void hgeParticleManager::Update(float dt)
{
	int i;
	for(i=0;i<nPS;i++)
	{
		psList[i]->Update(dt);
		if(psList[i]->GetAge()==-2.0f && psList[i]->GetParticlesAlive()==0)
		{
			delete psList[i];
			psList[i]=psList[nPS-1];
			nPS--;
			i--;
		}
	}
}

void hgeParticleManager::Render()
{
	int i;
	for(i=0;i<nPS;i++) psList[i]->Render();
}

hgeParticleSystem* hgeParticleManager::SpawnPS(hgeParticleSystemInfo *psi, float x, float y)
{
	if(nPS==MAX_PSYSTEMS) return 0;
	psList[nPS]=new hgeParticleSystem(psi);
	psList[nPS]->FireAt(x,y);
	psList[nPS]->Transpose(tX,tY);
	nPS++;
	return psList[nPS-1];
}

bool hgeParticleManager::IsPSAlive(hgeParticleSystem *ps) const
{
	int i;
	for(i=0;i<nPS;i++) if(psList[i]==ps) return true;
	return false;
}

void hgeParticleManager::Transpose(float x, float y)
{
	int i;
	for(i=0;i<nPS;i++) psList[i]->Transpose(x,y);
	tX=x; tY=y;
}

void hgeParticleManager::KillPS(hgeParticleSystem *ps)
{
	int i;
	for(i=0;i<nPS;i++)
	{
		if(psList[i]==ps)
		{
			delete psList[i];
			psList[i]=psList[nPS-1];
			nPS--;
			return;
		}
	}
}

void hgeParticleManager::KillAll()
{
	int i;
	for(i=0;i<nPS;i++) delete psList[i];
	nPS=0;
}
