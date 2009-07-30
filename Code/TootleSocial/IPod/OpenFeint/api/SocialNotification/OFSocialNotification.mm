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

#import "OFSocialNotification.h"

@implementation OFSocialNotification

@synthesize text;
@synthesize imageType;
@synthesize imageIdentifier;
@synthesize imageUrl;

-(id)initWithText:(NSString*)_text imageType:(NSString*)_imageType imageIdentifier:(NSString*)_imageIdentifier
{
	if(self = [super init])
	{
		self.text = _text;
		self.imageType = _imageType;
		self.imageIdentifier = _imageIdentifier;
		self.imageUrl = nil;
	}
	return self;
}

-(id)initWithText:(NSString*)_text imageNamed:(NSString*)_imageName
{
	return [self initWithText:_text imageType:@"notification_images" imageIdentifier:_imageName];
}

-(id) initWithText:(NSString*)_text
{
	return [self initWithText:_text imageType:nil imageIdentifier:nil];
}

-(id)initWithText:(NSString*)_text imageType:(NSString*)_imageType imageId:(NSString*)_imageId
{
	return [self initWithText:_text imageType:_imageType imageIdentifier:_imageId];
}

- (void)dealloc
{
	self.text = nil;
	self.imageType = nil;
	self.imageIdentifier = nil;
	self.imageUrl = nil;
	[super dealloc];
}

@end