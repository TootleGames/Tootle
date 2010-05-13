#include "IPodRender.h"
#include "../TLRender.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/EAGLDrawable.h>

#include <TootleAsset/TMesh.h>
#include <TootleCore/TColour.h>
#include <TootleCore/TKeyArray.h>

//---------------------------------------------------
//	globals
//---------------------------------------------------
namespace TLRender
{
	namespace Opengl
	{
		TPtr<TLAsset::TAssetFactory>	g_pShaderAssetFactory;
	}
}



//---------------------------------------------------
//	opengl init
//---------------------------------------------------
SyncBool TLRender::Opengl::Platform::Init()
{	
	//	gr: should check for an active context here (the caller should have created 
	//		a context and made it current. Otherwise the commands below will fail
	
	//	init opengl state
	glDisable( GL_CULL_FACE );
	glEnable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );
	
	//	setup opengl stuff
	glDisable( GL_LIGHTING );
	
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
		
	//	fastest for debug line/points
	glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	
	//	make perspective correction good (mostly for textures)
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	//	setup the texture alpha blending mode
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
		
	
	TLDebug_Print( TString("Device %s", glGetString( GL_RENDERER ) ) );
	TLDebug_Print( TString("Driver %s %s", glGetString( GL_VENDOR ), glGetString( GL_VERSION ) ) );
	
	Debug_CheckForError();		
	
	//	create fixed function shader asset factory
	if ( TLAsset::g_pManager )
	{
		g_pShaderAssetFactory = new TLRender::Platform::TShaderAssetFactory;
		TLAsset::g_pManager->AddAssetFactory( g_pShaderAssetFactory );
		
		//	pre-create assets
		//	gr: change this to actually create the assets and then add them to the asset directory
		TPtr<TLAsset::TAsset>& pShader = TLAsset::CreateAsset( TLAsset::TShader_TextureMatrix::GetShaderInstanceRef(), TLAsset::TShader::GetAssetType_Static() );
		if ( pShader )
			pShader->SetLoadingState( TLAsset::LoadingState_Loaded );
	}
	else
	{
		TLDebug_Break("cannot create fixed function shader assets as asset manager doesn't yet exist");
	}
	
	
	//	all succeeded
	return SyncTrue;
}


//---------------------------------------------------
//	opengl shutdown
//---------------------------------------------------
SyncBool TLRender::Opengl::Platform::Shutdown()
{
	//	gr: any shutdown required here? just need the opengl canvas's to free their contexts....
	//	maybe we need a ref counter to make sure there are no more context's left
	
	//	back to uninitialised gl state
	// [13/05/10] DB - not used on the ipod?  it's on the MAc and Win32 versions
	//g_Initialised = SyncWait;

	g_pShaderAssetFactory = NULL;
		
	return SyncTrue;
}



//---------------------------------------------------
//	platform/opengl initialisation
//---------------------------------------------------
SyncBool TLRender::Platform::Init()
{
	//	setup default states
	glEnable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
	
	
	return SyncTrue;
}



//---------------------------------------------------
//	platform/opengl shutdown
//---------------------------------------------------
SyncBool TLRender::Platform::Shutdown()	
{
	return SyncTrue;
}



//-------------------------------------------------------
//	custom shader creation
//-------------------------------------------------------
TLAsset::TAsset* TLRender::Platform::TShaderAssetFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	//	specifically create texture matrix shader
	if ( InstanceRef == TLAsset::TShader_TextureMatrix::GetShaderInstanceRef() && TypeRef == TLAsset::TShader::GetAssetType_Static() )
		return new TLAsset::TShader_TextureMatrix( InstanceRef );
	
	return NULL;
}


TLAsset::TShader_TextureMatrix::TShader_TextureMatrix(TRefRef AssetRef) :
TLAsset::TShader	( AssetRef )
{
}


//-------------------------------------------------------
//	pre-render, setup the texture matrix as dictated by data
//-------------------------------------------------------
Bool TLAsset::TShader_TextureMatrix::PreRender(TBinaryTree& ShaderData)	
{
	//	switch to texture matrix mode
	glMatrixMode(GL_TEXTURE);
	
	//	gr: identity should be redundant...
	glLoadIdentity();
	
	//	read transform from shader data
	TLMaths::TTransform TextureTransform;
	if ( TextureTransform.ImportData( ShaderData ) != TLMaths_TransformBitNone )
	{
		//	apply the transform to the scene (currently the texture mode)
		TLRender::Opengl::SceneTransform( TextureTransform );
	}
	
	//	gr: assuming we were in modelview mode before
	glMatrixMode(GL_MODELVIEW);
	
	return true;	
}


//-------------------------------------------------------
//	post-render, restore default texture matrix
//-------------------------------------------------------
void TLAsset::TShader_TextureMatrix::PostRender()
{
	//	restore matrix for texture mode
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	
	//	gr: again, assuming we were in modelview mode before
	glMatrixMode(GL_MODELVIEW);
}
