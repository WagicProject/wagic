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
#include "../include/JResourceManager.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JSprite.h"
#include "../include/JAnimator.h"

#include "tinyxml/tinyxml.h"


//////////////////////////////////////////////////////////////////////////
JAnimator::JAnimator(JResourceManager* resourceMgr)
{
	mResource = resourceMgr;
	mActive = false;
	mAnimating = false;
}

JAnimator::~JAnimator()
{
	while (mFrames.size()>0)
	{
		JAnimatorFrame* frame = mFrames.back();
		mFrames.pop_back();
		delete frame;
	}

	mFrames.clear();
}

bool JAnimator::Load(const char* scriptFile)
{

	JFileSystem *fileSystem = JFileSystem::GetInstance();
	if (fileSystem == NULL) return false;

	if (!fileSystem->OpenFile(scriptFile)) return false;

	int size = fileSystem->GetFileSize();
	char *xmlBuffer = new char[size];
	fileSystem->ReadFile(xmlBuffer, size);

	TiXmlDocument doc;
	doc.Parse(xmlBuffer);

	TiXmlNode* script = 0;
	TiXmlNode* frame = 0;
	TiXmlNode* obj = 0;
	TiXmlNode* param = 0;
	TiXmlElement* element = 0;

	float defaultTime = 0.033f;

	script = doc.FirstChild("script"); 
	if (script)
	{
		element = script->ToElement();
		printf("---- Loading %s:%s\n", element->Value(), element->Attribute("name"));

		const char *type[] =
		{
			"ANIMATION_TYPE_LOOPING",
			"ANIMATION_TYPE_ONCE_AND_STAY",
			"ANIMATION_TYPE_ONCE_AND_BACK",
			"ANIMATION_TYPE_ONCE_AND_GONE",
			"ANIMATION_TYPE_PINGPONG"
		};

		const char* aniType = element->Attribute("type");
		for (int i=0;i<5;i++)
			if (strcmp(type[i], aniType)==0)
			{
				SetAnimationType(i);
				break;
			}

		float fps;
		if (element->QueryFloatAttribute("framerate", &fps) != TIXML_SUCCESS)
			fps = 30.0f;

		defaultTime = 1/fps;

		for (frame = script->FirstChild("frame"); frame; frame = frame->NextSibling())
		{
			JAnimatorFrame *aniFrame = new JAnimatorFrame(this);

			float duration;
			element = frame->ToElement();
			if (element->QueryFloatAttribute("time", &duration) != TIXML_SUCCESS)
				duration = defaultTime;
			aniFrame->SetFrameTime(duration);

			for (obj = frame->FirstChild(); obj; obj = obj->NextSibling())
			{
				for (param = obj->FirstChild(); param; param = param->NextSibling())
				{
					
					element = param->ToElement();
					if (element != NULL)
					{
						if (strcmp(element->Value(), "settings")==0)
						{
							const char* quadName = element->Attribute("quad");
							JQuad* quad = mResource->GetQuad(quadName);

							float x, y;
							float vsize, hsize;
							float angle;
							int a, r, g, b;
							int value;
							bool flipped = false;

							if (element->QueryFloatAttribute("x", &x) != TIXML_SUCCESS)
								x = 0.0f;

							if (element->QueryFloatAttribute("y", &y) != TIXML_SUCCESS)
								y = 0.0f;

							if (element->QueryFloatAttribute("hsize", &hsize) != TIXML_SUCCESS)
								hsize = 1.0f;

							if (element->QueryFloatAttribute("vsize", &vsize) != TIXML_SUCCESS)
								vsize = 1.0f;

							if (element->QueryFloatAttribute("rotation", &angle) != TIXML_SUCCESS)
								angle = 0.0f;

							if (element->QueryIntAttribute("a", &a) != TIXML_SUCCESS)
								a = 255;

							if (element->QueryIntAttribute("r", &r) != TIXML_SUCCESS)
								r = 255;

							if (element->QueryIntAttribute("g", &g) != TIXML_SUCCESS)
								g = 255;

							if (element->QueryIntAttribute("b", &b) != TIXML_SUCCESS)
								b = 255;

							if (element->QueryIntAttribute("flip", &value) == TIXML_SUCCESS)
								flipped = (value==1);
							
							
							JAnimatorObject *object = new JAnimatorObject();
							object->SetQuad(quad);
							object->SetPosition(x, y);
							object->SetHScale(hsize);
							object->SetVScale(vsize);
							object->SetRotation(angle);
							object->SetColor(ARGB(a,r,g,b));
							object->SetFlip(flipped);

							aniFrame->AddObject(object);

						}
					}
				}
				
			}

			mFrames.push_back(aniFrame);
		}

	}

	fileSystem->CloseFile();
	delete[] xmlBuffer;

	return true;

}

void JAnimator::Start()
{
	mFrameDelta = 1;
	mAnimating = true;
	mActive = true;
	mCurrentFrame = 0;

	mFrames[mCurrentFrame]->Start();
}

