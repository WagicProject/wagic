//-------------------------------------------------------------------------------------
//
// JGE is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#ifndef _GAME_LAUNCHER_H_
#define _GAME_LAUNCHER_H_


#include "JApp.h"
#include "JTypes.h"
#include "JFileSystem.h"


//////////////////////////////////////////////////////////////////////////
/// An interface for JGE to get the user defined JApp class.
///  
//////////////////////////////////////////////////////////////////////////
class JGameLauncher
{
public:

	//////////////////////////////////////////////////////////////////////////
	/// Get user defined JApp instance. The function will be called when
	/// JGE starts.
	/// 
	/// @return - User defined JApp instance.
	/// 
	//////////////////////////////////////////////////////////////////////////
    static JApp* GetGameApp();

	//////////////////////////////////////////////////////////////////////////
	/// Get application name. Mainly for Windows build to setup the name
	/// on the title bar.
	/// 
	/// @return - Application name.
	///
	//////////////////////////////////////////////////////////////////////////
    static char *GetName();

	//////////////////////////////////////////////////////////////////////////
	/// Get initialization flags.
	/// 
	/// @return - Initialization flags.
	///
	//////////////////////////////////////////////////////////////////////////
    static u32 GetInitFlags();

};


#endif
