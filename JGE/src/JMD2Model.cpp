//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JMD2Model.h"


MD2Animation::MD2Animation(int start, int end)
{
	mStartFrame = start;
	mEndFrame = end;
}

JRenderer* JMD2Model::mRenderer = NULL;


//-------------------------------------------------------------------------------------------------
JMD2Model::JMD2Model()
{
	mRenderer = JRenderer::GetInstance();

	int endFrames[] = { 39, 46, 60, 66, 73, 95, 112, 122, 135, 154, 161, 169, 177, 185, 190, 198 };
	int startFrame = 0;

	mAnimations = (MD2Animation **)malloc(sizeof(MD2Animation)*MAX_ANIMATION);
	for (int i=0;i<MAX_ANIMATION;i++)
	{
		mAnimations[i] = new MD2Animation(startFrame, endFrames[i]);
		startFrame = endFrames[i]+1;
	}

	mState = -1;
	mNextState = STATE_IDLE;

	mModel = NULL;
	mAnimationSpeed = 6.0f;
}


//-------------------------------------------------------------------------------------------------
JMD2Model::~JMD2Model()
{
	if (mModel)
	{
	
		if (mModel->triIndex != NULL)
			free(mModel->triIndex);
		if (mModel->pointList != NULL)
			free(mModel->pointList);
		if (mModel->st != NULL)
			free(mModel->st);
		if (mModel->modelTex != NULL)
			delete mModel->modelTex;
	
		free(mModel);
	}

	if (mAnimations)
	{
		for (int i=0;i<MAX_ANIMATION;i++)
			delete mAnimations[i];

		free(mAnimations);
	}

	
}


//-------------------------------------------------------------------------------------------------
// loads MD2 model
bool JMD2Model::Load(char *filename, char *textureName)
{

	//FILE *filePtr;						// file pointer
	int fileLen;						// length of model file
    char *buffer;						// file buffer
		
	modelHeader_t *modelHeader;			// model header

	stIndex_t *stPtr;					// texture data
    frame_t *frame;						// frame data
	Vector3D *pointListPtr;				// index variable
    mesh_t *triIndex, *bufIndexPtr;		// index variables
    int i, j;							// index variables

	// open the model file

	JFileSystem* fileSystem = JFileSystem::GetInstance();
	if (!fileSystem->OpenFile(filename))
		return false;
	//filePtr = fopen(filename, "rb");
	//if (filePtr == NULL)
	//	return false;

	// find length of file
    //fseek(filePtr, 0, SEEK_END);
    //fileLen = ftell(filePtr);
    //fseek(filePtr, 0, SEEK_SET);

	fileLen = fileSystem->GetFileSize();
	
    // read entire file into buffer
    buffer = (char*)malloc(fileLen + 1);
    //fread(buffer, sizeof(char), fileLen, filePtr);
	fileSystem->ReadFile(buffer, fileLen);
	fileSystem->CloseFile();

	// extract model file header from buffer
    modelHeader = (modelHeader_t*)buffer;

	// allocate memory for model data
   	mModel = (modelData_t*)malloc(sizeof(modelData_t));
	if (mModel == NULL)
		return false;

	// allocate memory for all vertices used in model, including animations
    mModel->pointList = (Vector3D *)malloc(sizeof(Vector3D)*modelHeader->numXYZ * modelHeader->numFrames);

	// store vital model data
    mModel->numPoints = modelHeader->numXYZ;
    mModel->numFrames = modelHeader->numFrames;
	mModel->frameSize = modelHeader->framesize;

    // loop number of frames in model file
    for(j = 0; j < modelHeader->numFrames; j++)
    {
       // offset to the points in this frame
       frame = (frame_t*)&buffer[modelHeader->offsetFrames + modelHeader->framesize * j];

	   // calculate the point positions based on frame details
       pointListPtr = (Vector3D *)&mModel->pointList[modelHeader->numXYZ * j];
       for(i = 0; i < modelHeader->numXYZ; i++)
       {
          pointListPtr[i].x = frame->scale[0] * frame->fp[i].v[0] + frame->translate[0];
          pointListPtr[i].y = frame->scale[1] * frame->fp[i].v[1] + frame->translate[1];
          pointListPtr[i].z = frame->scale[2] * frame->fp[i].v[2] + frame->translate[2];
       }
    }
			 
	JTexture *tex = mRenderer->LoadTexture(textureName);
	if (tex)
		mModel->modelTex = tex;
	else
	{
		free(mModel);
		mModel = NULL;
		
		free(buffer);
		return false;
	}

	float texWidth = (float)tex->mWidth;
	float texHeight = (float)tex->mHeight;


    // allocate memory for the model texture coordinates
    mModel->st = (texCoord_t*)malloc(sizeof(texCoord_t)*modelHeader->numST);

	// store number of texture coordinates
    mModel->numST = modelHeader->numST;

	// set texture pointer to texture coordinate offset
    stPtr = (stIndex_t*)&buffer[modelHeader->offsetST];

	// calculate and store the texture coordinates for the model
    for (i = 0; i < modelHeader->numST; i++)
    {
		mModel->st[i].s = (float)stPtr[i].s / texWidth;
        mModel->st[i].t = (float)stPtr[i].t / texHeight;
    }

	// allocate an index of triangles
	triIndex = (mesh_t*)malloc(sizeof(mesh_t) * modelHeader->numTris);

	// set total number of triangles
	mModel->numTriangles = modelHeader->numTris;
	mModel->triIndex = triIndex;
	
	// point to triangle indexes in buffer
	bufIndexPtr = (mesh_t*)&buffer[modelHeader->offsetTris];

	// create a mesh (triangle) list
	for (j = 0; j < mModel->numFrames; j++)		
	{
		// for all triangles in each frame
		for(i = 0; i < modelHeader->numTris; i++)
		{
		   triIndex[i].meshIndex[0] = bufIndexPtr[i].meshIndex[0];
		   triIndex[i].meshIndex[1] = bufIndexPtr[i].meshIndex[1];
		   triIndex[i].meshIndex[2] = bufIndexPtr[i].meshIndex[2];
		   triIndex[i].stIndex[0] = bufIndexPtr[i].stIndex[0];
		   triIndex[i].stIndex[1] = bufIndexPtr[i].stIndex[1];
		   triIndex[i].stIndex[2] = bufIndexPtr[i].stIndex[2];
		}
	}

	// close file and free memory
	//fclose(filePtr);
    free(buffer);

	mModel->currentFrame = 0;
	mModel->nextFrame = 1;
	mModel->interpol = 0.0;

	CheckNextState();

    return true;
}


