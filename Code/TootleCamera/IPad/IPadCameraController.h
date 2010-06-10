/*
 *  IPodCameraController.h
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 02/02/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */



#import <UIKit/UIKit.h>


@interface IPodCameraController : UIImagePickerController <UINavigationControllerDelegate, UIImagePickerControllerDelegate> 
{
	UIViewController *overlayController;
	UIImageView *shadeOverlay;
}

@property (nonatomic, retain) UIViewController *overlayController;
@property (nonatomic, retain) UILabel *statusLabel;
@property (nonatomic, retain) UIImageView *shadeOverlay;

+ (BOOL)isAvailable;


//- (bool) ConnectToCamera;
//- (bool) DisconnectFromCamera;
//- (bool) IsConnected;


- (void)displayModalWithController:(UIViewController*)controller animated:(BOOL)animated;
- (void)dismissModalViewControllerAnimated:(BOOL)animated;

- (void)takePicture;
- (void)writeImageToDocuments:(UIImage*)image;

- (void)showShadeOverlay;
- (void)hideShadeOverlay;
- (void)animateShadeOverlay:(CGFloat)alpha;

@end
