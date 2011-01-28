//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
// Note: Inspired by HGE's DistortionMesh.
//
//-------------------------------------------------------------------------------------

#ifndef _JDISTORT_MESH_H
#define _JDISTORT_MESH_H

#include "JRenderer.h"

class JDistortionMesh
{
public:
     JDistortionMesh(JTexture *tex, float x, float y, float width, float height, int cols, int rows);
     ~JDistortionMesh();

     void		Render(float x, float y);
     void		SetColor(int col, int row, PIXEL_TYPE color);
     void		SetDisplacement(int col, int row, float dx, float dy);//, int ref);

private:
	
	static JRenderer *mRenderer;
	Vertex*	mVertices;
	int	mRows;
	int mCols;
	float mCellWidth;
	float mCellHeight;
	float mTexX;
	float mTexY;
	float mTexWidth;
	float mTexHeight;
	JTexture* mTexture;
	JQuad* mQuad;
	
	
};


#endif
