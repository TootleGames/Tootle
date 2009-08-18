
#pragma once

namespace TLRender
{

	enum TScreenShape
	{
		ScreenShape_Portrait = 0,	//	portrait
		ScreenShape_WideLeft,		//	widescreen, portrait but rendered sideways to the left
		ScreenShape_WideRight,		//	widescreen, portrait but rendered sideways to the right
		ScreenShape_Wide,			//	widescreen
	};
	
	FORCEINLINE const TLMaths::TAngle&	GetScreenAngle(TScreenShape ScreenShape);	//	get the angle that the screenshape is rotated to


}



//---------------------------------------------------
//	get the angle that the screenshape is rotated to
//---------------------------------------------------
FORCEINLINE const TLMaths::TAngle& TLRender::GetScreenAngle(TScreenShape ScreenShape)
{
	static TLMaths::TAngle g_Angles[3] = 
	{
		0.f,	//	Portrait
		-90.f,	//	Left
		90.f,	//	Right
	};
	
	return g_Angles[ ScreenShape ];
}