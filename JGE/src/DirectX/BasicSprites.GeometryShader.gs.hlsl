// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//----------------------------------------------------------------------

cbuffer RenderTargetInfoCB
{
    float2 renderTargetSize;
};

cbuffer MvpMatrixCB
{
    float4x4 mvpMatrix;
};

struct GeometryShaderInput
{
    float2 origin : TRANSFORM0;
    float2 offset : TRANSFORM1;
    float rotation : TRANSFORM2;
	float4 textRect : TRANSFORM3;
	vector<unsigned int, 4> colors : COLOR0;
	float4 pt_x: TRANSFORM4;
	float4 pt_y: TRANSFORM5;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 color : COLOR0;
};


void rotate(float4x4 matrixxx, float angle, float3 origin)
{

}

// This shader generates two triangles that will be used to draw the sprite.
// The vertex properties are calculated based on the per-sprite instance data
// passed in from the vertex shader.

[maxvertexcount(4)]
void main(point GeometryShaderInput input[1], inout TriangleStream<PixelShaderInput> spriteStream)
{
    float sinRotation;
    float cosRotation;
    sincos(input[0].rotation, sinRotation, cosRotation);

    float2 texCoord[4];
	
    texCoord[0] = float2(input[0].textRect[0], input[0].textRect[2]);
    texCoord[1] = float2(input[0].textRect[1], input[0].textRect[2]);
    texCoord[2] = float2(input[0].textRect[0], input[0].textRect[3]);
    texCoord[3] = float2(input[0].textRect[1], input[0].textRect[3]);
	
    float2 posDelta[4];

	posDelta[0] = float2(0.0f,					0.0f);
    posDelta[1] = float2( 2*input[0].offset.x,  0.0f);
    posDelta[2] = float2(0.0f,					-2*input[0].offset.y);
    posDelta[3] = float2( 2*input[0].offset.x,	-2*input[0].offset.y);
	/*
	posDelta[0] = float2(-input[0].offset.x,	input[0].offset.y);
    posDelta[1] = float2( input[0].offset.x,	input[0].offset.y);
    posDelta[2] = float2(-input[0].offset.x,	-input[0].offset.y);
    posDelta[3] = float2( input[0].offset.x,	-input[0].offset.y);
	*/
	spriteStream.RestartStrip();
    [unroll]
    for (int i = 0; i < 4; i++)
    {
		float4 colorFloat;
		colorFloat[3] =	(float)((input[0].colors[i] & 0xFF000000) >> 24) / (float)255;
		colorFloat[0] =	(float)((input[0].colors[i] & 0x00FF0000) >> 16) / (float)255;
		colorFloat[1] =	(float)((input[0].colors[i] & 0x0000FF00) >> 8) / (float)255;
		colorFloat[2] =	(float)((input[0].colors[i] & 0x000000FF) ) / (float)255;

        posDelta[i] = float2(
            posDelta[i].x * cosRotation - posDelta[i].y * sinRotation,
            posDelta[i].x * sinRotation + posDelta[i].y * cosRotation
            );
        posDelta[i] /= renderTargetSize;
        PixelShaderInput streamElement;
        streamElement.pos = float4(input[0].origin + posDelta[i], 0.5f, 1.0f);
        streamElement.tex = texCoord[i];
        streamElement.color = colorFloat;
        spriteStream.Append(streamElement);
    }
    spriteStream.RestartStrip();
}
