#pragma once

#include "TLTypes.h"

namespace TLThread
{
		template<class T>
		Bool CreateAsThread(T* pClassObj);

		namespace Platform
		{
			template<class T>
			void CreateAsThread(T* pClassObj);
		};
};


template<class T>
Bool TLThread::CreateAsThread(T* pClassObj)
{
	return Platform::CreateAsThread(pClassObj);
}
