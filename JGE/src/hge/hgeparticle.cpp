/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeParticleSystem helper class implementation
*/

#include "../../include/JGE.h"
#include "../../include/JTypes.h"
#include "../../include/JRenderer.h"
#include "../../include/JFileSystem.h"

#include "../../include/hge/hgeparticle.h"


//HGE	*hgeParticleSystem::hge=0;

/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** Core functions implementation: random number generation
*/


unsigned int g_seed=0;

void Random_Seed(int seed)
{
  if(!seed) g_seed=JGE::GetInstance()->GetTime();
  else g_seed=seed;
}

int Random_Int(int min, int max)
{
  g_seed=214013*g_seed+2531011;
  return min+(g_seed ^ g_seed>>15)%(max-min+1);
}

float Random_Float(float min, float max)
{
  g_seed=214013*g_seed+2531011;
  return min+(g_seed>>16)*(1.0f/65535.0f)*(max-min);
}




hgeParticleSystem::hgeParticleSystem(const char *filename, JQuad *sprite)
{
  //void *psi;
  //hgeParticleSystemInfo psi;

  JFileSystem* fileSys = JFileSystem::GetInstance();
  //hge=hgeCreate(HGE_VERSION);

  //psi=hge->Resource_Load(filename);
  if (!fileSys->OpenFile(filename)) return;

  //if(!psi) return;

  //memcpy(&info, psi, sizeof(hgeParticleSystemInfo));
  //hge->Resource_Free(psi);

  // Skip reading the pointer as it may be larger than 4 bytes in the structure
  void *dummyPointer;
  fileSys->ReadFile(&dummyPointer, 4);
  // we're actually trying to read more than the file size now, but it's no problem.
  // Note that this fix is only to avoid the largest problems, filling a structure
  // by directly reading a file, is really a bad idea ...
  fileSys->ReadFile(&(info.nEmission), sizeof(hgeParticleSystemInfo));
  fileSys->CloseFile();

  info.sprite=sprite;
  //  	info.fGravityMin *= 100;
  //  	info.fGravityMax *= 100;
  // 	info.fSpeedMin *= 100;
  // 	info.fSpeedMax *= 100;

  vecLocation.x=vecPrevLocation.x=0.0f;
  vecLocation.y=vecPrevLocation.y=0.0f;
  fTx=fTy=0;

  fEmissionResidue=0.0f;
  nParticlesAlive=0;
  fAge=-2.0;
  mTimer = 0.0f;

  rectBoundingBox.Clear();
  bUpdateBoundingBox=false;
}

hgeParticleSystem::hgeParticleSystem(hgeParticleSystemInfo *psi)
{
  //hge=hgeCreate(HGE_VERSION);

  memcpy(&info, psi, sizeof(hgeParticleSystemInfo));

  vecLocation.x=vecPrevLocation.x=0.0f;
  vecLocation.y=vecPrevLocation.y=0.0f;
  fTx=fTy=0;

  fEmissionResidue=0.0f;
  nParticlesAlive=0;
  fAge=-2.0;
  mTimer = 0.0f;

  rectBoundingBox.Clear();
  bUpdateBoundingBox=false;
}

hgeParticleSystem::hgeParticleSystem(const hgeParticleSystem &ps)
{
  memcpy(this, &ps, sizeof(hgeParticleSystem));
  //hge=hgeCreate(HGE_VERSION);
}

