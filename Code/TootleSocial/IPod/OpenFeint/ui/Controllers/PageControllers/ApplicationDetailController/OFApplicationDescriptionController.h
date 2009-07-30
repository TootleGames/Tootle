////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

@class UIWebView;
@class UIActivityIndicatorView;

@interface OFApplicationDescriptionController : UIViewController<UIWebViewDelegate>
{
	NSString* resourceId;
	UIWebView* mWebView;
	UIActivityIndicatorView* mLoadingView;	
}

+ (id)applicationDescriptionForId:(NSString*)resourceId;

@property (nonatomic, retain) NSString* resourceId;

@end
