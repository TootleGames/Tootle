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

#include "OFISerializer.h"
#include "OFHashedString.h"

OFImplementRTTI(OFISerializer, OFSmartObject);

const char* const OFISerializer::NoScope = 0;

OFISerializer::OFISerializer()
: mIsSerializingResourcesExternally(true)
{
}

void OFISerializer::serialize(const char* keyName, const char* elementKeyName, std::vector<bool>& container)
{
	serializePodVector(keyName, elementKeyName, container);
}

void OFISerializer::serialize(const char* keyName, const char* elementKeyName, std::vector<float>& container)
{
	serializePodVector(keyName, elementKeyName, container);
}

void OFISerializer::serialize(const char* keyName, const char* elementKeyName, std::vector<int>& container)
{
	serializePodVector(keyName, elementKeyName, container);
}

void OFISerializer::serialize(const char* keyName, const char* elementKeyName, std::vector<OFRetainedPtr<NSString> >& container)
{
	serializePodVector(keyName, elementKeyName, container);
}
	
void OFISerializer::pushScope(const char* scopeName, bool containsSeries)
{	
	OFPointer<OFISerializerKey> scope = createKey(scopeName);
	
	mActiveScopes.push_back(ScopeDescriptor(scope, containsSeries));
	onScopePushed(scope);
}

void OFISerializer::popScope()
{
	ScopeDescriptor poppingScope = mActiveScopes.back();

	mActiveScopes.pop_back();
	onScopePopped(poppingScope.scopeName);
}

void OFISerializer::io(const char* keyName, OFHashedString& value)
{
	io(keyName, value.valueForSerialization());	
}

OFISerializer::Scope::Scope(OFISerializer* serializer, const char* scope, bool containsSeries)
: mScope(scope)
{
	if(mScope == NoScope)
	{
		return;
	}
	
	mSerializer = serializer;
	mSerializer->pushScope(scope, containsSeries);
}

OFISerializer::Scope::~Scope()
{
	if(mScope == NoScope)
	{
		return;
	}

	mSerializer->popScope();
}

void OFISerializer::setSerializeResourcesExternally(bool isSerializingResourcesExternally)
{
	mIsSerializingResourcesExternally = isSerializingResourcesExternally;
}

OFPointer<OFISerializer> OFISerializer::createSerializerForInnerStreamOfType(const OFRTTI* type)
{
	return this;
}

void OFISerializer::storeInnerStreamOfType(OFISerializer* innerResourceStream, const OFRTTI* type)
{
	OFAssert(innerResourceStream == this, "An alternative serializer was partially specified for inner streams");

	// citron note: do nothing
}

const OFISerializer::ScopeDescriptorSeries& OFISerializer::getActiveScopes() const
{
	return mActiveScopes;
}

bool OFISerializer::isCurrentScopeASeries() const
{
	return !mActiveScopes.empty() && mActiveScopes.back().containsSeries;
}

void OFISerializer::serializeNumElementsInScopeSeries(unsigned int& count)
{
	io("num_elements", count);
}

OFPointer<OFISerializerKey> OFISerializer::createKey(const char* keyName) const
{
	return new StringKey(keyName);
}

void OFISerializer::io(const char* keyName, bool& value)			{	nviIo(createKey(keyName), value); }
void OFISerializer::io(const char* keyName, int& value)			{	nviIo(createKey(keyName), value); }
void OFISerializer::io(const char* keyName, unsigned int& value)	{	nviIo(createKey(keyName), value); }
void OFISerializer::io(const char* keyName, int64_t& value)		{	nviIo(createKey(keyName), value); }
void OFISerializer::io(const char* keyName, float& value)			{	nviIo(createKey(keyName), value); }
void OFISerializer::io(const char* keyName, double& value)		{	nviIo(createKey(keyName), value); }
void OFISerializer::io(const char* keyName, std::string& value)	{	nviIo(createKey(keyName), value); }
	
#ifdef __OBJC__	
void OFISerializer::io(const char* keyName, OFRetainedPtr<NSString>& value) {	nviIo(createKey(keyName), value); }

void OFISerializer::io(const char* keyName, OFRetainedPtr<NSData>& value)
{
	nviIo(createKey(keyName), value);
}

void OFISerializer::io(const char* keyName, NSString* value) 
{ 
	OFRetainedPtr<NSString> retainedString(value);
	OFAssert(!isReading(), @"When reading you must use a retained pointer");
	io(keyName, retainedString); 
}
#endif