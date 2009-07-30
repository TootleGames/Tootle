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

#import "OFBinaryKeyedReader.h"
#include "OFBinarySource.h"
#include "OFRTTIRepository.h"
#include "OFBinaryKeyed.h"

OFImplementRTTI(OFBinaryKeyedReader, OFInputSerializer);

OFBinaryKeyedReader::OFBinaryKeyedReader(bool serializeResourcesExternally)
{
	setSerializeResourcesExternally(serializeResourcesExternally);
}

void OFBinaryKeyedReader::readFromSourceNow(OFPointer<OFBinarySource> dataSource)
{
	doCommonConstruction(dataSource);
}

OFBinaryKeyedReader::OFBinaryKeyedReader(const char* filePath, bool serializeResourcesExternally)
{
	OFPointer<OFBinarySource> source(new OFBinaryFileSource(filePath));
	doCommonConstruction(source, serializeResourcesExternally);
}
	
OFBinaryKeyedReader::OFBinaryKeyedReader(OFPointer<OFBinarySource> dataSource, bool serializeResourcesExternally)
{	
	doCommonConstruction(dataSource, serializeResourcesExternally);
}

void OFBinaryKeyedReader::doCommonConstruction(OFPointer<OFBinarySource> dataSource)
{
	std::vector<OFPointer<Scope> > activeScopes;

	while(true)
	{
		char currentMarker;
		dataSource->read(&currentMarker, sizeof(char));
		
		if(dataSource->isEmpty())
		{
			break;
		}

		if(currentMarker == OFBinaryKeyed::ScopeMarkerBegin)
		{			
			OFPointer<OFISerializerKey> key;
			readKey(dataSource, key);
			
			if(activeScopes.empty())
			{
				mDocumentRoot.reset(new Scope());
				activeScopes.push_back(mDocumentRoot);
			}
			else
			{
				OFPointer<Scope> newScope = new Scope();
				activeScopes.back()->scopes.insert(Scope::Table::value_type(key, newScope));
				activeScopes.push_back(newScope);
			}							
		}
		else if(currentMarker == OFBinaryKeyed::DataMarker)
		{
			OFPointer<OFISerializerKey> key;
			readKey(dataSource, key);

			OFPointer<DataItem> newItem(readData(dataSource));
			activeScopes.back()->items.insert(DataItem::Table::value_type(key, newItem));
			
		}
		else if(currentMarker == OFBinaryKeyed::ScopeMarkerEnd)
		{							
			activeScopes.pop_back();
		}
		else
		{
			OFAssert(0, "Unknown marker. This is probably a corrupt file");
			return;
		}
	}
	
	dataSource.reset(0);
	
	mActiveScopes.push_back(mDocumentRoot);
}

void OFBinaryKeyedReader::doCommonConstruction(OFPointer<OFBinarySource> dataSource, bool serializeResourcesExternally)
{
	setSerializeResourcesExternally(serializeResourcesExternally);
	doCommonConstruction(dataSource);
}

OFBinaryKeyedReader::~OFBinaryKeyedReader()
{
}

const OFRTTI* OFBinaryKeyedReader::beginDecodeType()
{
	OFSdbmHashedString typeId;
	io("___type", typeId);	

	if(typeId == OFSdbmHashedString())
	{
		return NULL;
	}
	
	return OFRTTIRepository::Instance()->getType(typeId);
}

void OFBinaryKeyedReader::endDecodeType()
{
}

void OFBinaryKeyedReader::beginEncodeType(const OFRTTI* typeToEncode)
{
}

void OFBinaryKeyedReader::endEncodeType()
{
}

bool OFBinaryKeyedReader::supportsKeys() const
{
	return true;
}

void OFBinaryKeyedReader::readKey(OFPointer<OFBinarySource> dataSource, OFPointer<OFISerializerKey>& outKey)
{
	unsigned char keyLength = 0;
	dataSource->read(&keyLength, sizeof(keyLength));
	
	OFPointer<StaticStringKey> key = new StaticStringKey(keyLength);
	dataSource->read(&key->value[0], keyLength);
	
	outKey = key;
}

OFPointer<OFBinaryKeyedReader::DataItem> OFBinaryKeyedReader::readData(OFPointer<OFBinarySource> dataSource)
{
	OFPointer<DataItem> newItem(new DataItem());
	
	unsigned int dataSize = 0;
	dataSource->read(&dataSize, sizeof(dataSize));

	std::auto_ptr<char> data(new char[dataSize]);
	dataSource->read(data.get(), dataSize);

	newItem->data = data;
	newItem->mySize = dataSize;
	
	return newItem;
}

void OFBinaryKeyedReader::onScopePushed(OFISerializerKey* scopeName)
{
	Scope* nextScope = findScopeAtCurrentScope(scopeName);
	
	// even if the scope isn't found, we still push it. We will return unfound
	// for all of its child data and scopes, but still properly handle its siblings.
	mActiveScopes.push_back(nextScope);
}

void OFBinaryKeyedReader::onScopePopped(OFISerializerKey* scopeName)
{
	mActiveScopes.pop_back();
}

OFPointer<OFBinaryKeyedReader::DataItem> OFBinaryKeyedReader::findItemAtCurrentScope(OFISerializerKey* keyName)
{
	OFPointer<Scope> currentScope = mActiveScopes.back();
	if(!currentScope.get())
	{
		return NULL;
	}
	
	std::pair<DataItem::Table::iterator, DataItem::Table::iterator> sit = currentScope->items.equal_range(keyName);
	if(sit.first == sit.second)
	{
		return NULL; 
	}
	
	DataItem::Table::iterator itNextUnread = sit.first;
	for(; itNextUnread != sit.second; ++itNextUnread)
	{
		if(!itNextUnread->second->hasBeenRead)
		{
			itNextUnread->second->hasBeenRead = true;
			return itNextUnread->second;
		}
	}
	
	return NULL;
}

OFPointer<OFBinaryKeyedReader::Scope> OFBinaryKeyedReader::findScopeAtCurrentScope(OFISerializerKey* keyName)
{
	if(mActiveScopes.empty())
	{
		return NULL;
	}
	
	OFPointer<Scope> currentScope = mActiveScopes.back();
	if(!currentScope.get())
	{
		return NULL;
	}
	
	std::pair<Scope::Table::iterator, Scope::Table::iterator> sit = currentScope->scopes.equal_range(keyName);
	if(sit.first == sit.second)
	{
		return NULL; 
	}
	
	Scope::Table::iterator itNextUnread = sit.first;
	for(; itNextUnread != sit.second; ++itNextUnread)
	{
		if(!itNextUnread->second->hasBeenRead)
		{
			itNextUnread->second->hasBeenRead = true;
			return itNextUnread->second;
		}
	}
	
	return NULL;
}