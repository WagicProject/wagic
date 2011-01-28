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

#include "../include/JFileSystem.h"
#include "../include/JSpline.h"

#include "tinyxml/tinyxml.h"

#define SMALL_NUMBER		0.0001f


JSpline::JSpline()
{
	mCount = 0;

	mMidPoints.reserve(32);
	mPixels.reserve(32);
}


JSpline::~JSpline()
{
	mCount = 0;

	mMidPoints.clear();
	mPixels.clear();
}


bool JSpline::Load(const char *filename, float xscale, float yscale)
{
	JFileSystem *fileSystem = JFileSystem::GetInstance();

	if (fileSystem == NULL) return false;
	if (!fileSystem->OpenFile(filename)) return false;

	int size = fileSystem->GetFileSize();
	char *xmlBuffer = new char[size];
	fileSystem->ReadFile(xmlBuffer, size);

	TiXmlDocument doc;
	doc.Parse(xmlBuffer);

	mCount = 0;

	mMidPoints.clear();
	mPixels.clear();


	TiXmlNode* node = 0;
	//TiXmlElement* todoElement = 0;
	TiXmlElement* element;

	node = doc.RootElement();

	float xx, yy;

	for(element = node->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		xx = 0.0f;
		yy = 0.0f;
		element->QueryFloatAttribute("x", &xx);
		element->QueryFloatAttribute("y", &yy);

		Point pt(xx*xscale, yy*yscale);
		AddControlPoint(pt);
		
	}

	fileSystem->CloseFile();
	delete[] xmlBuffer;

	return true;
}



void JSpline::PointOnCurve(Point &out, float t, const Point &p0, const Point &p1, const Point &p2, const Point &p3)
{
	float t2 = t * t;
	float t3 = t2 * t;
	out.x = 0.5f * (( 2.0f * p1.x ) +
		( -p0.x + p2.x ) * t +
		( 2.0f * p0.x - 5.0f * p1.x + 4 * p2.x - p3.x ) * t2 +
		( -p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x ) * t3 );

	out.y = 0.5f * ( ( 2.0f * p1.y ) +
		( -p0.y + p2.y ) * t +
		( 2.0f * p0.y - 5.0f * p1.y + 4 * p2.y - p3.y ) * t2 +
		( -p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y ) * t3 );

}



void JSpline::GeneratePixels()
{
	float x, y;

	float inc = SMALL_NUMBER;

	mPixels.clear();

	x = mMidPoints[1].x;
	y = mMidPoints[1].y;

	Point newPt(x, y);
	Point extraPt;

	mPixels.push_back(newPt);

	for (int n=0; n < (int)mMidPoints.size()-3; n++)
	{
		float t = inc;
		while (t <= 1.0f)
		{
			PointOnCurve(newPt, t, mMidPoints[n], mMidPoints[n+1], mMidPoints[n+2], mMidPoints[n+3]);

			float dx = newPt.x-x;
			float dy = newPt.y-y;

			float dist = sqrtf(dx*dx + dy*dy);
			if (dist >= MID_POINT_THRESHOLD)
			{
				//
				//extraPt.x = (newPt.x+x)/2;
				//extraPt.y = (newPt.y+y)/2;
				//mPixels.push_back(extraPt);
				//
				mPixels.push_back(newPt);
				x = newPt.x;
				y = newPt.y;
			}

			t += inc;
		}
	}


	mCount = mPixels.size();

}


void JSpline::AddControlPoint(const Point &pt)
{
	mMidPoints.push_back(pt);
}


void JSpline::GetControlPoint(Point &point, int index)
{
	if (index < (int)mMidPoints.size())
	{
		point.x = mMidPoints[index].x;
		point.y = mMidPoints[index].y;
	}
}


void JSpline::GetPixel(Point &point, int index)
{
	if (index < (int)mPixels.size())
	{
		point.x = mPixels[index].x;
		point.y = mPixels[index].y;
	}
}

int JSpline::GetPixelCount()
{
	return mCount;
}


void JSpline::Render(float x, float y, PIXEL_TYPE color, PIXEL_TYPE controlColor)
{
	if (mCount > 0)
	{
		JRenderer* renderer = JRenderer::GetInstance();
//		renderer->SetLineWidth(1.2f);

		int size = mPixels.size();
		
		for (int i=0;i<size-1;i++)
			renderer->DrawLine(x+mPixels[i].x, y+mPixels[i].y, x+mPixels[i+1].x, y+mPixels[i+1].y, color);

		size = mMidPoints.size();
		for (int i=0; i < size; i++)
			renderer->FillRect(mMidPoints[i].x-3, mMidPoints[i].y-3, 6, 6, controlColor);
	}
	
}

