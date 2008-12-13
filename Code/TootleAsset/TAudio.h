#pragma once

#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TAsset.h>


namespace TLAsset
{
	class TAudio;
};



class TLAsset::TAudio : public TLAsset::TAsset
{
public:
	TAudio(const TRef& AssetRef);

	TRefRef			GetBufferRef()	const { return m_BufferRef; }
protected:
	
private:
	TRef	m_BufferRef;
};