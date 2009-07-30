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

#import "OFLog.h"

#if _DEBUG

@implementation OFLogger

static OFLogger *sharedDebugInstance = nil;

/*---------------------------------------------------------------------*/
+ (OFLogger *) Instance
{
	@synchronized(self)
	{
		if (sharedDebugInstance == nil)
		{
			[[self alloc] init];
		}
	}
	return sharedDebugInstance;
}

/*---------------------------------------------------------------------*/
+ (id) allocWithZone:(NSZone *) zone
{
	@synchronized(self)
	{
		if (sharedDebugInstance == nil)
		{
			sharedDebugInstance = [super allocWithZone:zone];
			return sharedDebugInstance;
		}
	}
	return nil;
}

/*---------------------------------------------------------------------*/
- (id)copyWithZone:(NSZone *)zone
{
	return self;
}

/*---------------------------------------------------------------------*/
- (id)retain
{
	return self;
}

/*---------------------------------------------------------------------*/
- (void)release
{
	// No action required...
}

/*---------------------------------------------------------------------*/
- (unsigned)retainCount
{
	return UINT_MAX;  // An object that cannot be released
}

/*---------------------------------------------------------------------*/
- (id)autorelease
{
	return self;
}

/*---------------------------------------------------------------------*/
- (void)setDefaultLevel:(OFLogLevel)level
{
	mDefaultLevel = level;
}

/*---------------------------------------------------------------------*/
- (void)setLevel:(const char*)strName level:(OFLogLevel)level
{
	OFSdbmHashedString type(strName);
	mLevels[type] = level;
}

/*---------------------------------------------------------------------*/
- (bool)hasLevel:(const char*)strName level:(OFLogLevel)level
{
	OFSdbmHashedString type(strName);
	int typeLevel = mDefaultLevel;
	std::map<OFSdbmHashedString, OFLogLevel>::iterator itr = mLevels.find(type);
	if (itr != mLevels.end())
	{
		typeLevel = itr->second;
	}
	
	if (typeLevel >= level)
	{
		mTypeString = strName;
		mType = type;
		mLevel = level;
		return true;
	}
	return false;
}

/*---------------------------------------------------------------------*/
- (void)output:(char const*)fileName lineNumber:(int)lineNumber input:(const char*)input, ...
{
	va_list argList;
	NSString *filePath, *formatStr, *nsInput;
	
	// Build the path string
	filePath = [[NSString alloc] initWithBytes:fileName length:strlen(fileName) encoding:NSUTF8StringEncoding];
	nsInput = [[NSString alloc] initWithBytes:input length:strlen(input) encoding:NSUTF8StringEncoding];
	
	// Process arguments, resulting in a format string
	va_start(argList, input);
	formatStr = [[NSString alloc] initWithFormat:nsInput arguments:argList];
	va_end(argList);
	
	// Call NSLog, prepending the filename and line number
	char const* levelStr = "";
	if (mLevel == OFErrorLogLevel)
		levelStr = " ERROR:";
	else if (mLevel == OFWarningLogLevel)
		levelStr = " WARNING:";
	OFLog(@"[%s]%s %@ (%s:%d)", mTypeString, levelStr, formatStr, [((DEBUG_SHOW_FULLPATH) ? filePath : [filePath lastPathComponent]) UTF8String], lineNumber);
	
	[filePath release];
	[formatStr release];
	[nsInput release];
}

@end

#endif //_DEBUG
