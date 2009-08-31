#include "PCConnectionHttp.h"
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

namespace TLNetwork
{
	namespace Platform
	{
		size_t	RecieveData(void *ptr, size_t size, size_t nmemb, void *data);	//	data should be a TConnectionHttp

		void*		Alloc(size_t size);	//	curl_malloc_callback
		void		Delete(void *ptr);	//	curl_free_callback
		void*		Realloc(void *ptr, size_t size);	//	curl_realloc_callback
		char*		StrDup(const char *str);	//	curl_strdup_callback
		void*		ArrayAlloc(size_t nmemb, size_t size);	//	curl_calloc_callback 
	}
}



void* TLNetwork::Platform::Alloc(size_t size)
{
	void* pMem = TLMemory::TMemorySystem::Instance().Allocate( size );

	if ( pMem && size > 0 )
		memset( pMem, 0, size );

	return pMem;
}

void TLNetwork::Platform::Delete(void *ptr)
{
	TLMemory::TMemorySystem::Instance().Deallocate( ptr );
}

void* TLNetwork::Platform::Realloc(void *ptr, size_t size)
{
	void* pMem = TLMemory::Platform::MemRealloc( ptr, size );
	return pMem;
}

char* TLNetwork::Platform::StrDup(const char *str)
{
	//	alloc a string long enough to include a terminator
	u32 Length = strlen( str );
	char* pNewString = (char*)Alloc( Length + 1 );
	pNewString[Length] = 0x0;

	//	copy rest of the string
	TLMemory::CopyData( pNewString, str, Length );

	return pNewString;
}

void* TLNetwork::Platform::ArrayAlloc(size_t nmemb, size_t size)
{
	//	gr: need to find out how this memory is deleted to see if we need to use ArrayAlloc/ArrayDelete with it
	void* pMem = Alloc( nmemb * size );
	return pMem;
}


size_t TLNetwork::Platform::RecieveData(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	TLNetwork::Platform::TConnectionHttp* pConnection = (TLNetwork::Platform::TConnectionHttp*)data;

	if ( !pConnection->OnRecieveData( (u8*)ptr, realsize ) )
		return 0;

	return realsize;
}



TLNetwork::Platform::TConnectionHttp::TConnectionHttp() :
	TLNetwork::TConnection	(),
	m_pCurl					( NULL ),
	m_pRecieveData			( NULL )
{
}


//---------------------------------------------------------
//	initialise curl
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::Initialise(TRef& ErrorRef)
{
	//	already initialised
	if ( m_pCurl )
		return SyncTrue;

	//	init curl global system with our own mem alloc routines
	CURLcode Result = curl_global_init_mem(CURL_GLOBAL_ALL,
											TLNetwork::Platform::Alloc,
											TLNetwork::Platform::Delete,
                                          TLNetwork::Platform::Realloc,
                                          TLNetwork::Platform::StrDup,
                                          TLNetwork::Platform::ArrayAlloc );

//	curl_global_init(CURL_GLOBAL_ALL);

	//	init curl
	m_pCurl = curl_easy_init();
	
	if ( !m_pCurl )
	{
		ErrorRef = "NoCurl";
		return SyncFalse;
	}

	return SyncTrue;
}

//---------------------------------------------------------
//	
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::GetData(const TString& Url,TBinary& Data,TRef& ErrorRef)
{
	if ( !m_pCurl )
	{
		ErrorRef = "NotInit";
		return SyncFalse;
	}

	if ( !Url.GetLength() )
	{
		ErrorRef = "NoUrl";
		return SyncFalse;
	}

	//	set url
    curl_easy_setopt( m_pCurl, CURLOPT_URL, Url.GetData() );

	//	tell it what func to recieve data to
	m_pRecieveData = &Data;
	curl_easy_setopt( m_pCurl, CURLOPT_WRITEFUNCTION, RecieveData);
	curl_easy_setopt( m_pCurl, CURLOPT_WRITEDATA, (void*)this );

	// some servers don't like requests that are made without a user-agent field, so we provide one
	curl_easy_setopt( m_pCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	//	store data in the data provided
	CURLcode Result = curl_easy_perform( m_pCurl );
	m_pRecieveData = NULL;

	//	work out error
	if ( Result != CURLE_OK )
	{
		TTempString ErrorString;
		ErrorString.Appendf("C%d", Result );
		ErrorRef = ErrorString;
		return SyncFalse;
	}

	return SyncTrue;
}


//---------------------------------------------------------
//	recieved some data from curl
//---------------------------------------------------------
Bool TLNetwork::Platform::TConnectionHttp::OnRecieveData(const u8* pData,u32 Size)
{
	if ( !m_pRecieveData )
	{
		TLDebug_Break("recieving data with no buffer assigned");
		return FALSE;
	}

	//	store this data
	m_pRecieveData->WriteData( pData, Size );

	return TRUE;
}

//---------------------------------------------------------
//	
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::Shutdown()
{
	if ( m_pCurl )
	{
		curl_easy_cleanup(m_pCurl);
		m_pCurl = NULL;
	}

	return SyncTrue;
}

