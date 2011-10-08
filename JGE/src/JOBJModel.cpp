//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#include "../include/JTypes.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JOBJModel.h"

JOBJModel::JOBJModel()
{
	mPolygons = NULL;
	mTexture = NULL;
}

JOBJModel::~JOBJModel()
{
	if (mPolygons)
		delete [] mPolygons;

	if (mTexture)
		delete mTexture;
}

int JOBJModel::ReadLine(char *output, const char *buffer, int start, int size)
{
	int index = 0;
	while (start < size && buffer[start] != '\n' && buffer[start] != '\r')
		output[index++] = buffer[start++];

	while ((start < size && buffer[start] == '\n') || buffer[start] == '\r')
		start++;

	output[index] = 0;

	return start;

}

bool JOBJModel::Load(const char *modelName, const char *textureName)
{

	JFileSystem* fileSys = JFileSystem::GetInstance();
	if (!fileSys->OpenFile(modelName))
		return false;

	int size = fileSys->GetFileSize();
	char *buffer = new char[size];

	fileSys->ReadFile(buffer, size);
	fileSys->CloseFile();

	Vector3D vert;

	vector<Face> faceList;
	vector<Vector3D> normalList;
	vector<Vector3D> texList;
	vector<Vector3D> vertList;

	normalList.reserve(32);
	texList.reserve(32);
	vertList.reserve(32);
	faceList.reserve(32);

	int filePtr = 0;

	char tmpLine[256];
	char s1[256];
	
	int count;

    while (filePtr < size)
    {
		filePtr = ReadLine(tmpLine, buffer, filePtr, size);
        {

			if ((tmpLine[0] == '#') || (strlen(tmpLine)  < 3))
			{
			}
			else if (tmpLine[0] == 'v')
			{
				count = sscanf(tmpLine, "%s  %f %f %f", s1, &vert.x, &vert.y, &vert.z);

				if (count == 4)
				{
					if (strcmp(s1, "vn") == 0)
						normalList.push_back(vert);
					else if (strcmp(s1, "vt") == 0)
						texList.push_back(vert);
					else if (strcmp(s1, "v") == 0)
						vertList.push_back(vert);
				}
				else if (count == 3)
				{
					if (strcmp(s1, "vt") == 0)
						texList.push_back(vert);
				}
	             
			}
			else if (tmpLine[0] == 'f')
			{
				Face face;
				face.mVertCount = 0;

				char *p = strchr(tmpLine, ' ');
				char *pNext = NULL;

				int vertIdx, texIdx, norIdx;
				
				while (p != NULL)
				{
					while (((*p) == ' ') || ((*p) == '\n') || ((*p) == '\t'))
						++p;
					strcpy(s1, p);
					count = sscanf(s1, "%d/%d/%d", &vertIdx, &texIdx, &norIdx);
					if (count == 3)
					{
						if (face.mVertCount < 4)
						{
							face.mVertIdx[face.mVertCount] = vertIdx - 1;
							face.mTexIdx[face.mVertCount] = texIdx - 1;
							face.mNormalIdx[face.mVertCount] = norIdx - 1;
							face.mVertCount++;
						}
					}
					else if (count == 2)
					{
						if (face.mVertCount < 4)
						{
							face.mVertIdx[face.mVertCount] = vertIdx - 1;
							face.mTexIdx[face.mVertCount] = texIdx - 1;
							face.mNormalIdx[face.mVertCount] = 0;
							face.mVertCount++;
						}
					}
					else if (count == 1)
					{
						if (face.mVertCount < 4)
						{
							face.mVertIdx[face.mVertCount] = vertIdx - 1;
							face.mTexIdx[face.mVertCount] = 0;
							face.mNormalIdx[face.mVertCount] = 0;
							face.mVertCount++;
						}
					}
					
					pNext = strchr(p, ' ');
					p = pNext;
				}
			
				if (face.mVertCount == 3)		// we do triangles only ;)
					faceList.push_back(face);
			}
			else if (tmpLine[0] == 'g')
			{
				
			}
			else if (tmpLine[0] == 'u')
			{
			}

		}
	}

	mPolycount = faceList.size();
	mPolygons = new Vertex3D[mPolycount*3];

	int idx = 0;
	for (int i=0;i<mPolycount;i++)
	{
		
		for (int j=0;j<3;j++)
		{
			mPolygons[idx].u = texList[faceList[i].mTexIdx[j]].x;
			mPolygons[idx].v = 1.0f-texList[faceList[i].mTexIdx[j]].y;
			mPolygons[idx].x = vertList[faceList[i].mVertIdx[j]].x;
			mPolygons[idx].y = vertList[faceList[i].mVertIdx[j]].y;
			mPolygons[idx].z = vertList[faceList[i].mVertIdx[j]].z;
			idx++;
		}
		
		/*
		mPolygons[idx].u = texList[faceList[i].mTexIdx[0]].x;
		mPolygons[idx].v = 1.0-texList[faceList[i].mTexIdx[0]].y;
		mPolygons[idx].x = vertList[faceList[i].mVertIdx[0]].x;
		mPolygons[idx].y = vertList[faceList[i].mVertIdx[0]].y;
		mPolygons[idx].z = vertList[faceList[i].mVertIdx[0]].z;
		idx++;
		mPolygons[idx].u = texList[faceList[i].mTexIdx[1]].x;
		mPolygons[idx].v = 1.0-texList[faceList[i].mTexIdx[1]].y;
		mPolygons[idx].x = vertList[faceList[i].mVertIdx[1]].x;
		mPolygons[idx].y = vertList[faceList[i].mVertIdx[1]].y;
		mPolygons[idx].z = vertList[faceList[i].mVertIdx[1]].z;
		idx++;
		mPolygons[idx].u = texList[faceList[i].mTexIdx[2]].x;
		mPolygons[idx].v = 1.0-texList[faceList[i].mTexIdx[2]].y;
		mPolygons[idx].x = vertList[faceList[i].mVertIdx[2]].x;
		mPolygons[idx].y = vertList[faceList[i].mVertIdx[2]].y;
		mPolygons[idx].z = vertList[faceList[i].mVertIdx[2]].z;
		idx++;
		*/
	}
   

	if (textureName != NULL)
		mTexture = JRenderer::GetInstance()->LoadTexture(textureName);

    return true;

}


void JOBJModel::Render()
{
	JRenderer::GetInstance()->RenderTriangles(mTexture, mPolygons, 0, mPolycount);
}