//-------------------------------------------------------------------------------------------------
// given 3 points, calculates the normal to the points
#if defined (WIN32) || defined (LINUX)
void JMD2Model::CalculateNormal(float *p1, float *p2, float *p3)
#else
void JMD2Model::CalculateNormal(ScePspFVector3 *normal, float *p1, float *p2, float *p3)
#endif
{
   float a[3], b[3], result[3];
   float length;

   a[0] = p1[0] - p2[0];
   a[1] = p1[1] - p2[1];
   a[2] = p1[2] - p2[2];

   b[0] = p1[0] - p3[0];
   b[1] = p1[1] - p3[1];
   b[2] = p1[2] - p3[2];

   result[0] = a[1] * b[2] - b[1] * a[2];
   result[1] = b[0] * a[2] - a[0] * b[2];
   result[2] = a[0] * b[1] - b[0] * a[1];

   // calculate the length of the normal
   length = (float)sqrt(result[0]*result[0] + result[1]*result[1] + result[2]*result[2]);

#if defined (WIN32) || defined (LINUX)
   // normalize and specify the normal
#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
   glNormal3f(result[0]/length, result[1]/length, result[2]/length);
#else
   // FIXME
#endif //(!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)

#else
   if (length == 0.0f)
	   length = SMALLEST_FP;

   normal->x = result[0]/length;
   normal->y = result[1]/length;
   normal->z = result[2]/length;
#endif

}


