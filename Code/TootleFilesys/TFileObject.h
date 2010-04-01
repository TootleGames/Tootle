
#pragma once

#include "TFileScheme.h"

#include <TootleAsset/TObject.h>

namespace TLFileSys
{
	class TFileObject;
}

//---------------------------------------------------------
//	Object file - xml file that has markup for nodes in a scheme format
//---------------------------------------------------------
class TLFileSys::TFileObject : public TLFileSys::TFileScheme
{
public:
	TFileObject(TRefRef FileRef,TRefRef FileTypeRef) :
	  TFileScheme(FileRef, FileTypeRef)
	 {}

	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static(O,b,j,e,c);	}

	virtual TLAsset::TScheme* CreateAsset() { 	return new TLAsset::TObject( GetFileRef()); }

};
