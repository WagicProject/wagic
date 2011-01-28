//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#include "../include/JDistortionMesh.h"


JRenderer* JDistortionMesh::mRenderer = NULL;

JDistortionMesh::JDistortionMesh(JTexture *tex, float x, float y, float width, float height, int cols, int rows)
{
	mRenderer = JRenderer::GetInstance();

	mCols = cols;
	mRows = rows;

	mCellWidth = width/(mCols-1);
	mCellHeight = height/(mRows-1);

	mTexX = x;
	mTexY = y;
	mTexWidth = width;
	mTexHeight = height;
	
	mQuad = new JQuad(tex, x, y, mCellWidth, mCellHeight);

	mVertices = new Vertex[mCols*mRows];

	for(int j=0; j<mRows; j++)
	{
		for(int i=0; i<mCols; i++)
		{
			mVertices[j*mCols+i].u = x+i*mCellWidth;
			mVertices[j*mCols+i].v = y+j*mCellHeight;

			mVertices[j*mCols+i].x = i*mCellWidth;
			mVertices[j*mCols+i].y = j*mCellHeight;

			mVertices[j*mCols+i].color = ARGB(0,0,0,0);
		}
	}

}


JDistortionMesh::~JDistortionMesh()
{

	delete mQuad;
	delete[] mVertices;

//	JGERelease();
}


void JDistortionMesh::Render(float x, float y)
{

//	mQuad->mBlend = GU_TFX_ADD;

	VertexColor points[4];
	
	int index;

	for(int j=0; j<mRows-1; j++)
	{
		for(int i=0; i<mCols-1; i++)
		{
			index=j*mCols+i;

			mQuad->SetTextureRect(mVertices[index].u, mVertices[index].v, mCellWidth, mCellHeight);

			points[0].x = x+mVertices[index].x;
			points[0].y = y+mVertices[index].y;
			points[0].z = mVertices[index].z;
			points[0].color = mVertices[index].color;

			points[1].x = x+mVertices[index+1].x;
			points[1].y = y+mVertices[index+1].y;
			points[1].z = mVertices[index+1].z;
			points[1].color = mVertices[index+1].color;

			points[2].x = x+mVertices[index+mCols].x;
			points[2].y = y+mVertices[index+mCols].y;
			points[2].z = mVertices[index+mCols].z;
			points[2].color = mVertices[index+mCols].color;

			points[3].x = x+mVertices[index+mCols+1].x;
			points[3].y = y+mVertices[index+mCols+1].y;
			points[3].z = mVertices[index+mCols+1].z;
			points[3].color = mVertices[index+mCols+1].color;

			mRenderer->RenderQuad(mQuad, points);
		}
	}


}


void JDistortionMesh::SetColor(int col, int row, PIXEL_TYPE color)
{
	mVertices[row*mCols+col].color = color;

}
 

void JDistortionMesh::SetDisplacement(int col, int row, float dx, float dy)
{
		dx += col*mCellWidth;
		dy += row*mCellHeight;

		mVertices[row*mCols+col].x = dx;
		mVertices[row*mCols+col].y = dy;
}
