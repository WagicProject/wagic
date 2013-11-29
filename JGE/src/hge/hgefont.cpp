/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeFont helper class implementation
*/

#include "../../include/JGE.h"
#include "../../include/JTypes.h"
#include "../../include/JRenderer.h"
#include "../../include/JFileSystem.h"

#include "../../include/hge/hgefont.h"
#include <stdlib.h>
#include <stdio.h>

const char FNTHEADERTAG[] = "[HGEFONT]";
const char FNTBITMAPTAG[] = "Bitmap";
const char FNTCHARTAG[]   = "Char";


//HGE *hgeFont::hge=0;
char hgeFont::buffer[256];


hgeFont::hgeFont(const char *szFont, bool bMipmap __attribute__((unused)))
{
	//void	*data;
	DWORD	size;
	char	*desc, *pdesc;
	char	linebuf[256];
	char	buf[512], *pbuf;
	char	chr;
	int		i, x, y, w, h, a, c;

	// Setup variables

	//hge=hgeCreate(HGE_VERSION);

	fHeight=0.0f;
	fScale=1.0f;
	fProportion=1.0f;
	fRot=0.0f;
	fTracking=0.0f;
	fSpacing=1.0f;
	hTexture=0;

	fZ=0.5f;
	//nBlend=BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE;
	dwCol=ARGB(0xFF,0xFF,0xFF,0xFF);

	memset( &letters, 0, sizeof(letters) );
	memset( &pre, 0, sizeof(letters) );
	memset( &post, 0, sizeof(letters) );

	// Load font description

	JFileSystem* fileSys = JFileSystem::GetInstance();
	if (!fileSys->OpenFile(szFont)) return;

	//data=hge->Resource_Load(szFont, &size);
	//if(!data) return;
	size = fileSys->GetFileSize();

	desc = new char[size+1];
	//memcpy(desc,data,size);
	fileSys->ReadFile(desc, size);
	desc[size]=0;

	//hge->Resource_Free(data);
	fileSys->CloseFile();

	pdesc=_get_line(desc,linebuf);
	if(strcmp(linebuf, FNTHEADERTAG))
	{
//		hge->System_Log("Font %s has incorrect format.", szFont);
		delete[] desc;
		return;
	}

	// Parse font description

	JRenderer* renderer = JRenderer::GetInstance();

	while((pdesc = _get_line(pdesc,linebuf))!=NULL)
	{
		if(!strncmp(linebuf, FNTBITMAPTAG, sizeof(FNTBITMAPTAG)-1 ))
		{
			strcpy(buf,szFont);
			pbuf=strrchr(buf,'\\');
			if(!pbuf) pbuf=strrchr(buf,'/');
			if(!pbuf) pbuf=buf;
			else pbuf++;
			if(!sscanf(linebuf, "Bitmap = %s", pbuf)) continue;

			//hTexture=hge->Texture_Load(buf, 0, bMipmap);
			hTexture = renderer->LoadTexture(buf);
			if(!hTexture)
			{
				delete[] desc;
				return;
			}
		}

		else if(!strncmp(linebuf, FNTCHARTAG, sizeof(FNTCHARTAG)-1 ))
		{
			pbuf=strchr(linebuf,'=');
			if(!pbuf) continue;
			pbuf++;
			while(*pbuf==' ') pbuf++;
			if(*pbuf=='\"')
			{
				pbuf++;
				i=(unsigned char)*pbuf++;
				pbuf++; // skip "
			}
			else
			{
				i=0;
				while((*pbuf>='0' && *pbuf<='9') || (*pbuf>='A' && *pbuf<='F') || (*pbuf>='a' && *pbuf<='f'))
				{
					chr=*pbuf;
					if(chr >= 'a') chr-='a'-':';
					if(chr >= 'A') chr-='A'-':';
					chr-='0';
					if(chr>0xF) chr=0xF;
					i=(i << 4) | chr;
					pbuf++;
				}
				if(i<0 || i>255) continue;
			}
			sscanf(pbuf, " , %d , %d , %d , %d , %d , %d", &x, &y, &w, &h, &a, &c);

			letters[i] = new JQuad(hTexture, (float)x, (float)y, (float)w, (float)h);
			pre[i]=(float)a;
			post[i]=(float)c;
			if(h>fHeight) fHeight=(float)h;
		}
	}

	delete[] desc;
}


hgeFont::~hgeFont()
{
	for(int i=0; i<256; i++)
		if(letters[i]) delete letters[i];
	if(hTexture) delete hTexture;
	//hge->Release();
}

