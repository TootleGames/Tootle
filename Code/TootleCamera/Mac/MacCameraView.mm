/*
 *  MacCameraView.cpp
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "MacCameraView.h"


@implementation MacCameraView


@end


@implementation MacCameraViewDelegate

// Intercept the image from the camera that will be displayed.
// We can do some post processing on the image data at this point and display something else
// or if we return nil it will display the image as-is
- (CIImage *)view:(QTCaptureView *)view willDisplayImage:(CIImage *)image
{
	return nil;
}

@end