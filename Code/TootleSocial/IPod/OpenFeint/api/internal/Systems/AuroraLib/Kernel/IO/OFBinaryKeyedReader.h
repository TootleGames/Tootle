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

#include "OFInputSerializer.h"

class OFBinarySource;

class OFBinaryKeyedReader : public OFInputSerializer
{
OFDeclareRTTI;
public:
	OFBinaryKeyedReader(const char* filePath, bool serializeResourcesExternally = true);
	OFBinaryKeyedReader(OFPointer<OFBinarySource> dataSource, bool serializeResourcesExternally = true);
	~OFBinaryKeyedReader();

	bool supportsKeys() const;

protected:
	explicit OFBinaryKeyedReader(bool serializeResourcesExternally);
	void readFromSourceNow(OFPointer<OFBinarySource> dataSource);
	
private:
	void nviIo(OFISerializerKey* keyName, bool& value)							{ return setValueAtScope(keyName, value); } 
	void nviIo(OFISerializerKey* keyName, int& value)								{ return setValueAtScope(keyName, value); } 
	void nviIo(OFISerializerKey* keyName, unsigned int& value)					{ return setValueAtScope(keyName, value); } 
	void nviIo(OFISerializerKey* keyName, int64_t& value)							{ return setValueAtScope(keyName, value); } 	
	void nviIo(OFISerializerKey* keyName, float& value)							{ return setValueAtScope(keyName, value); } 
	void nviIo(OFISerializerKey* keyName, double& value)							{ return setValueAtScope(keyName, value); } 
	void nviIo(OFISerializerKey* keyName, std::string& value)					
	{
		DataItem* item = findItemAtCurrentScope(keyName);
		if(!item)
		{
			return;
		}

		value.assign(item->data.get(), item->mySize);
	} 
	
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value)	
	{ 
		DataItem* item = findItemAtCurrentScope(keyName);
		if(!item)
		{
			return;
		}
		
		value.reset([NSString stringWithCString:reinterpret_cast<char*>(item->data.get()) length:item->mySize]);
	} 
	
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSData>& value)	
	{
		OFAssert(false, @"NSData not implemented in OFBinaryKeyedReader");
	}
	
	void doCommonConstruction(OFPointer<OFBinarySource> dataSource, bool serializeResourcesExternally);
	void doCommonConstruction(OFPointer<OFBinarySource> dataSource);
		
	const OFRTTI* beginDecodeType();
	void endDecodeType();	
	void beginEncodeType(const OFRTTI* typeToEncode);
	void endEncodeType();

	void onScopePushed(OFISerializerKey* scopeName);
	void onScopePopped(OFISerializerKey* scopeName);
	
	class DataItem : public OFSmartObject
	{
	public:
		DataItem() : hasBeenRead(false) {}
		
		typedef std::multimap<OFPointer<OFISerializerKey>, OFPointer<DataItem>, OFISerializerKey::PredicateLessThan > Table;
		
		unsigned int mySize;
		std::auto_ptr<char> data;
		bool hasBeenRead;
	};
	
	class Scope : public OFSmartObject
	{
	public:
		Scope() : hasBeenRead(false) {}
		
		typedef std::multimap<OFPointer<OFISerializerKey>, OFPointer<Scope>, OFISerializerKey::PredicateLessThan > Table;

		Table scopes;
		DataItem::Table items;
		bool hasBeenRead;
	};	
	
	class StaticStringKey : public OFISerializerKey
	{
	public:
		static const unsigned int keySize = 64;
		
		StaticStringKey(unsigned int size) { memset(value, 0, sizeof(char) * keySize);  OFAssert(size < keySize - 1, "Your key size is too large!"); }		
		
		bool equals(const OFISerializerKey* rhv) const			{ return strcmp(value, ((const StaticStringKey*)rhv)->value) == 0; }
		bool lessthan(const OFISerializerKey* rhv) const		{ return strcmp(value, ((const StaticStringKey*)rhv)->value) < 0; }
		const char* getAsString() const							{ return value; }
		
		char value[keySize];
	};
			
	virtual void readKey(OFPointer<OFBinarySource> dataSource, OFPointer<OFISerializerKey>& outKey);
	static OFPointer<DataItem> readData(OFPointer<OFBinarySource> dataSource);

	template <typename T>
	void setValueAtScope(OFISerializerKey* keyName, T& value);
	OFPointer<DataItem> findItemAtCurrentScope(OFISerializerKey* keyName);
	OFPointer<Scope> findScopeAtCurrentScope(OFISerializerKey* keyName);
	
	OFPointer<Scope> mDocumentRoot;
	std::vector<OFPointer<Scope> > mActiveScopes;
};

template <typename T>
void OFBinaryKeyedReader::setValueAtScope(OFISerializerKey* keyName, T& value)
{
	DataItem* item = findItemAtCurrentScope(keyName);
	if(!item)
	{
		return;
	}
	
	value = *reinterpret_cast<T*>(item->data.get());
}