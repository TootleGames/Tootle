/*------------------------------------------------------
	
	opengl render target

-------------------------------------------------------*/
#pragma once

#include "../TRenderTarget.h"
#include <TootleCore/TLMaths.h>


namespace TLAsset
{
	class TMesh;
}

namespace TLRender
{
	namespace Platform
	{
		class RenderTarget;
	}

	class TScreen;
};



//---------------------------------------------------------
//	opengl render target
//---------------------------------------------------------
class TLRender::Platform::RenderTarget : public TLRender::TRenderTarget
{
public:
	RenderTarget(const TRef& Ref=TRef());

	virtual Bool			BeginDraw(const Type4<s32>& MaxSize,const TScreen& Screen);

	//	rendering controls
	virtual void			BeginScene();								//	save off current scene 
	virtual void			BeginSceneReset(Bool ApplyCamera=TRUE);		//	save off current scene and reset camera 
	virtual void			EndScene();									//	restore previous scene

protected:
	virtual Bool			BeginProjectDraw(TLRender::TProjectCamera* pCamera,TLRender::TScreenShape ScreenShape);	//	setup projection mode
	virtual void			EndProjectDraw();
	virtual Bool			BeginOrthoDraw(TLRender::TOrthoCamera* pCamera,TLRender::TScreenShape ScreenShape);		//	setup ortho projection mode
	virtual void			EndOrthoDraw();
};



