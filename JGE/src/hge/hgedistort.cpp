/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeDistortionMesh helper class implementation
*/
#include "../../include/hge/hgedistort.h"

#include "../../include/JGE.h"
#include "../../include/JRenderer.h"
#include "../../include/JFileSystem.h"


//HGE *hgeDistortionMesh::hge=0;


hgeDistortionMesh::hgeDistortionMesh(int cols, int rows)
{
	int i;

	//hge=hgeCreate(HGE_VERSION);

	nRows=rows;
	nCols=cols;
	cellw=cellh=0;
	//quad.tex=0;
	//quad.blend=BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_ZWRITE;

	quad = NULL;

	disp_array=new Vertex[rows*cols];

	for(i=0;i<rows*cols;i++)
	{
		disp_array[i].x=0.0f;
		disp_array[i].y=0.0f;
		disp_array[i].u=0.0f;
		disp_array[i].v=0.0f;

		disp_array[i].z=0.5f;
		disp_array[i].color=ARGB(0xFF,0xFF,0xFF,0xFF);
	}
}

hgeDistortionMesh::hgeDistortionMesh(const hgeDistortionMesh &dm)
{
//	hge=hgeCreate(HGE_VERSION);

	nRows=dm.nRows;
	nCols=dm.nCols;
	cellw=dm.cellw;
	cellh=dm.cellh;
	tx=dm.tx;
	ty=dm.ty;
	width=dm.width;
	height=dm.height;
	quad=dm.quad;

	disp_array=new Vertex[nRows*nCols];
	memcpy(disp_array, dm.disp_array, sizeof(Vertex)*nRows*nCols);
}

hgeDistortionMesh::~hgeDistortionMesh()
{
	delete[] disp_array;
	SAFE_DELETE(quad);
}

hgeDistortionMesh& hgeDistortionMesh::operator= (const hgeDistortionMesh &dm)
{
	if(this!=&dm)
	{
		nRows=dm.nRows;
		nCols=dm.nCols;
		cellw=dm.cellw;
		cellh=dm.cellh;
		tx=dm.tx;
		ty=dm.ty;
		width=dm.width;
		height=dm.height;
		quad=dm.quad;

		delete[] disp_array;
		disp_array=new Vertex[nRows*nCols];
		memcpy(disp_array, dm.disp_array, sizeof(Vertex)*nRows*nCols);
	}

	return *this;

}

void hgeDistortionMesh::SetTexture(JTexture* tex)
{
	if (quad)
		delete quad;

	quad = new JQuad(tex, 0, 0, 16, 16);
	//quad.tex=tex;
}

void hgeDistortionMesh::SetTextureRect(float x, float y, float w, float h)
{
	int i,j;

	tx=x; ty=y; width=w; height=h;

	cellw=w/(nCols-1);
	cellh=h/(nRows-1);

	for(j=0; j<nRows; j++)
		for(i=0; i<nCols; i++)
		{
			disp_array[j*nCols+i].u=(x+i*cellw);
			disp_array[j*nCols+i].v=(y+j*cellh);

			disp_array[j*nCols+i].x=i*cellw;
			disp_array[j*nCols+i].y=j*cellh;
		}
}

void hgeDistortionMesh::SetBlendMode(int blend __attribute__((unused)))
{
//	quad.blend=blend;
}

void hgeDistortionMesh::Clear(PIXEL_TYPE col, float z)
{
	int i,j;

	for(j=0; j<nRows; j++)
		for(i=0; i<nCols; i++)
		{
			disp_array[j*nCols+i].x=i*cellw;
			disp_array[j*nCols+i].y=j*cellh;
			disp_array[j*nCols+i].color=col;
			disp_array[j*nCols+i].z=z;
		}
}

void hgeDistortionMesh::Render(float x, float y)
{
	int i,j,idx;

	VertexColor points[4];
	JRenderer* renderer = JRenderer::GetInstance();

	for(j=0; j<nRows-1; j++)
		for(i=0; i<nCols-1; i++)
		{
			idx=j*nCols+i;

			quad->SetTextureRect(disp_array[idx].u, disp_array[idx].v, cellw, cellh);

			points[0].x = x+disp_array[idx+nCols].x;
			points[0].y = y+disp_array[idx+nCols].y;
			points[0].z = disp_array[idx+nCols].z;
			points[0].color = disp_array[idx+nCols].color;

			points[1].x = x+disp_array[idx+nCols+1].x;
			points[1].y = y+disp_array[idx+nCols+1].y;
			points[1].z = disp_array[idx+nCols+1].z;
			points[1].color = disp_array[idx+nCols+1].color;

			points[2].x = x+disp_array[idx+1].x;
			points[2].y = y+disp_array[idx+1].y;
			points[2].z = disp_array[idx+1].z;
			points[2].color = disp_array[idx+1].color;

			points[3].x = x+disp_array[idx].x;
			points[3].y = y+disp_array[idx].y;
			points[3].z = disp_array[idx].z;
			points[3].color = disp_array[idx].color;

			renderer->RenderQuad(quad, points);

		}
}

void hgeDistortionMesh::SetZ(int col, int row, float z)
{
	if(row<nRows && col<nCols) disp_array[row*nCols+col].z=z;
}

void hgeDistortionMesh::SetColor(int col, int row, PIXEL_TYPE color)
{
	if(row<nRows && col<nCols) disp_array[row*nCols+col].color=color;
}

void hgeDistortionMesh::SetDisplacement(int col, int row, float dx, float dy, int ref)
{
	if(row<nRows && col<nCols)
	{
		switch(ref)
		{
			case HGEDISP_NODE:		dx+=col*cellw; dy+=row*cellh; break;
			case HGEDISP_CENTER:	dx+=cellw*(nCols-1)/2;dy+=cellh*(nRows-1)/2; break;
			case HGEDISP_TOPLEFT:	break;
		}

		disp_array[row*nCols+col].x=dx;
		disp_array[row*nCols+col].y=dy;
	}
}

float hgeDistortionMesh::GetZ(int col, int row) const
{
	if(row<nRows && col<nCols) return disp_array[row*nCols+col].z;
	else return 0.0f;
}

PIXEL_TYPE hgeDistortionMesh::GetColor(int col, int row) const
{
	if(row<nRows && col<nCols) return disp_array[row*nCols+col].color;
	else return 0;
}

void hgeDistortionMesh::GetDisplacement(int col, int row, float *dx, float *dy, int ref) const
{
	if(row<nRows && col<nCols)
	{
		switch(ref)
		{
			case HGEDISP_NODE:		*dx=disp_array[row*nCols+col].x-col*cellw;
									*dy=disp_array[row*nCols+col].y-row*cellh;
									break;

			case HGEDISP_CENTER:	*dx=disp_array[row*nCols+col].x-cellw*(nCols-1)/2;
									*dy=disp_array[row*nCols+col].x-cellh*(nRows-1)/2;
									break;

			case HGEDISP_TOPLEFT:	*dx=disp_array[row*nCols+col].x;
									*dy=disp_array[row*nCols+col].y;
									break;
		}
	}
}

