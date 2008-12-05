/*------------------------------------------------------
	
	Local file system... reads files out of a specific directory

-------------------------------------------------------*/
#pragma once
#include "TLFileSys.h"

namespace TLFileSys
{
	namespace Platform
	{
		class LocalFileSys;
	};

	typedef Platform::LocalFileSys TLocalFileSys;
};


