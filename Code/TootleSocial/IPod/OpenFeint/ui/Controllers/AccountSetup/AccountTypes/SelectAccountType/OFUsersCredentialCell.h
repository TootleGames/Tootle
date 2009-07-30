////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFTableCellHelper.h"
#import "OFCallbackable.h"

@class OFTableSequenceControllerHelper;

@interface OFUsersCredentialCell : OFTableCellHelper
{
@private
	OFTableSequenceControllerHelper* owner;
}

- (void)onResourceChanged:(OFResource*)resource;
+ (NSString*)getCredentialImage:(NSString*)credentialName;
+ (NSString*)getCredentialControllerName:(NSString*)credentialName;

@property (nonatomic, readonly, assign) IBOutlet OFTableSequenceControllerHelper* owner;


@end