//
//  OFTableSequenceControllerLoadMoreCell.h
//  OpenFeint
//
//  Created by Jason Citron on 5/29/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFTableCellHelper.h"

@class OFPaginatedSeriesHeader;

@interface OFTableSequenceControllerLoadMoreCell : OFTableCellHelper
{
	OFPaginatedSeriesHeader* lastLoadedPageHeader;
}

- (void)onResourceChanged:(OFResource*)resource;

@property (assign, nonatomic) bool isLoading;
@property (retain, nonatomic) OFPaginatedSeriesHeader* lastLoadedPageHeader;

@end
