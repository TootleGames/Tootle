/*
 *  IPodScreen.h
 *  TootleRender
 *
 *  Created by Graham Reeves on 01/09/2008.
 *  Copyright 2008 Tootle. All rights reserved.
 *
 */

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


//	external forward declarations
namespace TLRender 
{
	class TScreen;

	//	internal forward declarations
	namespace Platform	
	{
		class Screen;
		class ScreenWideLeft;
		class ScreenWideRight;
	};
};




//----------------------------------------------------------
//	IPod screen
//----------------------------------------------------------
class TLRender::Platform::Screen : public TLRender::TScreen
{
public:
	Screen(TRefRef ScreenRef,TScreenShape ScreenShape);
	
	virtual SyncBool		Init();
	virtual SyncBool		Update();
	virtual SyncBool		Shutdown();

	virtual void			Draw();
	
protected:
};



//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWideLeft : public TLRender::Platform::Screen
{
public:
	ScreenWideLeft(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_WideLeft )
	{
	}
};


//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWideRight : public TLRender::Platform::Screen
{
public:
	ScreenWideRight(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_WideRight )
	{
	}
};


//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWide : public TLRender::Platform::Screen
{
public:
	ScreenWide(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_Wide )
	{
		//	swap height and width
		s16 h = m_Size.Height();
		s16 w = m_Size.Width();
		m_Size.Width() = h;
		m_Size.Height() = w;
	}
};

