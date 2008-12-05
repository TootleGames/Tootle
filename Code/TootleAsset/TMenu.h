/*------------------------------------------------------

	Menu asset. Layout for a menu

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"


namespace TLAsset
{
	class TMenu;
};



class TLAsset::TMenu : public TLAsset::TAsset
{
public:
	TMenu(const TRef& AssetRef);

protected:
};

