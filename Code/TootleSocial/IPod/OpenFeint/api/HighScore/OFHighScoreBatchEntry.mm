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


#include "OFHighScoreBatchEntry.h"

OFHighScoreBatchEntry::OFHighScoreBatchEntry(OFISerializer* stream)
{
	serialize(stream);
}

OFHighScoreBatchEntry::OFHighScoreBatchEntry(NSString* _leaderboardId, int64_t _score)
: leaderboardId(_leaderboardId)
, score(_score)
{
}

OFHighScoreBatchEntry::OFHighScoreBatchEntry()
: leaderboardId(nil)
, score(0)
{
}

void OFHighScoreBatchEntry::serialize(OFISerializer* stream)
{
	stream->io("leaderboard_id", leaderboardId);
	stream->io("score", score);
}
