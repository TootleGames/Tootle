/*------------------------------------------------------
	
	Debug file system - this is for creating assets
	at runtime. Cubes, spheres etc

-------------------------------------------------------*/
#pragma once
#include "TLFileSys.h"

namespace TLFileSys
{
	class TVirtualFileSys;	
};



//------------------------------------------------------------
//	debug file system - this file system generates files at runtime
//	can just be used as a dummy file system too
//------------------------------------------------------------
class TLFileSys::TVirtualFileSys : public TLFileSys::TFileSys
{
public:
	TVirtualFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef);

	virtual SyncBool		LoadFileList();
	virtual SyncBool		LoadFile(TPtr<TFile>& pFile);
	virtual SyncBool		WriteFile(TPtr<TFile>& pFile);		//	add this file into the file system if it's not there
	virtual TPtr<TFile>		CreateFile(const TString& Filename,TRef TypeRef);	//	create a new empty file into file system if possible - if the filesys is read-only we cannot add external files and this fails
	virtual SyncBool		DeleteFile(const TTypedRef& FileRef);	//	delete file from file sys
};

