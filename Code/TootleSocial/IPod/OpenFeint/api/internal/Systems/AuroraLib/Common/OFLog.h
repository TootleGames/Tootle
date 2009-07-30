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

#include <map>
#include "OFHashedString.h"

#if _DEBUG

#define DEBUG_SHOW_FULLPATH NO

enum OFLogLevel
{
	OFErrorLogLevel = 0,
	OFWarningLogLevel = 1,
	OFInfoLogLevel = 2,
	OFDebugLogLevel = 3,
};

@interface OFLogger : NSObject
{
	const char* mTypeString;
	OFSdbmHashedString mType;
	OFLogLevel mLevel;
	OFLogLevel mDefaultLevel;
	std::map<OFSdbmHashedString, OFLogLevel> mLevels;
}

+ (OFLogger*)Instance;
- (void)setDefaultLevel:(OFLogLevel)level;
- (void)setLevel:(const char*)strName level:(OFLogLevel)level;
- (bool)hasLevel:(const char*)strName level:(OFLogLevel)level;
- (void)output:(char const*)fileName lineNumber:(int)lineNumber input:(const char*)input, ...;
@end

#define OFLogDefaultLevel(lvl)				[[OFLogger Instance] setDefaultLevel:lvl]
#define OFLogLevel(type,lvl)				[[OFLogger Instance] setLevel:#type level:lvl]
#define OFLogWithLevel(lvl,type,format,...)	{ if ([[OFLogger Instance] hasLevel:#type level:lvl]) [[OFLogger Instance] output:__FILE__ lineNumber:__LINE__ input:(format), ##__VA_ARGS__]; }
#define OFLogError(type,format,...)			OFLogWithLevel(OFErrorLogLevel, type, format, ##__VA_ARGS__)
#define OFLogWarning(type,format,...)		OFLogWithLevel(OFWarningLogLevel, type, format, ##__VA_ARGS__)
#define OFLogInfo(type,format,...)			OFLogWithLevel(OFInfoLogLevel, type, format, ##__VA_ARGS__)
#define OFLogDebug(type,format,...)			OFLogWithLevel(OFDebugLogLevel, type, format, ##__VA_ARGS__)

#else //_DEBUG

#define OFLogDefaultLevel(lvl)				(void)0
#define OFLogLevel(type,lvl)				(void)0
#define OFLogWithLevel(lvl,type,format,...)	(void)0
#define OFLogError(type,format,...)			(void)0
#define OFLogWarning(type,format,...)		(void)0
#define OFLogInfo(type,format,...)			(void)0
#define OFLogDebug(type,format,...)			(void)0
	
#endif //_DEBUG
