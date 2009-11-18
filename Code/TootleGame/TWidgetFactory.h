/*
 *  TWidgetFactory.h
 *  TootleGame
 *
 *  Created by Duane Bradbury on 17/11/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TWidget.h"

#include <TootleCore/TClassFactory.h>

namespace TLGui
{
	class TWidgetFactory;
}

class TLGui::TWidgetFactory : public TClassFactory<TLGui::TWidget>
{
protected:
	virtual TLGui::TWidget*			CreateObject(TRefRef InstanceRef,TRefRef TypeRef);

};