void hgeFont::Render(float x, float y, int align, const char *string)
{
	int i;
	float	fx=x;

	JRenderer* renderer = JRenderer::GetInstance();

	align &= HGETEXT_HORZMASK;
	if(align==HGETEXT_RIGHT) fx-=GetStringWidth(string);
	if(align==HGETEXT_CENTER) fx-=int(GetStringWidth(string)/2.0f);

	while(*string)
	{
		if(*string=='\n')
		{
			y += int(fHeight*fScale*fSpacing);
			fx = x;
			if(align == HGETEXT_RIGHT)  fx -= GetStringWidth(string+1);
			if(align == HGETEXT_CENTER) fx -= int(GetStringWidth(string+1)/2.0f);
		}
		else
		{
			i=(unsigned char)*string;
			if(!letters[i]) i='?';
			if(letters[i])
			{
				fx += pre[i]*fScale*fProportion;
				//letters[i]->RenderEx(fx, y, fRot, fScale*fProportion, fScale);
				renderer->RenderQuad(letters[i], fx, y, fRot, fScale*fProportion, fScale);
				fx += (letters[i]->mWidth+post[i]+fTracking)*fScale*fProportion;
			}
		}
		string++;
	}
}

void hgeFont::printf(float x, float y, int align, const char *format, ...)
{
	//char	*pArg=(char *) &format+sizeof(format);

	//_vsnprintf(buffer, sizeof(buffer)-1, format, pArg);
	//buffer[sizeof(buffer)-1]=0;
	//vsprintf(buffer, format, pArg);
	va_list list;

	va_start(list, format);
	vsprintf(buffer, format, list);
	va_end(list);

	Render(x,y,align,buffer);
}

void hgeFont::printfb(float x, float y, float w, float h, int align, const char *format, ...)
{
	char	chr, *pbuf, *prevword, *linestart;
	int		i,lines=0;
	float	tx, ty, hh, ww;
	//char	*pArg=(char *) &format+sizeof(format);

	//_vsnprintf(buffer, sizeof(buffer)-1, format, pArg);
	//buffer[sizeof(buffer)-1]=0;
	//vsprintf(buffer, format, pArg);

	va_list list;

	va_start(list, format);
	vsprintf(buffer, format, list);
	va_end(list);


	linestart=buffer;
	pbuf=buffer;
	prevword=0;

	for(;;)
	{
		i=0;
		while(pbuf[i] && pbuf[i]!=' ' && pbuf[i]!='\n') i++;

		chr=pbuf[i];
		pbuf[i]=0;
		ww=GetStringWidth(linestart);
		pbuf[i]=chr;

		if(ww > w)
		{
			if(pbuf==linestart)
			{
				pbuf[i]='\n';
				linestart=&pbuf[i+1];
			}
			else
			{
				*prevword='\n';
				linestart=prevword+1;
			}

			lines++;
		}

		if(pbuf[i]=='\n')
		{
			prevword=&pbuf[i];
			linestart=&pbuf[i+1];
			pbuf=&pbuf[i+1];
			lines++;
			continue;
		}

		if(!pbuf[i]) {lines++;break;}

		prevword=&pbuf[i];
		pbuf=&pbuf[i+1];
	}

	tx=x;
	ty=y;
	hh=fHeight*fSpacing*fScale*lines;

	switch(align & HGETEXT_HORZMASK)
	{
		case HGETEXT_LEFT: break;
		case HGETEXT_RIGHT: tx+=w; break;
		case HGETEXT_CENTER: tx+=int(w/2); break;
	}

	switch(align & HGETEXT_VERTMASK)
	{
		case HGETEXT_TOP: break;
		case HGETEXT_BOTTOM: ty+=h-hh; break;
		case HGETEXT_MIDDLE: ty+=int((h-hh)/2); break;
	}

	Render(tx,ty,align,buffer);
}

float hgeFont::GetStringWidth(const char *string) const
{
	int i;
	float linew, w = 0;

	while(*string)
	{
		linew = 0;

		while(*string && *string != '\n')
		{
			i=(unsigned char)*string;
			if(!letters[i]) i='?';
			if(letters[i])
				linew += letters[i]->mWidth + pre[i] + post[i] + fTracking;

			string++;
		}

		if(linew > w) w = linew;

		while (*string == '\n' || *string == '\r') string++;
	}

	return w*fScale*fProportion;
}

void hgeFont::SetColor(PIXEL_TYPE col)
{
	dwCol = col;

	for(int i=0; i<256; i++)
		if(letters[i])
			letters[i]->SetColor(col);
}

void hgeFont::SetZ(float z)
{
	fZ = z;

	//for(int i=0; i<256; i++)
	//	if(letters[i])
	//		letters[i]->SetZ(z);
}

void hgeFont::SetBlendMode(int blend)
{
	nBlend = blend;

	//for(int i=0; i<256; i++)
	//	if(letters[i])
	//		letters[i]->SetBlendMode(blend);
}

char *hgeFont::_get_line(char *file, char *line)
{
	int i=0;

	if(!file[i]) return 0;

	while(file[i] && file[i]!='\n' && file[i]!='\r')
	{
		line[i]=file[i];
		i++;
	}
	line[i]=0;

	while(file[i] && (file[i]=='\n' || file[i]=='\r')) i++;

	return file + i;
}