//-------------------------------------------------------------------------------------------------
// render a single frame of a MD2 model
void JMD2Model::Render(int frameNum)
{
	Vector3D *pointList;
	int i;

    // create a pointer to the frame we want to show
    pointList = &mModel->pointList[mModel->numPoints * frameNum];

	// set the texture
	mRenderer->BindTexture(mModel->modelTex);


#if defined (WIN32) || defined (LINUX)

	// display the textured model with proper lighting normals
#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

#elif (defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#else
        glBegin(GL_TRIANGLES);
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

	for(i = 0; i < mModel->numTriangles; i++)
	{
		CalculateNormal(pointList[mModel->triIndex[i].meshIndex[0]].v,
			pointList[mModel->triIndex[i].meshIndex[2]].v,
			pointList[mModel->triIndex[i].meshIndex[1]].v);

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

#elif (defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)
                float vertex_data[]={
                pointList[mModel->triIndex[i].meshIndex[0]].x, pointList[mModel->triIndex[i].meshIndex[0]].y, pointList[mModel->triIndex[i].meshIndex[0]].z,
                pointList[mModel->triIndex[i].meshIndex[2]].x, pointList[mModel->triIndex[i].meshIndex[2]].y, pointList[mModel->triIndex[i].meshIndex[2]].z,
                pointList[mModel->triIndex[i].meshIndex[1]].x, pointList[mModel->triIndex[i].meshIndex[1]].y, pointList[mModel->triIndex[i].meshIndex[1]].z,
                };
                float texcoord_data[] = {
                    mModel->st[mModel->triIndex[i].stIndex[0]].s,
                    mModel->st[mModel->triIndex[i].stIndex[0]].t,
                    mModel->st[mModel->triIndex[i].stIndex[2]].s,
                    mModel->st[mModel->triIndex[i].stIndex[2]].t,
                    mModel->st[mModel->triIndex[i].stIndex[1]].s,
                    mModel->st[mModel->triIndex[i].stIndex[1]].t,
                };

                glVertexPointer(3,GL_FLOAT,0,vertex_data);
                glTexCoordPointer(2,GL_FLOAT,0,texcoord_data);

#else
                glTexCoord2f(mModel->st[mModel->triIndex[i].stIndex[0]].s,
			mModel->st[mModel->triIndex[i].stIndex[0]].t);
		glVertex3fv(pointList[mModel->triIndex[i].meshIndex[0]].v);

		glTexCoord2f(mModel->st[mModel->triIndex[i].stIndex[2]].s ,
			mModel->st[mModel->triIndex[i].stIndex[2]].t);
		glVertex3fv(pointList[mModel->triIndex[i].meshIndex[2]].v);

		glTexCoord2f(mModel->st[mModel->triIndex[i].stIndex[1]].s,
			mModel->st[mModel->triIndex[i].stIndex[1]].t);
		glVertex3fv(pointList[mModel->triIndex[i].meshIndex[1]].v);
#endif //#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)
        }
#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

#elif (defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)
        glDrawArrays(GL_TRIANGLES,0,3); // seems suspicious to put that here, should probably be in the loop
#else
        glEnd();
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)


#else

	PSPVertex3D* vertices = (PSPVertex3D*) sceGuGetMemory(mModel->numTriangles * 3 * sizeof(PSPVertex3D));

		int n = 0;
		for(i = 0; i < mModel->numTriangles; i++)
        {
			//CalculateNormal(&vertices[n].normal,
			//				pointList[mModel->triIndex[i].meshIndex[0]].v,
            //                pointList[mModel->triIndex[i].meshIndex[2]].v,
            //                pointList[mModel->triIndex[i].meshIndex[1]].v);
			
			vertices[n].texture.x = mModel->st[mModel->triIndex[i].stIndex[0]].s;
			vertices[n].texture.y = mModel->st[mModel->triIndex[i].stIndex[0]].t;
			
			vertices[n].pos.x = pointList[mModel->triIndex[i].meshIndex[0]].x;
			vertices[n].pos.y = pointList[mModel->triIndex[i].meshIndex[0]].y;
			vertices[n].pos.z = pointList[mModel->triIndex[i].meshIndex[0]].z;
			n++;

			//vertices[n].normal.x = vertices[n-1].normal.x;
			//vertices[n].normal.y = vertices[n-1].normal.y;
			//vertices[n].normal.z = vertices[n-1].normal.z;

			vertices[n].texture.x = mModel->st[mModel->triIndex[i].stIndex[2]].s;
			vertices[n].texture.y = mModel->st[mModel->triIndex[i].stIndex[2]].t;

			vertices[n].pos.x = pointList[mModel->triIndex[i].meshIndex[2]].x;
			vertices[n].pos.y = pointList[mModel->triIndex[i].meshIndex[2]].y;
			vertices[n].pos.z = pointList[mModel->triIndex[i].meshIndex[2]].z;
			n++;
            
			
			//vertices[n].normal.x = vertices[n-1].normal.x;
			//vertices[n].normal.y = vertices[n-1].normal.y;
			//vertices[n].normal.z = vertices[n-1].normal.z;
			
			vertices[n].texture.x = mModel->st[mModel->triIndex[i].stIndex[1]].s;
			vertices[n].texture.y = mModel->st[mModel->triIndex[i].stIndex[1]].t;

			vertices[n].pos.x = pointList[mModel->triIndex[i].meshIndex[1]].x;
			vertices[n].pos.y = pointList[mModel->triIndex[i].meshIndex[1]].y;
			vertices[n].pos.z = pointList[mModel->triIndex[i].meshIndex[1]].z;
			n++;

            
        }

		
		sceGuColor(0xff000000);
		sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_NORMAL_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_3D,mModel->numTriangles*3,0,vertices);
		

