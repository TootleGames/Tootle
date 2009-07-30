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

#import "OFDashboardNotificationView.h"
#import "OpenFeint+Private.h"
#import "OFNotificationInputResponse.h"

@implementation OFDashboardNotificationView

- (id)initWithText:(NSString*)text andInputResponse:(OFNotificationInputResponse*)inputResponse
{
	const float viewWidth = [OpenFeint isInLandscapeMode] ? [UIScreen mainScreen].bounds.size.height : [UIScreen mainScreen].bounds.size.width;
	CGRect noticeFrame = CGRectMake(0.f, 0.f, viewWidth, 21.f);
	self = [super initWithFrame:noticeFrame];
	if (self)
	{
		UIImage* bgImage = [[UIImage imageNamed:@"OpenFeintDashboardNotificationBackground.png"] stretchableImageWithLeftCapWidth:2 topCapHeight:0];
		UIImageView* bgView = [[[UIImageView alloc] initWithImage:bgImage] autorelease];
		bgView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
		bgView.frame = noticeFrame;
		[self addSubview:bgView];

		UIImage* disclosureImage = [UIImage imageNamed:@"OpenFeintNotificationDisclosureArrow.png"];

		UIFont* font = [UIFont systemFontOfSize:10.0f];

		CGSize infoSize = [text sizeWithFont:font constrainedToSize:CGSizeMake(viewWidth, bgImage.size.height)];
		infoSize.width += disclosureImage.size.width + 2.0f;
		infoSize.width = MIN(infoSize.width, viewWidth);

		CGRect labelFrame = noticeFrame;
		labelFrame.origin.x = (viewWidth - infoSize.width) * 0.5f;
		UILabel* noticeLabel = [[[UILabel alloc] initWithFrame:labelFrame] autorelease];
		noticeLabel.font = font;
		noticeLabel.text = text;
		noticeLabel.backgroundColor = [UIColor clearColor];
		noticeLabel.opaque = NO;
		[self addSubview:noticeLabel];

		UIImageView* disclosureView = [[[UIImageView alloc] initWithImage:disclosureImage] autorelease];
		disclosureView.autoresizingMask = UIViewAutoresizingNone;
		CGRect disclosureRect = disclosureView.frame;
		disclosureRect.origin.x = infoSize.width - disclosureImage.size.width + labelFrame.origin.x;
		disclosureRect.origin.y = noticeFrame.size.height * 0.5f - disclosureRect.size.height * 0.5f;
		disclosureView.frame = disclosureRect;
		[self addSubview:disclosureView];

		self.autoresizesSubviews = YES;
		self.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;

		mInputResponse = [inputResponse retain];
	}
	return self;
}

+ (OFDashboardNotificationView*)notificationWithText:(NSString*)text andInputResponse:(OFNotificationInputResponse*)inputResponse
{
	return [[[OFDashboardNotificationView alloc] initWithText:text andInputResponse:inputResponse] autorelease];
}

- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
	UIView* hitView = [super hitTest:point withEvent:event];
	if (hitView == self)
	{
		[mInputResponse respondToInput];
	}
	return hitView;
}

- (void)dealloc
{
	OFSafeRelease(mInputResponse);
	[super dealloc];
}

@end

