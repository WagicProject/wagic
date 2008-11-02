//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#ifndef _RESOURCE_MANAGER_H_
#define _RESOURCE_MANAGER_H_

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include <stdio.h>
#include <vector>
#include <map>
#include <string>

using namespace std;


#define INVALID_ID				-1

class JRenderer;
class JParticleEffect;
class JMotionEmitter;
class JSample;
class JMusic;
class JTexture;
class JQuad;
class JLBFont;

class JResourceManager
{
public:
	JResourceManager();
	~JResourceManager();

	//void SetResourceRoot(const string& resourceRoot);
	bool LoadResource(const string& resourceName);

	void RemoveAll();
	void RemoveGraphics();
	void RemoveSound();
	void RemoveFont();
	
	int CreateTexture(const string &textureName);
	JTexture* GetTexture(const string &textureName);
	JTexture* GetTexture(int id);

	int CreateQuad(const string &quadName, const string &textureName, float x, float y, float width, float height);
	JQuad* GetQuad(const string &quadName);
	JQuad* GetQuad(int id);

	int LoadJLBFont(const string &fontName, int height);
	JLBFont* GetJLBFont(const string &fontName);
	JLBFont* GetJLBFont(int id);

	int LoadMusic(const string &musicName);
	JMusic* GetMusic(const string &musicName);
	JMusic* GetMusic(int id);

	int LoadSample(const string &sampleName);
	JSample* GetSample(const string &sampleName);
	JSample* GetSample(int id);

// 	int RegisterParticleEffect(const string &effectName);
// 	JParticleEffect* GetParticleEffect(const string &effectName);
// 	JParticleEffect* GetParticleEffect(int id);
// 
// 	int RegisterMotionEmitter(const string &emitterName);
// 	JMotionEmitter* GetMotionEmitter(const string &emitterName);
// 	JMotionEmitter* GetMotionEmitter(int id);

private:

	//JRenderer *mRenderer;
	
	//string mResourceRoot;

	vector<JTexture *> mTextureList;
	map<string, int> mTextureMap;

	vector<JQuad *> mQuadList;
	map<string, int> mQuadMap;

// 	vector<JParticleEffect *> mParticleEffectList;
// 	map<string, int> mParticleEffectMap;
// 
// 	vector<JMotionEmitter *> mMotionEmitterList;
// 	map<string, int> mMotionEmitterMap;

	vector<JLBFont *> mFontList;
	map<string, int> mFontMap;

	vector<JMusic *> mMusicList;
	map<string, int> mMusicMap;

	vector<JSample *> mSampleList;
	map<string, int> mSampleMap;
};

#endif
