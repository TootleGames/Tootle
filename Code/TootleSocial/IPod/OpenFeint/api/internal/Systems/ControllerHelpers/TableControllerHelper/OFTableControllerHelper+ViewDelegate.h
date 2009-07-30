//
//  OFTableControllerHelper+ViewDelegate.h
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFTableControllerHelper.h"

@interface OFTableControllerHelper (ViewDelegate)

- (UITableViewCell*)getCellForIndex:(NSIndexPath*)indexPath inTable:(UITableView*)tableView useHelperCell:(BOOL)useHelperCell;
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath;
- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath;

@end
