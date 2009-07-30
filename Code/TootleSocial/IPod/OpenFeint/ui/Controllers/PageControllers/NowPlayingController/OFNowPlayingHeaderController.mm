////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFNowPlayingHeaderController.h"
#import "OFNowPlayingController.h"
#import "OFViewHelper.h"
#import "OFImageView.h"
#import "OFApplicationDescription.h"
#import "OpenFeint+Private.h"

@implementation OFNowPlayingHeaderController

@synthesize nowPlayingController;
	
- (void)viewDidLoad
{
	[super viewDidLoad];

	UIImageView* bannerView = (UIImageView*)OFViewHelper::findViewByTag(self.view, 1);
	
	NSString* bannerImageName = [OpenFeint getBannerImageName];
	if (!bannerImageName || [bannerImageName length] == 0)
	{
		// default banner
		bannerImageName = @"OpenFeintLogoStripe.png";
		if ([OpenFeint isInLandscapeMode])
		{
			bannerImageName = @"OpenFeintLogoStripeLandscape.png";
		}
	}

	UIImage* bannerImage = [UIImage imageNamed:bannerImageName];
	bannerView.image = bannerImage;

	float bannerWidth = [nowPlayingController getBannerWidth];
	bannerView.frame = CGRectMake(
		(bannerWidth - bannerImage.size.width) * 0.5f, 
		0.0f, 
		bannerImage.size.width,
		bannerImage.size.height);

	CGRect myRect = CGRectMake(0.0f, 0.0f, [OpenFeint getDashboardBounds].size.width, bannerView.frame.size.height + 8.f);
	self.view.frame = myRect;
	
//	OFImageView* iconView = (OFImageView*)OFViewHelper::findViewByTag(self.view, 2);
//	if (bannerImage == defaultBannerImage)
//	{
//		float iconHeight = bannerView.frame.size.height;
//		
//		[iconView setDefaultImage:[UIImage imageNamed:@"OFDefaultApplicationIcon.png"]];
//
//		iconView.hidden = NO;
//		iconView.imageUrl = nowPlayingController.resource.iconUrl;
//		iconView.frame = CGRectMake((bannerWidth - iconHeight) * 0.5f, 0.f, iconHeight, iconHeight);
//	}
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end