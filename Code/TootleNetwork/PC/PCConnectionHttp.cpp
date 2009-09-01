#include "PCConnectionHttp.h"
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

namespace TLNetwork
{
	namespace Platform
	{
		size_t		RecieveData(void *ptr, size_t size, size_t nmemb, void *data);	//	data should be a TConnectionHttp

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
	TLNetwork::TTask* pTask = (TLNetwork::TTask*)data;
	if ( !pTask )
		return 0;

	if ( !pTask->RecieveData( (const u8*)ptr, (u32)realsize ) )
		return 0;

	return realsize;
}



TLNetwork::Platform::TConnectionHttp::TConnectionHttp() :
	TLNetwork::TConnection	(),
	m_pCurl					( NULL )
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
	
	//	setup some global curl params

	// some servers don't like requests that are made without a user-agent field, so we provide one
	curl_easy_setopt( m_pCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	return SyncTrue;
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

	return TConnection::Shutdown();
}


//---------------------------------------------------------
//	start a task.
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartTask(TTask& Task)
{
	//	start type of task
	if ( Task.GetTaskType() == "Get" )
	{
		StartGetTask( Task );
		return;
	}

	//	unknown type
	Task.SetStatusFailed("NoType");
}


//---------------------------------------------------------
//	start a GET task. Returns error ref. 
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartGetTask(TTask& Task)
{
	if ( !m_pCurl )
	{
		Task.SetStatusFailed("NoInit");
		return;
	}

	//	get the url string
	TTempString Url;
	if ( !Task.GetData().ImportDataString("url", Url ) )
	{
		Task.SetStatusFailed("NoUrl");
		return;
	}

	//	set url
    curl_easy_setopt( m_pCurl, CURLOPT_URL, Url.GetData() );

	//	tell it what func to recieve data to, the data param is a pointer to the task which should continue to exist
	//	todo: change to a ref so it's a bit safer and have a global task list
	curl_easy_setopt( m_pCurl, CURLOPT_WRITEFUNCTION, TLNetwork::Platform::RecieveData);
	curl_easy_setopt( m_pCurl, CURLOPT_WRITEDATA, (void*)&Task );

	//	execute 
	//	todo: make async
	CURLcode Result = curl_easy_perform( m_pCurl );

	//	finished
	if ( Result == CURLE_OK )
	{
		Task.SetStatusSuccess();
	}
	else 
	{
		TTempString ErrorString;
		ErrorString.Appendf("C%d", Result );
		TRef ErrorRef = ErrorString;
		Task.SetStatusFailed( ErrorRef );
	}
}


