#ifndef _JASSERT_H_
#define _JASSERT_H_

#include "JGE.h"

	#ifdef NDEBUG
		
		#define JASSERT(p)			((void)0)
		
	#else
	
		#ifdef WIN32
			#define JASSERT(e)		if (!(e)) { JGE::GetInstance()->Assert(__FILE__, __LINE__); }
			//#define JASSERT(e) 		if (!(e)) { _asm { int 3 }; }
			
		#else
			#define JASSERT(e) 		if (!(e)) { JGE::GetInstance()->Assert(__FILE__, __LINE__); }
			
		#endif
		
	#endif

#endif
