//
//  OFTableSectionCellDescription.h
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

@class OFResource;

@interface OFTableSectionCellDescription : NSObject
{
@private
	OFResource* resource;
	NSString* controllerName;
}

@property (nonatomic, retain) OFResource* resource;
@property (nonatomic, retain) NSString* controllerName;

+ (id)cellController:(NSString*)controllerName andResource:(OFResource*)resource;

@end
