/*
 *	Copyright (C) 2006 cooleyes
 *	eyes.cooleyes@gmail.com 
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
 
#include "../include/decoder_prx.h"
 
char *prx_static_init()
	{
	int result;
#ifdef DEVHOOK
#ifdef PSPFW3XX 
	result = sceUtilityLoadAvModule(0); 
#else
	result = pspSdkLoadStartModule("flash0:/kd/avcodec.prx", PSP_MEMORY_PARTITION_KERNEL);
#endif
	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on avcodec.prx");
		}
#else
	result = pspSdkLoadStartModule("flash0:/kd/me_for_vsh.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on me_for_vsh.prx");
		}
	result = pspSdkLoadStartModule("flash0:/kd/audiocodec.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on audiocodec.prx");
		}


	result = pspSdkLoadStartModule("flash0:/kd/videocodec.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on videocodec.prx");
		}


	result = pspSdkLoadStartModule("flash0:/kd/mpegbase.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on mpegbase.prx");
		}
#endif

#ifdef PSPFW3XX
	result = sceUtilityLoadAvModule(3);
#else
	result = pspSdkLoadStartModule("flash0:/kd/mpeg_vsh.prx", PSP_MEMORY_PARTITION_USER);
#endif
	if (result < 0)
		{
		return("prx_static_init: pspSdkLoadStartModule failed on mpeg_vsh.prx");
		}


#ifndef DEVHOOK
	pspSdkFixupImports(result);
#endif

	
	return(0);
	}
