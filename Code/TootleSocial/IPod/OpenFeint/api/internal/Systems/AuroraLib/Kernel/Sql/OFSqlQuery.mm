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


#include "OFSqlQuery.h"

#include <sqlite3.h>

#define SQLITE_CHECK(x)											\
	{																	\
		int result = x;											\
		if(result != SQLITE_OK)									\
		{																\
			OFLog(@"Failed executing: %s", ""#x);			\
			OFLog(@"   Result code: %d", result);			\
			OFLog(@"   %s", sqlite3_errmsg(mDbHandle));	\
			OFLog(@"   in query: %s", mQueryString);		\
			OFAssert(0, "");										\
		}																\
	}

OFSqlQuery::OFSqlQuery()
: mDbHandle(0)
, mCompiledStatement(0)
, mQueryString(NULL)
, mLastStepResult(SQLITE_OK)
{
}

OFSqlQuery::OFSqlQuery(sqlite3* dbHandle, const char* queryString)
: mDbHandle(dbHandle)
, mCompiledStatement(0)
, mQueryString(queryString)
, mLastStepResult(SQLITE_OK)
{		
	reset(dbHandle, queryString);
}

void OFSqlQuery::reset(sqlite3* dbHandle, const char* queryString)
{	
	destroyQueryNow();
	
	mDbHandle = dbHandle;
	mQueryString = queryString;
	mLastStepResult = SQLITE_OK;

	const unsigned int queryLength = strlen(queryString);
	SQLITE_CHECK(sqlite3_prepare_v2(dbHandle, queryString, queryLength, &mCompiledStatement, NULL));
	
	mColumnIndices.clear();
	const unsigned int numColumnsInRow = sqlite3_column_count(mCompiledStatement);
	for(unsigned int i = 0; i < numColumnsInRow; ++i)
	{	
		mColumnIndices.push_back(OFSdbmHashedString(sqlite3_column_name(mCompiledStatement, i)));
	}	
}

OFSqlQuery::~OFSqlQuery()
{
	destroyQueryNow();
}

void OFSqlQuery::destroyQueryNow()
{
	if(mCompiledStatement)
	{
		sqlite3_finalize(mCompiledStatement);
	}
	
	mCompiledStatement = NULL;
	mDbHandle = NULL;	
	mQueryString = NULL;
}

bool OFSqlQuery::execute()
{	
	mLastStepResult = sqlite3_step(mCompiledStatement);

	if(!(mLastStepResult == SQLITE_OK ||
		 mLastStepResult == SQLITE_ROW || 
		 mLastStepResult == SQLITE_DONE ||
		 mLastStepResult == SQLITE_CONSTRAINT))
	{
		OFLog(@"Failed stepping query");
		OFLog(@"   Result code: %d", mLastStepResult);	
		OFLog(@"   %s", sqlite3_errmsg(mDbHandle));
		OFLog(@"   in query: %s", mQueryString);	
		OFAssert(0, "");
	}

	if(mLastStepResult == SQLITE_DONE)
	{
		resetQuery();
	}

	return mLastStepResult == SQLITE_ROW;
}

void OFSqlQuery::resetQuery()
{
	sqlite3_reset(mCompiledStatement);
	mLastStepResult = SQLITE_OK;
}

bool OFSqlQuery::hasReachedEnd()
{
	return mLastStepResult != SQLITE_ROW;
}

void OFSqlQuery::step()
{
	execute();
}

int OFSqlQuery::getInt(const char* columnName)
{
	const unsigned int columnIndex = safeGetColumnIndex(columnName);
	return sqlite3_column_int(mCompiledStatement, columnIndex);
}

int OFSqlQuery::getBool(const char* columnName)
{
	return getInt(columnName) != 0;
}

const char* OFSqlQuery::getText(const char* columnName)
{
	const unsigned int columnIndex = safeGetColumnIndex(columnName);
	return (const char*)sqlite3_column_text(mCompiledStatement, columnIndex);
}

void OFSqlQuery::getBlob(const char* columnName, const char*& blobData, unsigned int& blobSizeInBytes)
{
	const unsigned int columnIndex = safeGetColumnIndex(columnName);
	
	blobData = static_cast<const char*>(sqlite3_column_blob(mCompiledStatement, columnIndex));
	blobSizeInBytes = sqlite3_column_bytes(mCompiledStatement, columnIndex);
}

void OFSqlQuery::bind(const char* namedParameter, NSString* value)
{	
	ensureQueryIsReset();
	
	SQLITE_CHECK(sqlite3_bind_text(mCompiledStatement, safeGetParamIndex(namedParameter), [value UTF8String], -1, SQLITE_TRANSIENT));
}

void OFSqlQuery::bind(const char* namedParameter, const void* value, unsigned int valueSize)
{	
	ensureQueryIsReset();
	
	SQLITE_CHECK(sqlite3_bind_blob(mCompiledStatement, safeGetParamIndex(namedParameter), value, valueSize, SQLITE_TRANSIENT));
}

void OFSqlQuery::ensureQueryIsReset()
{
}

unsigned int OFSqlQuery::safeGetParamIndex(const char* namedParameter) const
{
	unsigned int index =	sqlite3_bind_parameter_index(mCompiledStatement, [[NSString stringWithFormat:@":%s", namedParameter] UTF8String]);
	if(index == 0)
	{
		OFLog(@"Invalid named parameter in query: %s", mQueryString);
		OFAssert(0, "");
	}
	
	return index;
}

unsigned int OFSqlQuery::safeGetColumnIndex(const char* columnName) const
{	
	const OFSdbmHashedString hashedName(columnName);
	for(unsigned int i = 0; i < mColumnIndices.size(); ++i)
	{
		if(mColumnIndices[i] == hashedName)
		{
			return i;
		}
	}
	
	OFLog(@"Invalid column name in result-set for: %s", mQueryString);
	OFAssert(0, "");
	return mColumnIndices.size();
}