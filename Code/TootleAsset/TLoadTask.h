/*------------------------------------------------------

	Asset/file loading/conversion asynchronous task

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include <TootleCore/TStateMachine.h>


namespace TLAsset
{
	class TLoadTask;
	extern TPtrArray<TLoadTask>		g_LoadTasks;
	FORCEINLINE TPtr<TLoadTask>&	GetLoadTask(const TTypedRef& AssetAndTypeRef)	{	return g_LoadTasks.FindPtr( AssetAndTypeRef );	}	//	get the load task for this asset
}

namespace TLLoadTask
{
	class TLoadTaskMode;
}


//------------------------------------------------------------
//	asset loader
//------------------------------------------------------------
class TLAsset::TLoadTask : public TStateMachine
{
	friend class TLLoadTask::TLoadTaskMode;
public:
	TLoadTask(const TTypedRef& AssetAndTypeRef);

	SyncBool						Update(float Timestep,Bool Blocking);	//	update load - blocking will force it to loop until non-waiting
	SyncBool						GetLoadingState() const;		//	depending on the state we can tell if it's loading, failed or loaded okay

	FORCEINLINE const TTypedRef&	GetAssetAndTypeRef() const		{	return m_AssetAndTypeRef;	}	
	FORCEINLINE TPtr<TAsset>&		GetAsset() const				{	return TLAsset::GetAssetInstance( GetAssetAndTypeRef() );	}

	Bool							HasFailedToConvertFile(TLFileSys::TFile& File)			{	return m_FailedToConvertFiles.Exists( File );	}
	void							AddFailedToConvertFile(TPtr<TLFileSys::TFile> pFile)	{	if ( pFile )	m_FailedToConvertFiles.Add( pFile );	}

	FORCEINLINE Bool				operator==(const TTypedRef& AssetAndTypeRef) const	{	return GetAssetAndTypeRef() == AssetAndTypeRef;	}

protected:
	TTypedRef					m_AssetAndTypeRef;		//	ref of asset we're creating
	TPtr<TLFileSys::TFile>		m_pFile;				//	plain file needs converting to asset file
	TPtr<TLFileSys::TFileAsset>	m_pTempAssetFile;		//	asset file generated from plain file
	TPtr<TLFileSys::TFileAsset>	m_pAssetFile;			//	asset file in a proper file sys that needs converting to asset
	TPtrArray<TLFileSys::TFile>	m_FailedToConvertFiles;	//	list of files that didnt convert to an asset we require, eg. loading tree.mesh, tree.png might be in this list
};



namespace TLLoadTask
{
	class TLoadTaskMode : public TStateMode
	{
	protected:
		TLAsset::TLoadTask*				GetLoadTask()		{	return GetStateMachine<TLAsset::TLoadTask>();	}
		TPtr<TLFileSys::TFile>&			GetPlainFile()		{	return GetLoadTask()->m_pFile;	}
		TPtr<TLFileSys::TFileAsset>&	GetTempAssetFile()	{	return GetLoadTask()->m_pTempAssetFile;	}
		TPtr<TLFileSys::TFileAsset>&	GetAssetFile()		{	return GetLoadTask()->m_pAssetFile;	}
		TTypedRef&						GetAssetAndTypeRef(){	return GetLoadTask()->m_AssetAndTypeRef;	}	
		TPtr<TLAsset::TAsset>			GetAsset() 			{	return GetLoadTask()->GetAsset();	}

		void							Debug_PrintStep(const char* pStepString);	//	print out some debug info for this step
	};

	//	load plain file from file sys
	class Mode_PlainFileLoad : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	create a temporary asset file from plain file
	class Mode_PlainFileCreateTempAssetFile : public TLoadTaskMode
	{
	protected:
		virtual TRef				Update(float Timestep);
	};

	//	convert plain file to asset file
	class Mode_PlainFileExport : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	fetch asset file
	class Mode_GetAssetFile : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	load asset file
	class Mode_AssetFileLoad : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	turn plain file into asset file
	class Mode_AssetFileImport : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	turn asset file back to plain file
	class Mode_AssetFileExport : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	save asset file back to file sys
	class Mode_AssetFileWrite : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	save asset file back to file sys
	class Mode_AssetFileCreate : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	create new asset
	class Mode_CreateAsset : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	turn asset file into asset
	class Mode_AssetImport : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	all completed
	class Mode_Finished : public TLoadTaskMode
	{
	};

};

