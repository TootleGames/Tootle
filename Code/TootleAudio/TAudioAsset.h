#pragma once

#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TAsset.h>


namespace TLAsset
{
	class TAudioAsset;
};



class TLAsset::TAudioAsset : public TLAsset::TAsset
{
public:
	TAudioAsset(const TRef& AssetRef);

	TRefRef			GetBufferRef()	const { return m_BufferRef; }
protected:
	
private:
	TRef	m_BufferRef;
};