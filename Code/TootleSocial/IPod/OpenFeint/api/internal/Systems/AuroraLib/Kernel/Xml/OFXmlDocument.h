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

#pragma once

#import <UIKit/UIKit.h>

class OFXmlElement;

@interface OFXmlDocument : NSObject
{
	NSXMLParser* mParser;
	OFPointer<OFXmlElement> mDocumentRoot;
	
	std::vector<OFPointer<OFXmlElement> > mActiveElements;
}

+ (id)xmlDocumentWithString:(NSString*)str;
+ (id)xmlDocumentWithData:(NSData*)data;
- (id)initWithData:(NSData*)data;
- (bool)nextValueAtCurrentScopeWithKey:(const char*)keyName outValue:(NSString*&)outString;

- (NSString*)getCurrentScopeShortName;

- (NSString*)getElementValue:(const char*)targetElementFullName;

- (OFPointer<OFXmlElement>)readNextElement;

- (void)pushNextUnreadScope;
- (void)pushNextScope:(const char*)scopeName;
- (void)popScope;

- (bool)pushNextUnreadScopeWithNameIfAvailable:(const char*)scopeName;

@end
