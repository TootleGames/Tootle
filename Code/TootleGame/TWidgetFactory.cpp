/*
 *  TWidgetFactory.cpp
 *  TootleGame
 *
 *  Created by Duane Bradbury on 17/11/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TWidgetFactory.h"

#include "TWidget.h"
#include "TWidgetButton.h"
#include "TWidgetDrag.h"
#include "TWidgetText.h"
#include "TWidgetThumbStick.h"
#include "TWidgetScrollbar.h"


TLGui::TWidget*	TLGui::TWidgetFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Button" )
		return new TLGui::TWidgetButton( InstanceRef, TypeRef );

	if ( TypeRef == "Drag" )
		return new TLGui::TWidgetDrag( InstanceRef, TypeRef );

	if ( TypeRef == "Text" )
		return new TLGui::TWidgetText( InstanceRef, TypeRef );

	if ( TypeRef == "Stick" )
		return new TLGui::TWidgetThumbStick( InstanceRef, TypeRef );
	
	if ( TypeRef == "Scroll" )
		return new TLGui::TWidgetScrollbar( InstanceRef, TypeRef );


	return NULL;
}