void hgeParticleSystem::Update(float fDeltaTime)
{
  int i;
  float ang;
  hgeVector vecAccel, vecAccel2;

  if(fAge >= 0)
  {
    fAge += fDeltaTime;
    if(fAge >= info.fLifetime) fAge = -2.0f;
  }

  mTimer += fDeltaTime;
  if (mTimer < 0.01f)
    return;

  fDeltaTime = mTimer;
  mTimer = 0.0f;


  // update all alive particles

  if(bUpdateBoundingBox) rectBoundingBox.Clear();

  ParticleBuffer::iterator particle = mParticleBuffer.begin();
  while(particle != mParticleBuffer.end())
  {
    particle->fAge += fDeltaTime;
    if(particle->fAge >= particle->fTerminalAge)
    {
      nParticlesAlive--;
      ++particle;
      mParticleBuffer.pop_front();
      continue;
    }

    vecAccel = particle->vecLocation-vecLocation;
    vecAccel.Normalize();
    vecAccel2 = vecAccel;
    vecAccel *= particle->fRadialAccel;

    // vecAccel2.Rotate(M_PI_2);
    // the following is faster
    ang = vecAccel2.x;
    vecAccel2.x = -vecAccel2.y;
    vecAccel2.y = ang;

    vecAccel2 *= particle->fTangentialAccel;
    particle->vecVelocity += (vecAccel+vecAccel2)*fDeltaTime;
    particle->vecVelocity.y += particle->fGravity*fDeltaTime;

    //par->vecVelocity.y = 0.1f;
    particle->vecLocation += particle->vecVelocity;

    particle->fSpin += particle->fSpinDelta*fDeltaTime;
    particle->fSize += particle->fSizeDelta*fDeltaTime;
    particle->colColor += particle->colColorDelta*fDeltaTime;

    if(bUpdateBoundingBox) rectBoundingBox.Encapsulate(particle->vecLocation.x, particle->vecLocation.y);

    ++particle;
  }

  // generate new particles

  if(fAge != -2.0f)
  {
    float fParticlesNeeded = info.nEmission*fDeltaTime + fEmissionResidue;
    int nParticlesCreated = (unsigned int)fParticlesNeeded;
    fEmissionResidue=fParticlesNeeded-nParticlesCreated;

    for(i=0; i<nParticlesCreated; i++)
    {
      if(nParticlesAlive>=MAX_PARTICLES) break;

      hgeParticle newParticle;
      newParticle.fAge = 0.0f;
      newParticle.fTerminalAge = Random_Float(info.fParticleLifeMin, info.fParticleLifeMax);

      newParticle.vecLocation = vecPrevLocation+(vecLocation-vecPrevLocation)*Random_Float(0.0f, 1.0f);
      newParticle.vecLocation.x += Random_Float(-2.0f, 2.0f);
      newParticle.vecLocation.y += Random_Float(-2.0f, 2.0f);

      ang=info.fDirection-M_PI_2+Random_Float(0,info.fSpread)-info.fSpread/2.0f;
      if(info.bRelative) ang += (vecPrevLocation-vecLocation).Angle()+M_PI_2;
      newParticle.vecVelocity.x = cosf(ang);
      newParticle.vecVelocity.y = sinf(ang);
      newParticle.vecVelocity *= Random_Float(info.fSpeedMin, info.fSpeedMax);

      newParticle.fGravity = Random_Float(info.fGravityMin, info.fGravityMax);
      newParticle.fRadialAccel = Random_Float(info.fRadialAccelMin, info.fRadialAccelMax);
      newParticle.fTangentialAccel = Random_Float(info.fTangentialAccelMin, info.fTangentialAccelMax);

      newParticle.fSize = Random_Float(info.fSizeStart, info.fSizeStart+(info.fSizeEnd-info.fSizeStart)*info.fSizeVar);
      newParticle.fSizeDelta = (info.fSizeEnd-newParticle.fSize) / newParticle.fTerminalAge;

      newParticle.fSpin = Random_Float(info.fSpinStart, info.fSpinStart+(info.fSpinEnd-info.fSpinStart)*info.fSpinVar);
      newParticle.fSpinDelta = (info.fSpinEnd-newParticle.fSpin) / newParticle.fTerminalAge;

      newParticle.colColor.r = Random_Float(info.colColorStart.r, info.colColorStart.r+(info.colColorEnd.r-info.colColorStart.r)*info.fColorVar);
      newParticle.colColor.g = Random_Float(info.colColorStart.g, info.colColorStart.g+(info.colColorEnd.g-info.colColorStart.g)*info.fColorVar);
      newParticle.colColor.b = Random_Float(info.colColorStart.b, info.colColorStart.b+(info.colColorEnd.b-info.colColorStart.b)*info.fColorVar);
      newParticle.colColor.a = Random_Float(info.colColorStart.a, info.colColorStart.a+(info.colColorEnd.a-info.colColorStart.a)*info.fAlphaVar);

      newParticle.colColorDelta.r = (info.colColorEnd.r-newParticle.colColor.r) / newParticle.fTerminalAge;
      newParticle.colColorDelta.g = (info.colColorEnd.g-newParticle.colColor.g) / newParticle.fTerminalAge;
      newParticle.colColorDelta.b = (info.colColorEnd.b-newParticle.colColor.b) / newParticle.fTerminalAge;
      newParticle.colColorDelta.a = (info.colColorEnd.a-newParticle.colColor.a) / newParticle.fTerminalAge;

      if(bUpdateBoundingBox) rectBoundingBox.Encapsulate(newParticle.vecLocation.x, newParticle.vecLocation.y);

      mParticleBuffer.push_back(newParticle);
      ++nParticlesAlive;
    }
  }

  vecPrevLocation=vecLocation;
}

void hgeParticleSystem::MoveTo(float x, float y, bool bMoveParticles)
{
  float dx,dy;

  if(bMoveParticles)
  {
    dx=x-vecLocation.x;
    dy=y-vecLocation.y;

    ParticleBuffer::iterator particle = mParticleBuffer.begin();
    for (; particle != mParticleBuffer.end(); ++particle)
    {
      particle->vecLocation.x += dx;
      particle->vecLocation.y += dy;
    }

    vecPrevLocation.x=vecPrevLocation.x + dx;
    vecPrevLocation.y=vecPrevLocation.y + dy;
  }
  else
  {
    if(fAge==-2.0) { vecPrevLocation.x=x; vecPrevLocation.y=y; }
    else { vecPrevLocation.x=vecLocation.x;	vecPrevLocation.y=vecLocation.y; }
  }

  vecLocation.x=x;
  vecLocation.y=y;
}

void hgeParticleSystem::FireAt(float x, float y)
{
  Stop();
  MoveTo(x,y);
  Fire();
}

void hgeParticleSystem::Fire()
{
  mTimer = 0.0f;

  if(info.fLifetime==-1.0f) fAge=-1.0f;
  else fAge=0.0f;
}

void hgeParticleSystem::Stop(bool bKillParticles)
{
  fAge=-2.0f;
  if(bKillParticles) 
  {
    nParticlesAlive=0;
    mParticleBuffer.clear();
    rectBoundingBox.Clear();
  }
}

void hgeParticleSystem::Render()
{
  ParticleBuffer::iterator particle = mParticleBuffer.begin();
  for (;particle != mParticleBuffer.end(); ++particle)
  {
    info.sprite->SetColor(particle->colColor.GetHWColor());
    JRenderer::GetInstance()->RenderQuad(
      info.sprite,
      particle->vecLocation.x+fTx, particle->vecLocation.y+fTy, 
      particle->fSpin * particle->fAge, 
      particle->fSize, particle->fSize);
  }
}
