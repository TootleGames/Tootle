/*------------------------------------------------------

	Asset/file loading/conversion asynchronous task

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include <TootleCore/TStateMachine.h>


namespace TLAsset
{
	class TLoadTask;
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
	TLoadTask(TRefRef AssetRef);

	SyncBool		Update(float Timestep,Bool Blocking);	//	update load - blocking will force it to loop until non-waiting

	TRefRef			GetAssetRef() const					{	return m_AssetRef;	}	
	TPtr<TAsset>&	GetAsset() const					{	return TLAsset::GetAsset( GetAssetRef() );	}

	inline Bool		operator==(TRefRef AssetRef) const	{	return GetAssetRef() == AssetRef;	}

protected:
	TRef						m_AssetRef;			//	ref of asset we're creating
	TPtr<TLFileSys::TFile>		m_pFile;			//	plain file needs converting to asset file
	TPtr<TLFileSys::TFileAsset>	m_pAssetFile;		//	asset file needs converting to asset
};




namespace TLLoadTask
{
	class TLoadTaskMode : public TStateMode
	{
	protected:
		TLAsset::TLoadTask*				GetLoadTask()		{	return GetStateMachine<TLAsset::TLoadTask>();	}
		TPtr<TLFileSys::TFile>&			GetPlainFile()		{	return GetLoadTask()->m_pFile;	}
		TPtr<TLFileSys::TFileAsset>&	GetAssetFile()		{	return GetLoadTask()->m_pAssetFile;	}
		TRef&							GetAssetRef() 		{	return GetLoadTask()->m_AssetRef;	}	
		TPtr<TLAsset::TAsset>			GetAsset() 			{	return GetLoadTask()->GetAsset();	}

		void							Debug_PrintStep(const char* pStepString);	//	print out some debug info for this step
	};

	//	first mode to decide what to do
	class Mode_Init : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	fetch the plain file
	class Mode_GetPlainFile : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	load plain file from file sys
	class Mode_PlainFileLoad : public TLoadTaskMode
	{
	protected:
		virtual TRef			Update(float Timestep);
	};

	//	create asset file from plain file
	class Mode_PlainFileCreateAssetFile : public TLoadTaskMode
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

