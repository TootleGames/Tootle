/*

	Base type of window to edit an asset

*/
#pragma once

#include <TootleGui/TWindow.h>


namespace TLReflection
{
	class TEditAssetWindow;
}


class TLReflection::TEditAssetWindow
{
public:
	TEditAssetWindow(const TTypedRef& Asset);
	
	
private:
	TTypedRef	m_Asset;	
};




