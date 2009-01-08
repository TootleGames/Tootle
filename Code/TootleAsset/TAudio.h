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

	TRefRef			GetBufferRef()	const	{ return m_BufferRef; }
	
	u32			GetSize()								const	{ return m_HeaderData.m_Size; }
	void		SetSize(u32 uSize)								{ m_HeaderData.m_Size = uSize; }
	
	u32			GetSampleRate()							const	{ return m_HeaderData.m_SampleRate; }
	void		SetSampleRate(u32 uRate)						{ m_HeaderData.m_SampleRate = uRate; }
	
	u8			GetNumberOfChannels()					const	{ return m_HeaderData.m_NumChannels; }
	void		SetNumberOfChannels(u8 uNumChannels)			{ m_HeaderData.m_NumChannels = uNumChannels; }
	
	u8			GetBitsPerSample()						const	{ return m_HeaderData.m_BitsPerSample; }
	void		SetBitsPerSample(u8 uBitsPerSample)				{ m_HeaderData.m_BitsPerSample = uBitsPerSample; }
	
	
	TBinary&	RawAudioDataBinary()		{ return m_RawAudioData; }
protected:
	
	class TAudioHeader
	{
	public:
		u32		m_Size;				// Amount of audio data
		u32		m_SampleRate;		// Sample rate
		u8		m_NumChannels;		// Number of channels
		u8		m_BitsPerSample;	// Bits per sample
	};
	
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data
	
private:
	TAudioHeader	m_HeaderData;	
	
	TBinary		m_RawAudioData;
	
	TRef	m_BufferRef;			// Audio object ID - not sure this is needed yet
	int		m_Buffer;				// ALUint OpenAL mapping
};