/*------------------------------------------------

	Base class for creating/controlling win32 windows

-------------------------------------------------*/
#pragma once

#include "MacWinControl.h"


using namespace TLRender::Platform;

namespace Win32
{
	class GWindow;
	class GOpenglWindow;
};


class Win32::GWindow : public Win32::GWinControl
{
public:
	GWindow(TRefRef InstanceRef) : GWinControl	(InstanceRef)			{}
	~GWindow()					{}

	virtual u32					DefaultFlags()						{	return GWinControlFlags::ClientEdge|GWinControlFlags::OverlappedWindow;	};

	virtual const char*			ClassName()							{	return "Window";	};
	virtual Bool				Init(TPtr<GWinControl>& pOwner, u32 Flags);	//	window is overloaded to create class
};




class Win32::GOpenglWindow : public Win32::GWindow
{
public:
	HDC				m_HDC;
	HGLRC			m_HGLRC;
	Bool			m_HasArbMultiSample;

public:
	GOpenglWindow(TRefRef InstanceRef);
	~GOpenglWindow()	{}
	
	virtual const char*		ClassName()				{	return "OpenGL";	};	//	what class this window creates
	virtual Bool			Init(TPtr<GWinControl>& pOwner, u32 Flags);	//	overloaded to create rendering stuff
	virtual Bool			OnEraseBackground()		{	return FALSE;	};		//	dont erase backgroud over the opegnl display (stops WM_PAINT flicker)
	Bool					HasArbMultiSample()		{	return m_HasArbMultiSample;	}

private:
	Bool					InitDisplay();
	void					ShutdownDisplay();
};

