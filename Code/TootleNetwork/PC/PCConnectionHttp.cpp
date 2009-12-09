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


TLNetwork::Platform::TCurlTask::~TCurlTask()
{
	if ( m_pMultiHandle )
	{
		curl_multi_cleanup( m_pMultiHandle );
		m_pMultiHandle = NULL;
	}

	if ( m_pHandle )
	{
		curl_easy_cleanup( m_pHandle );
		m_pHandle = NULL;
	}
}


//---------------------------------------------------------
//	create curl handle
//---------------------------------------------------------
CURL* TLNetwork::Platform::TCurlTask::InitHandle()
{
	//	todo: make sure global curl is initialised

	//	alloc handle
	if ( !m_pHandle )
	{
		m_pHandle = curl_easy_init();
		
		if ( !m_pHandle )
			return FALSE;
	}
	
	//	tell it what func to recieve data to, the data param is a pointer to the task which should continue to exist
	//	todo: change to a ref so it's a bit safer and have a global task list
	curl_easy_setopt( m_pHandle, CURLOPT_WRITEFUNCTION, TLNetwork::Platform::RecieveData);
	curl_easy_setopt( m_pHandle, CURLOPT_WRITEDATA, (void*)this );

	// some servers don't like requests that are made without a user-agent field, so we provide one
	curl_easy_setopt( m_pHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	//	debug output
    curl_easy_setopt( m_pHandle, CURLOPT_VERBOSE, 1L);

	return m_pHandle;
}


TLNetwork::Platform::TConnectionHttp::TConnectionHttp() :
	TLNetwork::TConnection	()
{
}


//---------------------------------------------------------
//	initialise curl
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::Initialise(TRef& ErrorRef)
{
	//	init curl global system with our own mem alloc routines
	CURLcode Result = curl_global_init_mem(CURL_GLOBAL_ALL,
											TLNetwork::Platform::Alloc,
											TLNetwork::Platform::Delete,
                                          TLNetwork::Platform::Realloc,
                                          TLNetwork::Platform::StrDup,
                                          TLNetwork::Platform::ArrayAlloc );

//	curl_global_init(CURL_GLOBAL_ALL);


	return SyncTrue;
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::Shutdown()
{
	return TConnection::Shutdown();
}



//---------------------------------------------------------
//	start a GET task. Returns error ref. 
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartDownloadTask(TTask& Task)
{
	TCurlTask& CurlTask = static_cast<TCurlTask&>( Task );

	CURL* pHandle = CurlTask.InitHandle();
	if ( !pHandle )
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
    curl_easy_setopt( pHandle, CURLOPT_URL, Url.GetData() );

	//	execute 
	//	todo: make async
	CURLcode Result = curl_easy_perform( pHandle );

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




//---------------------------------------------------------
//	start a POST task.
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartUploadTask(TTask& Task)
{
	TCurlTask& CurlTask = static_cast<TCurlTask&>( Task );

	CURL* pHandle = CurlTask.InitHandle();
	if ( !pHandle )
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
    curl_easy_setopt( pHandle, CURLOPT_URL, Url.GetData() );


	//	setup form
	curl_httppost* pForm = NULL;
	curl_httppost* pLast = NULL;

	//	add form data for each bit of upload data
	TPtr<TBinaryTree>& pUploadData = CurlTask.GetData().GetChild("Upload");
	if ( !pUploadData )
	{
		Task.SetStatusFailed("NoData");
		return;
	}

	TPtrArray<TBinaryTree>& UploadDatas = pUploadData->GetChildren();
	for ( u32 i=0;	i<UploadDatas.GetSize();	i++ )
	{
		TBinaryTree& UploadData = *UploadDatas[i];

		//	the data's ref represents the field name
		TTempString DataRefString;
		UploadData.GetDataRef().GetUrlString( DataRefString );

		//	append the data type if we have it. Parenthesis are safe to use as theyre not in the ref alphabet, and are safe url-enc characters
		if ( UploadData.GetDataTypeHint().IsValid() )
		{
			//	this produces "MYINT(U16)=00FF" instead of just "MYINT=00FF"
			DataRefString.Append('(');
			UploadData.GetDataTypeHint().GetUrlString( DataRefString );
			DataRefString.Append(')');
		}

		//	convert the raw data to a hex string(UTF8 compatible)
		//	we're doing this to match the IPod code which I can't see a way to send raw data
		//	without converting it to a string
		TString DataString;
		UploadData.GetData().GetDataHexString( DataString, TRUE, FALSE );

		//	add pointers to the data for the form
		CURLFORMcode FormError = curl_formadd(	&pForm,
												&pLast,
												CURLFORM_COPYNAME, DataRefString.GetData(),		//	copies data
												//CURLFORM_PTRCONTENTS, UploadData.GetData().GetData(),
												//CURLFORM_CONTENTSLENGTH, UploadData.GetSize(),
												CURLFORM_COPYCONTENTS, DataString.GetData(),
												CURLFORM_CONTENTSLENGTH, DataString.GetLengthWithoutTerminator(),
												CURLFORM_END);

		//	setup okay, continue
		if ( FormError == CURL_FORMADD_OK )
			continue;

		//	error with form
		TLDebug_Break("Error with form params");
		Task.SetStatusFailed("Error");
		return;
	}

	//	assign form
    curl_easy_setopt( pHandle, CURLOPT_HTTPPOST, pForm);

	//	execute 
	CURLcode Result = curl_easy_perform( pHandle );

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

