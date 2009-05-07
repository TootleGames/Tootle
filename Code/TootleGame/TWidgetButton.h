
#pragma once

#include "TInputInterface.h"

namespace TLGui
{
	class TWidgetButton;
}


class TLGui::TWidgetButton : public TLInput::TInputInterface
{
public:
	TWidgetButton(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef()) : 
		TLInput::TInputInterface	( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp )
	{
	}

};