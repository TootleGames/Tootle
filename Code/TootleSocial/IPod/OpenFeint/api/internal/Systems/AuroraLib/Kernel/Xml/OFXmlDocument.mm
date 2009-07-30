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

#import "OFXmlDocument.h"
#include "OFXmlElement.h"

@implementation OFXmlDocument

+ (id)xmlDocumentWithData:(NSData*)data
{
	return [[[OFXmlDocument alloc] initWithData:data] autorelease];
}

+ (id)xmlDocumentWithString:(NSString*)str
{
	return [OFXmlDocument xmlDocumentWithData:[str dataUsingEncoding:NSUTF8StringEncoding]];
}

- (id)initWithData:(NSData*)data
{
	self = [super init];
	if(self)
	{
		mParser = [[NSXMLParser alloc] initWithData:data];
		
		[mParser setDelegate:self];
		[mParser setShouldProcessNamespaces:NO];
		[mParser setShouldReportNamespacePrefixes:NO];
		[mParser setShouldResolveExternalEntities:NO];
		
		OFPointer<OFXmlElement> sentinel(new OFXmlElement(@"root"));
		mActiveElements.push_back(sentinel);
		[mParser parse];
		NSError* parseError = [mParser parserError];
		if(parseError)
		{
			OFLog(@"Error parsing XML document, %@", [parseError localizedDescription]);
			OFLog(@"%s", [data bytes]);
			//OFAssert(parseError.code == 5, "Error parsing XML document: %@", [parseError localizedDescription]);
		}
		else
		{
			mDocumentRoot = sentinel->getChildAt(0);
		}
	}

	return self;
}

- (NSString*)getElementValue:(const char*)targetElementFullName
{
	NSArray* elementPath = [[NSString stringWithCString:targetElementFullName] componentsSeparatedByString:@"."];
	OFXmlElement* currentNode = mDocumentRoot.get();
	
	if(currentNode)
	{
		const unsigned int numElementsInPath = [elementPath count];
		
		for(unsigned int i = 1; i < numElementsInPath; ++i)
		{
			NSString* currentName = [elementPath objectAtIndex:i];
			
			currentNode = currentNode->getChildWithName(currentName);
			if(currentNode == NULL)
			{
				break;
			}
		}
		
		if(currentNode)
		{
			return currentNode->getValue();
		}
	}
		
	return @"";
}

- (OFPointer<OFXmlElement>)readNextElement
{
	if(mActiveElements.back())
	{
		return mActiveElements.back()->dequeueNextUnreadChild();
	}
	
	return NULL;
}

- (void)pushNextScope:(const char*)scopeName
{	
	OFPointer<OFXmlElement> child;

	if(mActiveElements.size() == 0)
	{
		child = mDocumentRoot;
	}
	else if(mActiveElements.back().get() != NULL)
	{
		child = mActiveElements.back()->dequeueNextUnreadChild(scopeName);
	}
	 
	mActiveElements.push_back(child);
}

- (void)popScope
{
	mActiveElements.pop_back();
}

- (void)pushNextUnreadScope
{
	if(mActiveElements.back().get() != NULL)
	{
		mActiveElements.push_back(mActiveElements.back()->dequeueNextUnreadChild());
	}
}

- (bool)pushNextUnreadScopeWithNameIfAvailable:(const char*)scopeName
{
	if(mActiveElements.back().get() != NULL)
	{
		OFPointer<OFXmlElement> nextElement = mActiveElements.back()->dequeueNextUnreadChild(scopeName);		
		if(nextElement.get())
		{
			mActiveElements.push_back(nextElement);
			return true;
		}
	}
	
	return false;
}

- (NSString*)getCurrentScopeShortName
{
	if(mActiveElements.empty())
	{
		return @"";
	}
	
	return mActiveElements.back()->getName();
}

- (bool)nextValueAtCurrentScopeWithKey:(const char*)keyName outValue:(NSString*&)outString
{
	if(mActiveElements.back().get() != NULL)
	{
		const bool isValid = mActiveElements.back()->getValueWithName(keyName, outString, true);
		
		//OFLog(@"getting value from %s", keyName);

		return isValid;
	}
	else
	{
		//OFLog(@"getting value from %s --- INVALID SCOPE ---", keyName);
		return false;
	}
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
	//OFLog(@"started parsing %@", elementName);
	if(qName)
	{
		elementName = qName;
	}
	
	OFPointer<OFXmlElement> newElement(new OFXmlElement(elementName));
	if([attributeDict count] > 0)
	{
		newElement->setAttributes(attributeDict);
	}
	
	mActiveElements.back()->addChild(newElement);
	mActiveElements.push_back(newElement);
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
	//OFLog(@"finished parsing %@", elementName);
	if (mActiveElements.back()->hasNilValue())
	{
		mActiveElements.back()->setValue(@"");
	}
	mActiveElements.pop_back();
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
	if(mActiveElements.back()->hasValue())
	{
		string = [mActiveElements.back()->getValue() stringByAppendingString:string];
	}

	mActiveElements.back()->setValue(string);
}

- (void)dealloc
{
	[mParser release];
	[super dealloc];
}

@end
