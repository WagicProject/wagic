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

#include "../include/JFileSystem.h"
#include "../include/JResourceManager.h"

#include "tinyxml/tinyxml.h"

//-------------------------------------------------------------------------------------


JParticleEffect::JParticleEffect(JResourceManager* mgr)
{
	mResourceManager = mgr;

	mX = 0.0f;
	mY = 0.0f;

	mEmitterCount = 0;
}


JParticleEffect::~JParticleEffect()
{
	for (int i=0;i<mEmitterCount;i++)
		if (mParticleEmitters[i])
			delete (mParticleEmitters[i]);
}



bool JParticleEffect::Load(const char* filename)
{
	mX = 0.0f;
	mY = 0.0f;

	mEmitterCount = 0;


	JFileSystem *fileSystem = JFileSystem::GetInstance();
	if (fileSystem == NULL) return false;

	if (!fileSystem->OpenFile(filename)) return false;

	int size = fileSystem->GetFileSize();
	char *xmlBuffer = new char[size];
	fileSystem->ReadFile(xmlBuffer, size);

	TiXmlDocument doc;
	
	doc.Parse(xmlBuffer);

	
	TiXmlNode* effect = 0;
	TiXmlNode* emitter = 0;
	TiXmlNode* param = 0;
	TiXmlNode* key = 0;
	
	TiXmlElement* element = 0;

	float keyTime;
	float value;
	float baseValue;
	int int_value;
// 	
// 	enum ParticleField
// 	{
// 		FIELD_SPEED,
// 		FIELD_SIZE,
// 		FIELD_ROTATION,
// 		FIELD_ALPHA,
// 		FIELD_RED,
// 		FIELD_GREEN,
// 		FIELD_BLUE,
// 		FIELD_COUNT
// 	};

	const char* lifeValues[] =
	{
		"speed",
		"size",
		"rotation",
		"alpha",
		"red",
		"green",
		"blue",
		"radial_accel",
		"tangential_accel",
		"gravity"
	};

	const char* typeNames[] =
	{
		"POINT",
		"AREA",
		"HORIZONTAL",
		"VERTICAL",
		"CIRCLE"
	};

	const char* modeNames[] =
	{
		"REPEAT",
		"ONCE",
		"NTIMES",
		"CONTINUOUS"
	};

	//////////////////////////////////////////////////////////////////////////
	// One effect per file only, well, we may extend it later to permit 
	// multiple effect definitions:
	//
	// for (effect = doc.FirstChild("effect"); effect; effect  = effect->NextSibling())
	// 
	//////////////////////////////////////////////////////////////////////////

	effect = doc.FirstChild("effect"); 
	if (effect)
	{
		element = effect->ToElement();
		printf("%s:%s\n", element->Value(), element->Attribute("name"));


		for (emitter = effect->FirstChild("emitter"); emitter && mEmitterCount < MAX_EMITTER; emitter = emitter->NextSibling())
		{

			mParticleEmitters[mEmitterCount] = new JParticleEmitter(this);
			element = emitter->ToElement();

			if (element->QueryFloatAttribute("life", &value) == TIXML_SUCCESS)
				mParticleEmitters[mEmitterCount]->mLife = value;

			for (param = emitter->FirstChild(); param; param = param->NextSibling())
			{
				element = param->ToElement();
				
				if (strcmp(element->Attribute("name"), "settings")==0)
				{
					
					if (strcmp(element->Attribute("blend"), "NORMAL")==0)
						mParticleEmitters[mEmitterCount]->SetBlending(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
					else if (strcmp(element->Attribute("blend"), "ADDITIVE")==0)
						mParticleEmitters[mEmitterCount]->SetBlending(BLEND_SRC_ALPHA, BLEND_ONE);
					
					for (unsigned int i=0;i<sizeof(modeNames)/sizeof(char*);i++)
					{
						if (strcmp(element->Attribute("mode"), modeNames[i])==0)
						{
							mParticleEmitters[mEmitterCount]->mEmitterMode = i;
							#if defined (_DEBUG)
								printf("emitter mode:%s\n", modeNames[i]);
							#endif
							break;
						}
					}
					for (unsigned i=0;i<sizeof(typeNames)/sizeof(char*);i++)
					{
						if (strcmp(element->Attribute("type"), typeNames[i])==0)
						{
							mParticleEmitters[mEmitterCount]->mType = i;
							#if defined (_DEBUG)
								printf("emitter type:%s\n", typeNames[i]);
							#endif
							break;
						}
					}

					string quadName = element->Attribute("image");
					JQuad* quad = mResourceManager->GetQuad(quadName);
					if (quad != NULL)
						mParticleEmitters[mEmitterCount]->SetQuad(quad);
					
// 					if (element->QueryIntAttribute("image", &int_value) == TIXML_SUCCESS)
// 					{
// 						mParticleEmitters[mEmitterCount]->mQuadIndex = int_value;
// 
// 					}
					if (element->QueryIntAttribute("width", &int_value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mWidth = int_value;

					}
					if (element->QueryIntAttribute("height", &int_value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mHeight = int_value;

					}

					if (element->QueryIntAttribute("id", &int_value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mId = int_value;

					}

					if (element->QueryIntAttribute("repeat_count", &int_value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mRepeatTimes = int_value;

					}
				}
				else if (strcmp(element->Attribute("name"), "quantity")==0)
				{
					for (key = param->FirstChild(); key; key = key->NextSibling())
					{
						element = key->ToElement();

						if (element->QueryFloatAttribute("timeslice", &keyTime) == TIXML_SUCCESS &&
							element->QueryFloatAttribute("value", &value) == TIXML_SUCCESS)
						{
							mParticleEmitters[mEmitterCount]->mQuantity.AddKey(keyTime, value);
						}
						
					}
				}
				else if (strcmp(element->Attribute("name"), "lifex")==0)
				{
					if (element->QueryFloatAttribute("base", &baseValue) == TIXML_SUCCESS &&
						element->QueryFloatAttribute("max", &value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mLifeBase = baseValue;
						mParticleEmitters[mEmitterCount]->mLifeMax= value;	
					}
				}
				else if (strcmp(element->Attribute("name"), "anglex")==0)
				{
					if (element->QueryFloatAttribute("base", &baseValue) == TIXML_SUCCESS &&
						element->QueryFloatAttribute("max", &value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mAngleBase = baseValue*DEG2RAD;
						mParticleEmitters[mEmitterCount]->mAngleMax= value*DEG2RAD;	
					}
				}
				else if (strcmp(element->Attribute("name"), "speedx")==0)
				{
					if (element->QueryFloatAttribute("base", &baseValue) == TIXML_SUCCESS &&
						element->QueryFloatAttribute("max", &value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mSpeedBase = baseValue;
						mParticleEmitters[mEmitterCount]->mSpeedMax= value;	
					}
				}
				else if (strcmp(element->Attribute("name"), "sizex")==0)
				{
					if (element->QueryFloatAttribute("base", &baseValue) == TIXML_SUCCESS &&
						element->QueryFloatAttribute("max", &value) == TIXML_SUCCESS)
					{
						mParticleEmitters[mEmitterCount]->mSizeBase = baseValue;
						mParticleEmitters[mEmitterCount]->mSizeMax= value;	
					}
				}
				else 
				{
					for (int i=0;i<FIELD_COUNT;i++)
					{
						if (strcmp(element->Attribute("name"), lifeValues[i])==0)
						{
							for (key = param->FirstChild(); key; key = key->NextSibling())
							{
								element = key->ToElement();

								if (element->QueryFloatAttribute("lifeslice", &keyTime) == TIXML_SUCCESS &&
									element->QueryFloatAttribute("value", &value) == TIXML_SUCCESS)
								{
									if (i==FIELD_ROTATION)
										value *= DEG2RAD;
									
									mParticleEmitters[mEmitterCount]->mData[i].AddKey(keyTime, value);
								}

							}
							break;
						}
					}

				}


			}

			mEmitterCount++;
		}
	}

	fileSystem->CloseFile();
	delete[] xmlBuffer;

	return true;
}


void JParticleEffect::SetParticleSystem(JParticleSystem* particleSys)
{
	mParticleSystem = particleSys;
}


void JParticleEffect::SetPosition(float x, float y)
{
	mX = x;
	mY = y;
}


JParticleSystem* JParticleEffect::GetParticleSystem()
{
	return mParticleSystem;
}


float JParticleEffect::GetX()
{
	return mX;
}


float JParticleEffect::GetY()
{
	return mY;
}


void JParticleEffect::Update(float dt)
{
//	mTimer += dt;
	
	for (int i=0;i<mEmitterCount;i++)
		mParticleEmitters[i]->Update(dt);
}

void JParticleEffect::Render()
{
	for (int i=0;i<mEmitterCount;i++)
		mParticleEmitters[i]->Render();
}

bool JParticleEffect::Done()
{
	bool done = true;
	for (int i=0;i<mEmitterCount;i++)
		if (!mParticleEmitters[i]->Done())
			done = false;

	return (done);
}


void JParticleEffect::Start()
{
	for (int i=0;i<mEmitterCount;i++)
		mParticleEmitters[i]->Start();
}


void JParticleEffect::Stop()
{

	for (int i=0;i<mEmitterCount;i++)
		mParticleEmitters[i]->SetActive(false);
}


void JParticleEffect::MoveTo(float x, float y)
{
	float dx = x - mX;
	float dy = y - mY;

	mX = x;
	mY = y;
	
	for (int i=0;i<mEmitterCount;i++)
	{
		mParticleEmitters[i]->MoveAllParticles(dx, dy);
	}
}
