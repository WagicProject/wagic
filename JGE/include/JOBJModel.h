//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#ifndef _OBJMODEL_H
#define _OBJMODEL_H

#include <vector>

using namespace std;

#if defined (PSP)
#include <pspgu.h>
#include <pspgum.h>
#endif


#include "JGE.h"
#include "Vector3D.h"

class JTexture;

//////////////////////////////////////////////////////////////////////////
/// Helper class to display Wavefront OBJ model.
/// 
//////////////////////////////////////////////////////////////////////////
class JOBJModel
{

struct Face
{
	int mVertCount;
	int mVertIdx[4];
	int mTexIdx[4];
	int mNormalIdx[4];
};

public:

	//////////////////////////////////////////////////////////////////////////
	/// Constructor.
	//////////////////////////////////////////////////////////////////////////
	JOBJModel();
	
	~JOBJModel();

	int ReadLine(char *output, const char *buffer, int start, int size);
	
	//////////////////////////////////////////////////////////////////////////
	/// Load OBJ model.
	/// 
	/// @param modelName - Name of OBJ file.
	/// @param texturenName - Name of texture.
	/// 
	//////////////////////////////////////////////////////////////////////////
	bool Load(const char *modelName, const char *textureName);
	
	//////////////////////////////////////////////////////////////////////////
	/// Render the model to screen.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void Render();

private:
	int mPolycount;
	Vertex3D* mPolygons;
	JTexture* mTexture;
};

#endif
