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

#include "OFDependencies.h"
#include "OFRetainedPtr.h"
#include "OFISerializer.h"


class OFHighScoreBatchEntry : public OFSmartObject
{
public:
	OFHighScoreBatchEntry(OFISerializer* stream);
	OFHighScoreBatchEntry(NSString* _leaderboardId, int64_t _score);
	OFHighScoreBatchEntry();
	void serialize(OFISerializer* stream);
	
public:
	OFRetainedPtr<NSString> leaderboardId;
	int64_t score;
	
};

typedef std::vector<OFPointer<OFHighScoreBatchEntry> >  OFHighScoreBatchEntrySeries;