#endif

}


//-------------------------------------------------------------------------------------------------
void JMD2Model::SetState(int newState)
{
	mNextState = newState;

}


//-------------------------------------------------------------------------------------------------
void JMD2Model::CheckNextState()
{
	if (mState != mNextState)
	{
		mState = mNextState;
		mModel->currentFrame = mAnimations[mState]->mStartFrame;
		mModel->nextFrame = mAnimations[mState]->mStartFrame+1;
		mModel->interpol = 0.0f;
	}
}


void JMD2Model::Update(float dt)
{

	mModel->interpol += mAnimationSpeed*dt;

	if (mModel->interpol >= 1.0)
	{
		mModel->interpol = 0.0f;

		int startFrame = mAnimations[mState]->mStartFrame;
		int endFrame = mAnimations[mState]->mEndFrame;

		if ( (startFrame < 0) || (endFrame < 0) )
			return;

		if ( (startFrame >= mModel->numFrames) || (endFrame >= mModel->numFrames) )
			return;

		mModel->currentFrame++;
		if (mModel->currentFrame >= endFrame)
			mModel->currentFrame = startFrame;

		mModel->nextFrame = mModel->currentFrame + 1;

		if (mModel->nextFrame >= endFrame)
		{
			mModel->nextFrame = startFrame;
			CheckNextState();
		}
	}

}


void JMD2Model::SetAnimationSpeed(float speed)
{
	mAnimationSpeed = speed;
}


