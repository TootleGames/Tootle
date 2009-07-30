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

class OFSqlQuery
{
OFDeclareNonCopyable(OFSqlQuery);
	
public:
	OFSqlQuery();
	OFSqlQuery(struct sqlite3* dbHandle, const char* queryString);
	~OFSqlQuery();
	
	void reset(struct sqlite3* dbHandle, const char* queryString);
	void destroyQueryNow();
	
	// ------------------------------------------------------------
	// These should be invoked before calling execute to set named
	// parameters (if any) in the query
	// ------------------------------------------------------------
	void bind(const char* namedParameter, NSString* value);
	void bind(const char* namedParameter, const void* value, unsigned int size);
		
	// ------------------------------------------------------------
	// Sample multi-row usage:
	//
	//	OFSqlQuery example(...)
	// ...
	// while(example.execute())
	// {
	//		// Do stuff
	// }
	//
	// ------------------------------------------------------------		
	bool execute();
	void resetQuery();
	bool hasReachedEnd();
	void step();
	
	int getLastStepResult() { return mLastStepResult; }

	// ------------------------------------------------------------
	// These act on the current row in the result set
	// ------------------------------------------------------------
	int getInt(const char* columnName);
	int getBool(const char* columnName);
	const char* getText(const char* columnName);
	void getBlob(const char* columnName, const char*& blobData, unsigned int& blobSizeInBytes);
	
private:
	unsigned int safeGetParamIndex(const char* namedParameter) const;
	unsigned int safeGetColumnIndex(const char* columnName) const;
	void ensureQueryIsReset();
	
	int mLastStepResult;
	struct sqlite3* mDbHandle;
	struct sqlite3_stmt* mCompiledStatement;
	const char* mQueryString;
	
	std::vector<OFSdbmHashedString> mColumnIndices;
};
