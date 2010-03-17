/*
 *  MacCameraView.h
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */


#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

#include <TootleCore/TSubscriber.h>


class TPublisherProxy : public TLMessaging::TPublisher
{
};


@interface MacCameraView : QTCaptureView 
{
}

@end



@interface MacCameraViewDelegate : NSObject 
{
	TPublisherProxy		m_PublisherProxy;
}

- (void)addSubscriber:(TLMessaging::TSubscriber*) pSubscriber;

//- (NSImage *)getNSImageFromCIImage:(CIImage*)image;
//- (NSBitmapImageRep *)RGBABitmapImageRepFromCImage:(CIImage *) ciImage;


@end