/*------------------------------------------------------

	Render object

-------------------------------------------------------*/
#pragma once

#include <TootleAsset/TShader.h>


namespace TLAsset
{
	class TAtlas;
}

namespace TLRender
{
	class TEffect;
	class TEffect_TextureAnimate;

	class TEffectFactory;

	extern TPtr<TEffectFactory>	g_pEffectFactory;
};


//---------------------------------------------------------------
//	factory to create effects
//---------------------------------------------------------------
class TLRender::TEffectFactory : public TClassFactory<TLRender::TEffect,false>
{
protected:
	virtual TLRender::TEffect*		CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};


//---------------------------------------------------------------
//	runtime shader interface. Overload this for specific features for a rendernode
//	eg. a sprite animator would use a texture-scrolling shader. 
//		an animated skeleton shaderinstance would use a skinning shader
//	gr: todo: turn this into an object from a factory
//---------------------------------------------------------------
class TLRender::TEffect
{
public:
	TEffect(const TArray<TRef>& ShaderCandidates);

	virtual bool	Initialise(TPtr<TBinaryTree>& pData);		//	set our data pointer and initialise vars from that data
	bool			IsValid() const					{	return (m_pShader && m_pShaderData);	}	//	can we use this shader instance?
	TBinaryTree&	GetShaderData()					{	return *m_pShaderData.GetObjectPointer();	}		//	

	virtual Bool	PreRender();					//	pre-render this shader. return false to abort render (eg. if the shader makes the object non-visible)
	virtual void	PostRender();					//	post-render of the shader

	FORCEINLINE bool	operator==(TRefRef EffectRef) const		{	return false;	}	//	gr: effects have no ref atm...

protected:
	TPtr<TLAsset::TShader>	m_pShader;				//	shader we're using
	TArray<TRef>			m_ShaderCandidates;		//	list of shader assets we can use for this in order of preference
	TPtr<TBinaryTree>		m_pShaderData;			//	shader data. this is usually "allocated" from the node data
};


//------------------------------------------------
class TLRender::TEffect_TextureAnimate : public TLRender::TEffect, public TLMessaging::TSubscriber
{
public:
	TEffect_TextureAnimate();

	virtual bool	Initialise(TPtr<TBinaryTree>& pData);		//	set our data pointer and initialise vars from that data
	bool			Update(float Timestep);
	void			SetTransform(const TLMaths::TTransform& Transform)		{	Transform.ReplaceData( GetShaderData() );	}

protected:
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);
	virtual TRefRef	GetSubscriberRef() const							{	static TRef Ref("FxTxAnim");	return Ref;	}
	void			OnAtlasChanged(const TLAsset::TAtlas& Atlas);		//	update frame transforms

	void			SetFrame(u32 Frame)					{	SetTransform( m_Frames[Frame] );	}
	u32				GetFrameCount() const				{	return m_Frames.GetSize();	}

protected:
	TArray<TLMaths::TTransform>	m_Frames;		//	frames in the atlas to cycle through
	float						m_Time;			//	current time counter
	float						m_FrameRate;	//	change frame every N amount (frames per second)
};

