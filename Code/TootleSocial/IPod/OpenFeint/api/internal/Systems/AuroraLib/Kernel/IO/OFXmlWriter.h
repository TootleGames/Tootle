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

#include "OFOutputSerializer.h"

class OFXmlWriter : public OFOutputSerializer
{
OFDeclareRTTI;
public:
	OFXmlWriter(const char* rootNodeName);
	OFXmlWriter();

	bool supportsKeys() const;
	
	OFRetainedPtr<NSString> buildAndGetDocument() const;
	
private:
	void nviIo(OFISerializerKey* keyName, bool& value);
	void nviIo(OFISerializerKey* keyName, int& value);
	void nviIo(OFISerializerKey* keyName, unsigned int& value);
	void nviIo(OFISerializerKey* keyName, float& value);
	void nviIo(OFISerializerKey* keyName, double& value);
	void nviIo(OFISerializerKey* keyName, std::string& value);	
	void nviIo(OFISerializerKey* keyName, OFRetainedPtr<NSString>& value);	

	const OFRTTI* beginDecodeType();
	void endDecodeType();	
	void beginEncodeType(const OFRTTI* typeToEncode);
	void endEncodeType();
	
	void onScopePushed(OFISerializerKey* scopeName);
	void onScopePopped(OFISerializerKey* scopeName);
	
	class Node : public OFSmartObject
	{
	public:
		Node(OFISerializerKey* _name, OFRetainedPtr<NSString> _value) : name(_name), value(_value) {}
		Node(OFISerializerKey* _name) : name(_name), value(0) {}

		OFRetainedPtr<NSString> createSubtree() const;

		OFPointer<OFISerializerKey> name;
		OFRetainedPtr<NSString> value;
		std::vector<OFPointer<Node> > children;
	};

	void attachNode(OFPointer<Node> nodeToAttach);

	OFPointer<Node> mDocumentRoot;
	std::vector<OFPointer<Node> > mActiveNodes;
};