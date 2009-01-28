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
};

using namespace TLRender;


//---------------------------------------------------------
//	opengl render target
//---------------------------------------------------------
class Platform::RenderTarget : public TRenderTarget
{
public:
	RenderTarget(const TRef& Ref=TRef());

	virtual Bool			BeginDraw(const Type4<s32>& MaxSize);

	//	rendering controls
	virtual void			BeginScene();										//	save off current scene 
	virtual void			BeginSceneReset(Bool ApplyCamera=TRUE);				//	save off current scene and reset camera 
	virtual void			EndScene();											//	restore previous scene

protected:
	virtual Bool			BeginProjectDraw(const Type4<s32>& ViewportSize);	//	setup projection mode
	virtual void			EndProjectDraw();

	virtual Bool			BeginOrthoDraw(const Type4<s32>& ViewportSize);		//	setup ortho projection mode
	virtual void			EndOrthoDraw();

	u32						GetCurrentMatrixMode();						//	fetch the current matrix mode
};



