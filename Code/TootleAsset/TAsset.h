/*------------------------------------------------------

	Base asset type 

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"

class TBinaryTree;

namespace TLFileSys
{
	class TFile;
	class TFileAsset;
}

namespace TLAsset
{
	class TAsset;			//	base asset type

	enum TLoadingState
	{
		LoadingState_Init = 0,	//	just created - nothing done to it yet
		LoadingState_Loading,	//	currently loading
		LoadingState_Failed,	//	failed to load corrupt file/asset
		LoadingState_Loaded,	//	is loaded okay
		LoadingState_Deleted,	//	asset has been formely deleted. We shouldnt really ever come across this state, debugging essentially
	};

};


//----------------------------------------------------
//	base asset type
//----------------------------------------------------
class TLAsset::TAsset
{
public:
	TAsset(TRefRef AssetType,TRefRef AssetRef);

	static TRef			GetAssetType_Static()					{	return TRef_Static(A,s,s,e,t);	}

	Bool				IsLoaded() const						{	return GetLoadingState() == TLAsset::LoadingState_Loaded;	}
	TLoadingState		GetLoadingState() const					{	return m_LoadingState;	}
	void				SetLoadingState(TLoadingState State)	{	m_LoadingState = State;	}

	TRefRef				GetAssetType() const					{	return m_AssetAndTypeRef.GetTypeRef();	}
	void				SetAssetRef(TRefRef Ref)				{	m_AssetAndTypeRef.SetRef( Ref );	}
	TRefRef				GetAssetRef() const						{	return m_AssetAndTypeRef.GetRef();	}
	const TTypedRef&	GetAssetAndTypeRef() const				{	return m_AssetAndTypeRef;	}

	virtual void		Update()						{	}	//	update this asset
	virtual SyncBool	Shutdown()						{	return SyncTrue;	}

	void				Import(TPtr<TLFileSys::TFileAsset>& pAssetFile);	//	load asset data out of file - get state with GetLoadingState()
	SyncBool			Export(TPtr<TLFileSys::TFileAsset>& pAssetFile);	//	save asset to file

	TPtr<TBinaryTree>&	GetData(TRefRef DataRef,Bool CreateNew=FALSE);
	TBinaryTree&		GetData()								{	return m_Data;	}

	inline Bool			operator==(const TRef& AssetRef) const		{	return GetAssetRef() == AssetRef;	}
	inline Bool			operator==(const TTypedRef& AssetRef) const	{	return GetAssetAndTypeRef() == AssetRef;	}
	inline Bool			operator==(const TAsset& Asset) const 		{	return GetAssetAndTypeRef() == Asset.GetAssetAndTypeRef();	}

protected:
	virtual SyncBool	ImportData(TBinaryTree& Data);			//	load asset data out binary data - base type just imports dumb m_Data
	virtual SyncBool	ExportData(TBinaryTree& Data);			//	save asset data to binary data - base type just exports dumb m_Data
	void				ImportUnknownData(TBinaryTree& Data);	//	take any data in this binary tree that we didn't read after importing and put it into this asset's data (m_Data)
	void				ExportUnknownData(TBinaryTree& Data);	//	write out our unknown data

protected:
	TBinaryTree			m_Data;							//	after importing, this data was not read in. could just be additonal meta data or old data we don't use any more

private:
	TTypedRef			m_AssetAndTypeRef;
	TLoadingState		m_LoadingState;					//	loading state 
	TString				m_Debug_AssetRefString;	
};