void JAnimator::Stop()
{
	mAnimating = false;
	mActive = false;
}

void JAnimator::Pause()
{
	mAnimating = false;
}

void JAnimator::Resume()
{
	mAnimating = true;
}

void JAnimator::Update(float dt)
{
	if (!mAnimating) return;

	if (mFrames[mCurrentFrame]->Update(dt))
	{
		mCurrentFrame+=mFrameDelta;

		int frameCount = mFrames.size();
		if  (mCurrentFrame >= frameCount)
		{
			if (mAnimationType == JSprite::ANIMATION_TYPE_LOOPING)
				mCurrentFrame = 0;
			else if (mAnimationType == JSprite::ANIMATION_TYPE_ONCE_AND_GONE)
			{
				mAnimating = false;
				mActive = false;
			}
			else if (mAnimationType == JSprite::ANIMATION_TYPE_ONCE_AND_STAY)
			{
				mCurrentFrame = frameCount-1;
				mAnimating = false;
			}
			else if (mAnimationType == JSprite::ANIMATION_TYPE_ONCE_AND_BACK)
			{
				mCurrentFrame = 0;
				mAnimating = false;
			}
			else	// ping pong
			{
				mFrameDelta *= -1;
				mCurrentFrame += mFrameDelta;
			}
		}
		else if (mCurrentFrame < 0)
		{
			if (mAnimationType == JSprite::ANIMATION_TYPE_PINGPONG)
			{
				mFrameDelta *= -1;
				mCurrentFrame += mFrameDelta;
			}
		}

		if (mAnimating)
			mFrames[mCurrentFrame]->Start();
	}

}


void JAnimator::Render()
{
	if (!mActive)
		return;

	mFrames[mCurrentFrame]->Render(mX-mHotSpotX, mY-mHotSpotY);

}


bool JAnimator::IsActive()
{
	return mActive;
}

bool JAnimator::IsAnimating()
{
	return mAnimating;
}


int JAnimator::GetCurrentFrameIndex()
{
	return mCurrentFrame;
}


void JAnimator::SetCurrentFrameIndex(int index)
{
	if (index < (int)mFrames.size())
		mCurrentFrame = index;
}


void JAnimator::SetAnimationType(int type)
{
	mAnimationType = type;
}


JResourceManager* JAnimator::GetResourceManager()
{
	return mResource;
}


void JAnimator::SetPosition(float x, float y)
{
	mX = x;
	mY = y;
}


void JAnimator::SetHotSpot(float x, float y)
{
	mHotSpotX = x;
	mHotSpotY = y;
}


//////////////////////////////////////////////////////////////////////////
JAnimatorFrame::JAnimatorFrame(JAnimator* parent __attribute__((unused)))
{
	mTimer = 0.0f;
	mFrameTime = 100.0f;
}

JAnimatorFrame::~JAnimatorFrame()
{
	while (mObjects.size()>0)
	{
		JAnimatorObject* obj = mObjects.back();
		mObjects.pop_back();
		delete obj;
	}

	mObjects.clear();

}

void JAnimatorFrame::AddObject(JAnimatorObject *obj)
{
	mObjects.push_back(obj);
}

bool JAnimatorFrame::Update(float dt)
{
	mTimer += dt;
	if (mTimer >= mFrameTime)
		return true;
	else
	{
		int size = mObjects.size();
		for (int i=0;i<size;i++)
			mObjects[i]->Update(dt);

		return false;
	}
}

void JAnimatorFrame::Render(float x, float y)
{
	int size = mObjects.size();
	for (int i=0;i<size;i++)
		mObjects[i]->Render(x, y);
}

void JAnimatorFrame::Start()
{
	mTimer = 0.0f;
}

// bool JAnimatorFrame::IsDone()
// {
// 	return false;
// }

void JAnimatorFrame::SetFrameTime(float duration)
{
	mFrameTime = duration;
}

//////////////////////////////////////////////////////////////////////////
JAnimatorObject::JAnimatorObject()
{
	mRenderer = JRenderer::GetInstance();
	mFlipped = false;
}

JAnimatorObject::~JAnimatorObject()
{

}

void JAnimatorObject::Update(float dt __attribute__((unused)))
{
}


void JAnimatorObject::Render(float x, float y)
{
	mQuad->SetHFlip(mFlipped);
	mQuad->SetColor(mColor);
	mRenderer->RenderQuad(mQuad, x+mX, y+mY, mRotation, mHScale, mVScale);
}

void JAnimatorObject::SetQuad(JQuad *quad) { mQuad = quad; }
void JAnimatorObject::SetPosition(float x, float y) { mX = x; mY = y; }
void JAnimatorObject::SetRotation(float angle) { mRotation = angle; }
void JAnimatorObject::SetHScale(float scale) { mHScale = scale; }
void JAnimatorObject::SetVScale(float scale) { mVScale = scale; }
void JAnimatorObject::SetColor(PIXEL_TYPE color) { mColor = color; }
void JAnimatorObject::SetFlip(bool flag) { mFlipped = flag; }
