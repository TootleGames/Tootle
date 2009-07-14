#include "TWidgetButton.h"




void TLGui::TWidgetButton::OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef)
{
	if(AllowClickOnCursorMove())
	{
		
		//	gr: the Move action is dependant on the parent, so assuming mouse is down.
		//		if that changes (which would be for hover-detection, which would be windows only)
		//		then we need to work out the button's raw state from this message...
		float RawValue = 1.f;
		QueueClick( NewCursorPosition, RawValue, ActionRef, TLGui_WidgetActionType_Down );	
	}
}
