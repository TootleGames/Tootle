/*------------------------------------------------------
	PC Core include header

-------------------------------------------------------*/
#pragma once

#include "../TLTypes.h"

class TBinaryTree;

namespace TLCore
{
	namespace Platform
	{
		void				Sleep(u32 Millisecs);	//	platform thread/process sleep

		void				QueryHardwareInformation(TBinaryTree& Data);
		void				QueryLanguageInformation(TBinaryTree& Data);
	}
};


