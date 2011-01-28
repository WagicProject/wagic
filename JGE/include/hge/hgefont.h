/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeFont helper class header
*/


#ifndef HGEFONT_H
#define HGEFONT_H

#include "../JTypes.h"
//#include "hge.h"
//#include "hgesprite.h"


#define HGETEXT_LEFT		0
#define HGETEXT_RIGHT		1
#define HGETEXT_CENTER		2
#define HGETEXT_HORZMASK	0x03

#define HGETEXT_TOP			0
#define HGETEXT_BOTTOM		4
#define HGETEXT_MIDDLE		8
#define HGETEXT_VERTMASK	0x0C

class JTexture;
class JQuad;
/*
** HGE Font class
*/
class hgeFont
{
public:
	hgeFont(const char *filename, bool bMipmap=false);
	~hgeFont();

	void		Render(float x, float y, int align, const char *string);
	void		printf(float x, float y, int align, const char *format, ...);
	void		printfb(float x, float y, float w, float h, int align, const char *format, ...);

	void		SetColor(PIXEL_TYPE col);
	void		SetZ(float z);
	void		SetBlendMode(int blend);
	void		SetScale(float scale) {fScale=scale;}
	void		SetProportion(float prop) { fProportion=prop; }
	void		SetRotation(float rot) {fRot=rot;}
	void		SetTracking(float tracking) {fTracking=tracking;}
	void		SetSpacing(float spacing) {fSpacing=spacing;}

	PIXEL_TYPE	GetColor() const {return dwCol;}
	float		GetZ() const {return fZ;}
	int			GetBlendMode() const {return nBlend;}
	float		GetScale() const {return fScale;}
	float		GetProportion() const { return fProportion; }
	float		GetRotation() const {return fRot;}
	float		GetTracking() const {return fTracking;}
	float		GetSpacing() const {return fSpacing;}

	JQuad*		GetSprite(char chr) const { return letters[(unsigned char)chr]; }
	float		GetHeight() const { return fHeight; }
	float		GetStringWidth(const char *string) const;

private:
	hgeFont();
	hgeFont(const hgeFont &fnt);
	hgeFont&	operator= (const hgeFont &fnt);

	char*		_get_line(char *file, char *line);

	//static HGE	*hge;

	static char	buffer[256];

	JTexture*	hTexture;
	JQuad*		letters[256];
	float		pre[256];
	float		post[256];
	float		fHeight;
	float		fScale;
	float		fProportion;
	float		fRot;
	float		fTracking;
	float		fSpacing;

	PIXEL_TYPE	dwCol;
	float		fZ;
	int			nBlend;
};


#endif
