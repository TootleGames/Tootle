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
	virtual Bool			GetSceneMatrix(TLMaths::TMatrix& Matrix);			//	save off the current scene's camera matrix
	virtual void			EndScene();											//	restore previous scene
	virtual void			Translate(const TLMaths::TTransform& Transform);	//	create a transform/rot matrix from the quaternion and apply
	virtual void			SetSceneColour(const TColour& SceneColour);			//	set current render colour (glColourf)

protected:
	virtual TLRender::DrawResult	DrawMesh(TLAsset::TMesh& Mesh,const TRenderNode* pRenderNode,const TFlags<TRenderNode::RenderFlags::Flags>* pForceFlags);

	virtual Bool					BeginProjectDraw(const Type4<s32>& ViewportSize);	//	setup projection mode
	virtual void					EndProjectDraw();

	virtual Bool					BeginOrthoDraw(const Type4<s32>& ViewportSize);		//	setup ortho projection mode
	virtual void					EndOrthoDraw();

	u32								GetCurrentMatrixMode();						//	fetch the current matrix mode
};



