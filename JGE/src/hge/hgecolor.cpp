/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeColor*** helper classes implementation
*/


#include "../../include/hge/hgecolor.h"
#include <math.h>

#ifndef min
	#define min(x, y) ((x<y)?x:y)
#endif

#ifndef max
	#define max(x, y) ((x>y)?x:y)
#endif

void hgeColorHSV::SetHWColor(DWORD col)
{
	float r, g, b;
	float minv, maxv, delta;
	float del_R, del_G, del_B;

	a = (col>>24) / 255.0f;
	r = ((col>>16) & 0xFF) / 255.0f;
	g = ((col>>8)  & 0xFF) / 255.0f;
	b = (col       & 0xFF) / 255.0f;

	minv = min(min(r, g), b);
	maxv = max(max(r, g), b);
	delta = maxv - minv;

	v = maxv;

	if (delta == 0)
	{
		h = 0;
		s = 0;
	}
	else
	{
		s = delta / maxv;
		del_R = (((maxv - r) / 6) + (delta / 2)) / delta;
		del_G = (((maxv - g) / 6) + (delta / 2)) / delta;
		del_B = (((maxv - b) / 6) + (delta / 2)) / delta;

		if      (r == maxv) {h = del_B - del_G;}
		else if (g == maxv) {h = (1 / 3) + del_R - del_B;}
		else if (b == maxv) {h = (2 / 3) + del_G - del_R;}
		
		if (h < 0) h += 1;
		if (h > 1) h -= 1;
	}
}

DWORD hgeColorHSV::GetHWColor() const
{
	float r, g, b;
	float xh, i, p1, p2, p3;

	if (s == 0)
	{
		r = v;
		g = v;
		b = v;
	}
	else
	{
		xh = h * 6;
		if(xh == 6) xh=0;
		i = floorf(xh);
		p1 = v * (1 - s);
		p2 = v * (1 - s * (xh - i));
		p3 = v * (1 - s * (1 - (xh - i)));

		if      (i == 0) {r = v;  g = p3; b = p1;}
		else if (i == 1) {r = p2; g = v;  b = p1;}
		else if (i == 2) {r = p1; g = v;  b = p3;}
		else if (i == 3) {r = p1; g = p2; b = v; }
		else if (i == 4) {r = p3; g = p1; b = v; }
		else			 {r = v;  g = p1; b = p2;}
	}

	return (ARGB((int)(a*255.0f), (int)(r*255.0f), (int)(g*255.0f), (int)(b*255.0f)));
}

