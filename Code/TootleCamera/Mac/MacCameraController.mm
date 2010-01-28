/*
 *  MacCameraController.mm
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 28/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "MacCameraController.h"
#include "MacCameraView.h"

@implementation MacCameraController




- (void)dealloc {
	
	if([self IsConnected])
	{
		[self DisconnectFromCamera];
	}
	
	[super dealloc];
}


- (void)loadView
{	
	// Create the view
	[self CreateCameraView];

	
}


- (bool) ConnectToCamera 
{
	//Create the QT capture session
	m_Session = [[QTCaptureSession alloc] init];
	
	if(!m_Session)
		return false;
	
	/* Select the default Video input device */
	m_Camera = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
	
	if(!m_Camera)
	{
		[m_Session release];
		
		return false;		
	}
	
	/* Passing nil for the NSError parameter may not be the best idea
	 but i will leave error handling up to you */
	[m_Camera open:nil];
	
	/* Create a QTKit input for the session using the iSight Device */
	m_CameraInput = [QTCaptureDeviceInput deviceInputWithDevice:m_Camera];
	
	
	if(!m_CameraInput)
	{
		[m_Session release];
		[m_Camera release];
		
		return false;		
	}
	
	/* Create a capture session for the live video and add inputs get the ball rolling etc */
	[m_Session addInput:m_CameraInput error:nil];
	
	MacCameraView* pCameraView = (MacCameraView*)self.view;
	[pCameraView setCaptureSession:m_Session];
	
	/* Let the video madness begin */
	[m_Session startRunning]; 
	
	return true;
}


-(bool) CreateCameraView
{
	NSRect frame = NSMakeRect(0, 0, 320, 480);
	//NSRect frame = [NSMakeRect(0, 0, 480, 320)];
	
	MacCameraView *view = [[MacCameraView alloc] initWithFrame:frame];
	
   // [view setAutoresizingMask:NSViewAutoresizingNone];	
    //[view setBackgroundColor: [NSColor redColor]];
    //[view setBackgroundColor: [UIColor clearColor]];

	// Create the camera view delegate so we can intercept the image to be displayed 
	// and do some post processing on it before it is displayed
	[view setDelegate: [MacCameraViewDelegate alloc]];
	
    self.view = view;
	
    [view release];

	return true;
}


- (bool) DisconnectFromCamera 
{
	// Stop the camera
	[m_Session stopRunning];
	
	[m_Session removeInput:m_CameraInput];
	
	[m_Camera close];

	
	// Release the camera and session
	[m_Camera release];
	[m_Session release];
	
	
	m_Session = nil;
	m_Camera = nil;
	m_CameraInput = nil;
	
	return true;
}

-(bool) IsConnected
{
	if(m_Camera && m_Session)
		return true;
	
	return false;
}



@end