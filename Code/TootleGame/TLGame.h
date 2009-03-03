/*------------------------------------------------------

	Mostly misc functions/class useful for games

-------------------------------------------------------*/

#include <TootleCore/TSubscriber.h>
#include <TootleMaths/TLine.h>

namespace TLRender
{
	class TRenderTarget;
}

namespace TLAsset
{
	class TMesh;
}

namespace TLGame
{
	class TScreenRayTest;

};



//-------------------------------------------------------
//	class to test the project rays
//-------------------------------------------------------
class TLGame::TScreenRayTest : public TLMessaging::TSubscriber
{
public:
	TScreenRayTest(TRefRef RenderTargetRef);
	~TScreenRayTest();							//	delete mesh and render node created

	virtual void			ProcessMessage(TLMessaging::TMessage& Message);	//	
	Bool					SetScreenPos(const Type2<s32>& ScreenPos);				//	screen pos has changed, get new projection Ray and update mesh

protected:
	Bool					CreateRenderNode(TPtr<TLRender::TRenderTarget>& pRenderTarget);							//	creates the mesh and render node if they dont exist
	void					SetMeshLine(const TLMaths::TLine& Line);	//	update the line on the mesh to match our ray

protected:
	TRef					m_RenderTargetRef;
	TRef					m_RenderNodeRef;
	TPtr<TLAsset::TMesh>	m_pMesh;
};

