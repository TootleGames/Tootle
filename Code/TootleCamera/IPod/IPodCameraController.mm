/*
 *  IPodCameraController.mm
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 02/02/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "IPodCameraController.h"

#include <TootleCore/TLDebug.h>
#include <TootleCore/TString.h>

#include <QuartzCore/QuartzCore.h>

#define CAMERA_SCALAR 1.12412 // scalar = (480 / (2048 / 480))

@implementation IPodCameraController

@synthesize statusLabel, shadeOverlay, overlayController;

- (id)init 
{
	if (self = [super init]) 
	{
		self.sourceType = UIImagePickerControllerSourceTypeCamera;

		self.navigationBarHidden = YES;
	
#ifdef __MAC_OS_X_VERSION_MIN_REQUIRED
		
	// Note: Test the value explictly rather than __IPHONE_3_0 as the define won't be available when compiling against 
	// the 2.2.1 SDK for example
	#if __IPHONE_OS_VERSION_MIN_REQUIRED > 30000
		self.showsCameraControls = NO;
		self.toolbarHidden = YES;
		self.wantsFullScreenLayout = YES;
		self.cameraViewTransform = CGAffineTransformScale(self.cameraViewTransform, CAMERA_SCALAR, CAMERA_SCALAR);    			
	#endif
#endif
	}
	
	return self;
}


- (void)viewDidAppear:(BOOL)animated 
{
	[super viewDidAppear:animated];	

	self.shadeOverlay = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"overlay1.png"]] autorelease];
	self.shadeOverlay.alpha = 0.0f;
	
	[self.view addSubview:self.shadeOverlay];
}


+ (BOOL)isAvailable 
{
	return [self isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera];
}

- (void)displayModalWithController:(UIViewController*)controller animated:(BOOL)animated 
{
	[controller presentModalViewController:self animated:YES];
}

- (void)dismissModalViewControllerAnimated:(BOOL)animated 
{
	[self.overlayController dismissModalViewControllerAnimated:animated];
}

- (void)takePicture 
{
	if ([self.overlayController respondsToSelector:@selector(cameraWillTakePicture:)]) 
	{
		[self.overlayController cameraWillTakePicture:self];
	}
	
	self.delegate = self;
	
	TLDebug_Print("Taking photo...");
	
	
	[self showShadeOverlay];
	[super takePicture];
}

- (UIImage*)dumpOverlayViewToImage 
{
#ifdef __MAC_OS_X_VERSION_MIN_REQUIRED
	
	// Note: Test the value explictly rather than __IPHONE_3_0 as the define won't be available when compiling against 
	// the 2.2.1 SDK for example
	#if __IPHONE_OS_VERSION_MIN_REQUIRED > 30000
	
		CGSize imageSize = self.cameraOverlayView.bounds.size;
		UIGraphicsBeginImageContext(imageSize);
	
		[self.cameraOverlayView.layer renderInContext:UIGraphicsGetCurrentContext()];
	
		UIImage *viewImage = UIGraphicsGetImageFromCurrentImageContext();
	
		UIGraphicsEndImageContext();
	
		return viewImage;
	#endif
#endif
	// Fails in pre iPhone OS 3.0 SDK
	return nil;
}

- (UIImage*)addOverlayToBaseImage:(UIImage*)baseImage 
{
	UIImage *overlayImage = [self dumpOverlayViewToImage];	
	CGPoint topCorner = CGPointMake(0, 0);
	CGSize targetSize = CGSizeMake(320, 480);	
	CGRect scaledRect = CGRectZero;
	
	CGFloat scaledX = 480 * baseImage.size.width / baseImage.size.height;
	CGFloat offsetX = (scaledX - 320) / -2;
	
	scaledRect.origin = CGPointMake(offsetX, 0.0);
	scaledRect.size.width  = scaledX;
	scaledRect.size.height = 480;
	
	UIGraphicsBeginImageContext(targetSize);	
	
	[baseImage drawInRect:scaledRect];	
	[overlayImage drawAtPoint:topCorner];	
	
	UIImage* result = UIGraphicsGetImageFromCurrentImageContext();
	
	UIGraphicsEndImageContext();	
	
	return result;	
}

- (void)adjustLandscapePhoto:(UIImage*)image 
{
	// TODO: maybe use this for something
	TTempString str("Camera image:");
	str.Append("%f x %f ", image.size.width, image.size.height);
	
	switch (image.imageOrientation) 
	{
		case UIImageOrientationLeft:
		case UIImageOrientationRight:
			// portrait
			str.Append("portrait");
			break;
		default:
			// landscape
			str.Append("landscape");
			break;
	}
	
	TLDebug_Print(str);

}

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info 
{
	
	TLDebug_Print("Saving photo...");
	
	UIImage *baseImage = [info objectForKey:UIImagePickerControllerOriginalImage];
	if (baseImage == nil) return;
	
	// save composite
	UIImage *compositeImage = [self addOverlayToBaseImage:baseImage];
	[self hideShadeOverlay];
	
	if ([self.overlayController respondsToSelector:@selector(cameraDidTakePicture:)]) 
	{
		[self.overlayController cameraDidTakePicture:self];
	}
	
	UIImageWriteToSavedPhotosAlbum(compositeImage, self, @selector(image:didFinishSavingWithError:contextInfo:), nil);
}

- (void)image:(UIImage*)image didFinishSavingWithError:(NSError *)error contextInfo:(NSDictionary*)info 
{
	
}


- (void)showShadeOverlay 
{
	[self animateShadeOverlay:0.82f];
}

- (void)hideShadeOverlay 
{
	[self animateShadeOverlay:0.0f];
}

- (void)animateShadeOverlay:(CGFloat)alpha 
{
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationCurve:UIViewAnimationCurveEaseInOut];
	[UIView setAnimationDuration:0.35f];
	self.shadeOverlay.alpha = alpha;
	[UIView commitAnimations];	
}

- (void)writeImageToDocuments:(UIImage*)image 
{
	NSData *png = UIImagePNGRepresentation(image);
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	
	NSError *error = nil;
	[png writeToFile:[documentsDirectory stringByAppendingPathComponent:@"image.png"] options:NSAtomicWrite error:&error];
}

- (BOOL)canBecomeFirstResponder { return YES; }

- (void)dealloc 
{
	[overlayController release];
	
	[shadeOverlay release];
	
	[super dealloc];
	
}


@end
