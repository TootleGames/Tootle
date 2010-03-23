
#pragma once

#ifndef _MSC_EXTENSIONS
	#error PC file should not be included in ansi builds
#endif

#include <TootleGui/PC/PCGui.h>	//	windows headers

#include <process.h>	// threads - may want to use pthreads lib instead for maximum portability


namespace TLThread
{
	namespace Platform
	{
		template<class T>
		Bool CreateAsThread(T* pClassObj);

		template<class T>
		class Win32ThreadWrapper;
	}
};


template<class T>
Bool TLThread::Platform::CreateAsThread(T* pClassObj)
{

	Win32ThreadWrapper* pThread = new Win32ThreadWrapper(pClassObject);

	delete pThread;

	return TRUE;
}

/*	
	ThreadWrapper class - allows you to create an object as a thread if required without having to explicitly alter the class to support threading
	
	Class (that will be threaded) requirements:
		* ThreadEntryPoint routine - the main 'update' routine of the thread part of the class.  When it returns the thread will be removed.  
		  NOTE: This may change to be an update routine that is called manually from the thread wrapper.

		TODO:
		* Add message handling for each thread so that each thread has it's own message pipeline
*/
template<class T>
class TLThread::Platform::Win32ThreadWrapper
{
public:
	Win32ThreadWrapper(T* pThreadClass);
	~Win32ThreadWrapper(void);

	static unsigned __stdcall ThreadStaticEntryPoint(void * pClass)
	{
			T* pthX = (T*)pClass;   // the tricky cast
			pthX->ThreadEntryPoint();    // now call the true entry-point-function

			// A thread terminates automatically if it completes execution,
			// or it can terminate itself with a call to _endthread().

			return 1;          // the thread exit code
	}

private:
	T*				m_pThreadClass;
	HANDLE			m_ThreadHandle;
	unsigned int	m_uiThreadID;
};

// Thread wrapper constructor
template<class T>
TLThread::Platform::Win32ThreadWrapper<T>::Win32ThreadWrapper(T* pThreadClass)
{
	m_pThreadClass = pThreadClass;

	// Create the thread
	m_ThreadHandle = (HANDLE)_beginthreadex( NULL,         // security
                      0,            // stack size
                      ThreadStaticEntryPoint,// entry-point-function
                      m_pThreadClass,           // arg list holding the "this" pointer
                      0, // so we can later call ResumeThread()
                      &m_uiThreadID );
};

// ThreadWrapper destructor
template<class T>
TLThread::Platform::Win32ThreadWrapper<T>::~Win32ThreadWrapper(void)
{
	printf_s("Waiting for thread\n");
	// Wait for the thread to finish
	WaitForSingleObject( m_ThreadHandle, INFINITE );

	m_pThreadClass = NULL;

	printf_s("Thread finished\n");
};
