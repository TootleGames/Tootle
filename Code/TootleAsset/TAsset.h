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
};


//----------------------------------------------------
//	base asset type
//----------------------------------------------------
class TLAsset::TAsset
{
public:
	TAsset(const TRef& AssetType,const TRef& AssetRef);

	Bool				IsLoaded() const				{	return GetLoadingState() == SyncTrue;	}
	SyncBool			GetLoadingState() const			{	return m_LoadingState;	}
	void				SetLoadingState(SyncBool State)	{	m_LoadingState = State;	}

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
	SyncBool			m_LoadingState;					//	loading state - FALSE not loaded, WAIT loading, TRUE loaded & ready
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

