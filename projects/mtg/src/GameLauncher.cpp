//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------
#include "../include/debug.h"
#include <string.h>
#include <JGameLauncher.h>

#include "../include/GameApp.h"


//-------------------------------------------------------------------------------------
JApp* JGameLauncher::GetGameApp()
{
	return NEW GameApp();
};


//-------------------------------------------------------------------------------------
char *JGameLauncher::GetName()
{
        return strdup("Wagic");
}


//-------------------------------------------------------------------------------------
u32 JGameLauncher::GetInitFlags()
{
	return JINIT_FLAG_NORMAL;
}

