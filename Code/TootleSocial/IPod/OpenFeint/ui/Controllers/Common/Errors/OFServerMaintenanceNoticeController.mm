////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFServerMaintenanceNoticeController.h"
#import "OFControllerLoader.h"
#import "OFXmlDocument.h"
#import "OFActionRequest.h"

@implementation OFServerMaintenanceNoticeController

+ (id)maintenanceControllerWithHtmlData:(NSData*)data
{
	OFServerMaintenanceNoticeController* controller = (OFServerMaintenanceNoticeController*)OFControllerLoader::load(@"ServerMaintenanceNotice");
	
	OFXmlDocument* errorDocument = [OFXmlDocument xmlDocumentWithData:data];	
	controller.message = [errorDocument getElementValue:"server_interruption_notice"];
	
	return controller;
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	self.navigationItem.hidesBackButton = YES;
	self.title = @"Maintenance";
	self.messageView.font = [UIFont systemFontOfSize:14.0f];
}

@end
