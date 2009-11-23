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
class JSample;
class JMusic;
class JTexture;
class JQuad;
class JLBFont;

class JResourceManager
{
public:
	JResourceManager();
	virtual ~JResourceManager();

	//void SetResourceRoot(const string& resourceRoot);
	bool LoadResource(const string& resourceName);

	virtual void RemoveAll();
  virtual void RemoveJLBFonts();
	
	virtual int CreateTexture(const string &textureName);
	virtual JTexture* GetTexture(const string &textureName);
	virtual JTexture* GetTexture(int id);

	virtual int CreateQuad(const string &quadName, const string &textureName, float x, float y, float width, float height);
	virtual JQuad* GetQuad(const string &quadName);
	virtual JQuad* GetQuad(int id);

	virtual JLBFont * LoadJLBFont(const string &fontName, int height);
	virtual JLBFont* GetJLBFont(const string &fontName);
	virtual JLBFont* GetJLBFont(int id);


protected:

	vector<JTexture *> mTextureList;
	map<string, int> mTextureMap;

	vector<JQuad *> mQuadList;
	map<string, int> mQuadMap;

	vector<JLBFont *> mFontList;
	map<string, int> mFontMap;
};

#endif
