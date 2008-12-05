/*------------------------------------------------------
 
	screen implementation on IPod - opengl ES
 
-------------------------------------------------------*/
#pragma once

#include "../TScreen.h"

/*
#define USE_FIXED_POINT
 
#ifdef USE_FIXED_POINT
  #define Float2Fixed(fl) ((GLfixed)((fl)*65536.0f))
  #define Fixed2Float(fx) ((float)((fx)/65536.0f))
#else
  #define Float2Fixed(fl) (fl)
  #define Fixed2Float(fx) (fx)
#endif
*/


namespace TLRender 
{
	namespace Platform
	{
		class Screen;
	};

};




//----------------------------------------------------------
//	IPod screen
//----------------------------------------------------------
class TLRender::Platform::Screen : public TLRender::TScreen
{
public:
	Screen(TRefRef Ref);
	
	virtual SyncBool		Init();
	virtual SyncBool		Update();
	virtual SyncBool		Shutdown();
	
	virtual void			Draw();
	
protected:
};




