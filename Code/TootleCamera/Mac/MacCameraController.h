/*
 *  MacCameraController.h
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 28/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

#include <TootleCore/TSubscriber.h>


@interface MacCameraController : NSViewController 
{
	QTCaptureSession*			m_Session;			// The device session
	
	QTCaptureDevice*			m_Camera;			// Camera device access
	QTCaptureDeviceInput*		m_CameraInput;		// Camera input	
}

- (bool) ConnectToCamera;
- (bool) DisconnectFromCamera;
-(bool) IsConnected;


-(bool) CreateCameraView;

-(void) SubscribeToCamera:(TLMessaging::TSubscriber*)pSubscriber;
-(void) UnsubscribeFromCamera:(TLMessaging::TSubscriber*)pSubscriber;

@end