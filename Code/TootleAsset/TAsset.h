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
	class TTempAsset;		//	placeholder asset type

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
	TAsset(const TRef& AssetType,const TRef& AssetRef);

	Bool				IsLoaded() const						{	return GetLoadingState() == TLAsset::LoadingState_Loaded;	}
	TLoadingState		GetLoadingState() const					{	return m_LoadingState;	}
	void				SetLoadingState(TLoadingState State)	{	m_LoadingState = State;	}

	const TRef&			GetAssetType() const			{	return m_AssetType;	}
	void				SetAssetRef(const TRef& Ref)	{	m_AssetRef = Ref;	}
	const TRef&			GetAssetRef() const				{	return m_AssetRef;	}

	virtual void		Update()						{	}	//	update this asset
	virtual SyncBool	Shutdown()						{	return SyncTrue;	}

	void				Import(TPtr<TLFileSys::TFileAsset>& pAssetFile);	//	load asset data out of file - get state with GetLoadingState()
	SyncBool			Export(TPtr<TLFileSys::TFileAsset>& pAssetFile);	//	save asset to file

	TPtr<TBinaryTree>&	GetData(TRefRef DataRef,Bool CreateNew=FALSE);

	inline Bool			operator==(const TRef& AssetRef) const	{	return GetAssetRef() == AssetRef;	}
	inline Bool			operator==(const TAsset& Asset) const 	{	return GetAssetRef() == Asset.GetAssetRef();	}

protected:
	virtual SyncBool	ImportData(TBinaryTree& Data)			{	return SyncFalse;	}	//	load asset data out binary data
	virtual SyncBool	ExportData(TBinaryTree& Data)			{	return SyncFalse;	}	//	save asset data to binary data
	void				ImportUnknownData(TBinaryTree& Data);	//	take any data in this binary tree that we didn't read after importing and put it into this asset's data (m_Data)
	void				ExportUnknownData(TBinaryTree& Data);	//	write out our unknown data

protected:
	TPtrArray<TBinaryTree>	m_Data;						//	after importing, this data was not read in. could just be additonal meta data

private:
	TRef				m_AssetType;
	TRef				m_AssetRef;
	TLoadingState		m_LoadingState;					//	loading state 
	TString				m_Debug_AssetRefString;	
};





//----------------------------------------------------
//	base asset type
//----------------------------------------------------
class TLAsset::TTempAsset : public TLAsset::TAsset
{
public:
	TTempAsset(const TRef& AssetRef) : 
		TAsset( "Temp", AssetRef )
	{
	}
};