//-------------------------------------------------------------------------------------------------
// displays a frame of the model between startFrame and endFrame with an interpolation percent
void JMD2Model::Render()
{

	CheckNextState();

	Vector3D *pointList;			// current frame vertices
	Vector3D *nextPointList;		// next frame vertices
	int i;						
	float x1, y1, z1;				// current frame point values
	float x2, y2, z2;				// next frame point values

	Vector3D vertex[3];	

	if (mModel == NULL)
		return;
	
	pointList = &mModel->pointList[mModel->numPoints*mModel->currentFrame];
	nextPointList = &mModel->pointList[mModel->numPoints*mModel->nextFrame];
	
	mRenderer->BindTexture(mModel->modelTex);

#if defined (WIN32) || defined (LINUX)

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
        // FIXME
#elif (defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#else
        glBegin(GL_TRIANGLES);
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

        for (i = 0; i < mModel->numTriangles; i++)
	{
		// get first points of each frame
		x1 = pointList[mModel->triIndex[i].meshIndex[0]].x;
		y1 = pointList[mModel->triIndex[i].meshIndex[0]].y;
		z1 = pointList[mModel->triIndex[i].meshIndex[0]].z;
		x2 = nextPointList[mModel->triIndex[i].meshIndex[0]].x;
		y2 = nextPointList[mModel->triIndex[i].meshIndex[0]].y;
		z2 = nextPointList[mModel->triIndex[i].meshIndex[0]].z;

		// store first interpolated vertex of triangle
		vertex[0].x = x1 + mModel->interpol * (x2 - x1);
		vertex[0].y = y1 + mModel->interpol * (y2 - y1);
		vertex[0].z = z1 + mModel->interpol * (z2 - z1);

		// get second points of each frame
		x1 = pointList[mModel->triIndex[i].meshIndex[2]].x;
		y1 = pointList[mModel->triIndex[i].meshIndex[2]].y;
		z1 = pointList[mModel->triIndex[i].meshIndex[2]].z;
		x2 = nextPointList[mModel->triIndex[i].meshIndex[2]].x;
		y2 = nextPointList[mModel->triIndex[i].meshIndex[2]].y;
		z2 = nextPointList[mModel->triIndex[i].meshIndex[2]].z;

		// store second interpolated vertex of triangle
		vertex[2].x = x1 + mModel->interpol * (x2 - x1);
		vertex[2].y = y1 + mModel->interpol * (y2 - y1);
		vertex[2].z = z1 + mModel->interpol * (z2 - z1);	

		// get third points of each frame
		x1 = pointList[mModel->triIndex[i].meshIndex[1]].x;
		y1 = pointList[mModel->triIndex[i].meshIndex[1]].y;
		z1 = pointList[mModel->triIndex[i].meshIndex[1]].z;
		x2 = nextPointList[mModel->triIndex[i].meshIndex[1]].x;
		y2 = nextPointList[mModel->triIndex[i].meshIndex[1]].y;
		z2 = nextPointList[mModel->triIndex[i].meshIndex[1]].z;

		// store third interpolated vertex of triangle
		vertex[1].x = x1 + mModel->interpol * (x2 - x1);
		vertex[1].y = y1 + mModel->interpol * (y2 - y1);
		vertex[1].z = z1 + mModel->interpol * (z2 - z1);

		// calculate the normal of the triangle
		//CalculateNormal(vertex[0].v, vertex[2].v, vertex[1].v);

		// render properly textured triangle

#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
        // FIXME
#elif (defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)
                float vertex_data[]={
                vertex[0].x, vertex[0].y, vertex[0].z,
                vertex[2].x, vertex[2].y, vertex[2].z,
                vertex[1].x, vertex[1].y, vertex[1].z,
                };
                float texcoord_data[] = {
                    mModel->st[mModel->triIndex[i].stIndex[0]].s,
                    mModel->st[mModel->triIndex[i].stIndex[0]].t,
                    mModel->st[mModel->triIndex[i].stIndex[2]].s,
                    mModel->st[mModel->triIndex[i].stIndex[2]].t,
                    mModel->st[mModel->triIndex[i].stIndex[1]].s,
                    mModel->st[mModel->triIndex[i].stIndex[1]].t,
                };

                glVertexPointer(3,GL_FLOAT,0,vertex_data);
                glTexCoordPointer(2,GL_FLOAT,0,texcoord_data);
#else
                glTexCoord2f(mModel->st[mModel->triIndex[i].stIndex[0]].s,
                        mModel->st[mModel->triIndex[i].stIndex[0]].t);
                glVertex3fv(vertex[0].v);

                glTexCoord2f(mModel->st[mModel->triIndex[i].stIndex[2]].s ,
                        mModel->st[mModel->triIndex[i].stIndex[2]].t);
                glVertex3fv(vertex[2].v);

                glTexCoord2f(mModel->st[mModel->triIndex[i].stIndex[1]].s,
                        mModel->st[mModel->triIndex[i].stIndex[1]].t);
                glVertex3fv(vertex[1].v);
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

        }
#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
        // FIXME
#elif (defined GL_ES_VERSION_1_1) || (defined GL_VERSION_1_1)
        glDrawArrays(GL_TRIANGLES,0,3); // seems suspicious to put that here, should probably be in the loop
#else
        glEnd();
#endif //(defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)

#else

	PSPVertex3D* vertices = (PSPVertex3D*) sceGuGetMemory(mModel->numTriangles * 3 * sizeof(PSPVertex3D));

	int n = 0;
		for (i = 0; i < mModel->numTriangles; i++)
		{
			// get first points of each frame
			x1 = pointList[mModel->triIndex[i].meshIndex[0]].x;
			y1 = pointList[mModel->triIndex[i].meshIndex[0]].y;
			z1 = pointList[mModel->triIndex[i].meshIndex[0]].z;
			x2 = nextPointList[mModel->triIndex[i].meshIndex[0]].x;
			y2 = nextPointList[mModel->triIndex[i].meshIndex[0]].y;
			z2 = nextPointList[mModel->triIndex[i].meshIndex[0]].z;

			// store first interpolated vertex of triangle
			vertex[0].x = x1 + mModel->interpol * (x2 - x1);
			vertex[0].y = y1 + mModel->interpol * (y2 - y1);
			vertex[0].z = z1 + mModel->interpol * (z2 - z1);
		
			// get second points of each frame
			x1 = pointList[mModel->triIndex[i].meshIndex[2]].x;
			y1 = pointList[mModel->triIndex[i].meshIndex[2]].y;
			z1 = pointList[mModel->triIndex[i].meshIndex[2]].z;
			x2 = nextPointList[mModel->triIndex[i].meshIndex[2]].x;
			y2 = nextPointList[mModel->triIndex[i].meshIndex[2]].y;
			z2 = nextPointList[mModel->triIndex[i].meshIndex[2]].z;

			// store second interpolated vertex of triangle
			vertex[2].x = x1 + mModel->interpol * (x2 - x1);
			vertex[2].y = y1 + mModel->interpol * (y2 - y1);
			vertex[2].z = z1 + mModel->interpol * (z2 - z1);	
	
			// get third points of each frame
			x1 = pointList[mModel->triIndex[i].meshIndex[1]].x;
			y1 = pointList[mModel->triIndex[i].meshIndex[1]].y;
			z1 = pointList[mModel->triIndex[i].meshIndex[1]].z;
			x2 = nextPointList[mModel->triIndex[i].meshIndex[1]].x;
			y2 = nextPointList[mModel->triIndex[i].meshIndex[1]].y;
			z2 = nextPointList[mModel->triIndex[i].meshIndex[1]].z;

			// store third interpolated vertex of triangle
			vertex[1].x = x1 + mModel->interpol * (x2 - x1);
			vertex[1].y = y1 + mModel->interpol * (y2 - y1);
			vertex[1].z = z1 + mModel->interpol * (z2 - z1);


			//CalculateNormal(&vertices[n].normal,
			//				vertex[0].v,
            //                vertex[2].v,
            //                vertex[1].v);
			
			vertices[n].texture.x = mModel->st[mModel->triIndex[i].stIndex[0]].s;
			vertices[n].texture.y = mModel->st[mModel->triIndex[i].stIndex[0]].t;
			
			vertices[n].pos.x = vertex[0].x;
			vertices[n].pos.y = vertex[0].y;
			vertices[n].pos.z = vertex[0].z;
			n++;

			//vertices[n].normal.x = vertices[n-1].normal.x;
			//vertices[n].normal.y = vertices[n-1].normal.y;
			//vertices[n].normal.z = vertices[n-1].normal.z;

			vertices[n].texture.x = mModel->st[mModel->triIndex[i].stIndex[2]].s;
			vertices[n].texture.y = mModel->st[mModel->triIndex[i].stIndex[2]].t;

			vertices[n].pos.x = vertex[2].x;
			vertices[n].pos.y = vertex[2].y;
			vertices[n].pos.z = vertex[2].z;
			n++;
            
			
			//vertices[n].normal.x = vertices[n-1].normal.x;
			//vertices[n].normal.y = vertices[n-1].normal.y;
			//vertices[n].normal.z = vertices[n-1].normal.z;
			
			vertices[n].texture.x = mModel->st[mModel->triIndex[i].stIndex[1]].s;
			vertices[n].texture.y = mModel->st[mModel->triIndex[i].stIndex[1]].t;

			vertices[n].pos.x = vertex[1].x;
			vertices[n].pos.y = vertex[1].y;
			vertices[n].pos.z = vertex[1].z;
			n++;

		}

	sceGuColor(0xff000000);
	sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_3D,mModel->numTriangles*3,0,vertices);

#endif

//	mModel->interpol += percent;	// increase percentage of interpolation between frames
}

