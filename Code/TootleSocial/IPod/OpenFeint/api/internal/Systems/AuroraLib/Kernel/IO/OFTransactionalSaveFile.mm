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

#include "OFTransactionalSaveFile.h"
#include "OFBinarySdbmKeyedWriter.h"
#include "OFBinarySdbmKeyedReader.h"
#include "OFBinarySink.h"
#include "OFBinarySource.h"

OFTransactionalSaveFile::OFTransactionalSaveFile(
	OFRetainedPtr<NSString> fileName,
	OFRetainedPtr<NSObject> delegate,
	SEL onLoad,
	SEL onSave,
	SEL onFailedPreviousSave
)
: mFileName(fileName)
, mDelegate(delegate)
, mOnLoad(onLoad)
, mOnSave(onSave)
, mOnFailedPreviousSave(onFailedPreviousSave)
{
}

NSString* OFTransactionalSaveFile::getBackupPath() const
{
	return getSavePathForFile([NSString stringWithFormat:@"%@.bak", mFileName.get()]);
}

NSString* OFTransactionalSaveFile::getSavePathForFile(NSString* fileName)
{
	NSArray* folders = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);	
	return [[folders objectAtIndex:0] stringByAppendingPathComponent:fileName];
}

void OFTransactionalSaveFile::attemptLoad()
{
	NSString* backupFilePath = getBackupPath();
	NSString* newFilePath = getNewFileFullPath();
	NSString* liveFilePath = getSavePathForFile(mFileName.get());
		
	NSFileManager* fileManager = [NSFileManager defaultManager];
	
	if([fileManager fileExistsAtPath:newFilePath])
	{
		OFLog(@"Detected previous save failed (%@). Continuing anyway...", mFileName.get());
		[mDelegate.get() performSelector:mOnFailedPreviousSave withObject:nil];
	}
	
	
	NSString* fileToLoad = liveFilePath;
	if(![fileManager fileExistsAtPath:liveFilePath])
	{	
		OFLog(@"Missing live version of file (%@). Attempting to load archive...", mFileName.get());
		
		if([fileManager fileExistsAtPath:backupFilePath])
		{
			fileToLoad = backupFilePath;
		}
		else
		{
			// We could attempt to load an archive here. But, realistically, this will never happen.
			OFLog(@"Failed to find a valid file to load %@. Critical Error.", mFileName.get());
		}
	}

	std::auto_ptr<OFISerializer> stream(new OFBinarySdbmKeyedReader(new OFBinaryFileSource([fileToLoad UTF8String])));
	[mDelegate.get() performSelector:mOnLoad withObject:[[[OFISerializerOCWrapper alloc] initWithStream:stream.get()] autorelease]];
}

void OFTransactionalSaveFile::writeAndCommit()
{
	writeNewFile();
	commitNewFile();
}	

NSString* OFTransactionalSaveFile::getNewFileFullPath() const
{
	return getSavePathForFile([NSString stringWithFormat:@"%@.new", mFileName.get()]);
}

void OFTransactionalSaveFile::writeNewFile()
{
	NSString* fileName = getNewFileFullPath();
		
	std::auto_ptr<OFISerializer> stream(new OFBinarySdbmKeyedWriter(new OFBinaryFileSink([fileName UTF8String])));
	[mDelegate.get() performSelector:mOnSave withObject:[[[OFISerializerOCWrapper alloc] initWithStream:stream.get()] autorelease]];
}

void OFTransactionalSaveFile::commitNewFile()
{
	NSString* backupFilePath = getBackupPath();
	NSString* newFilePath = getNewFileFullPath();
	NSString* liveFilePath = getSavePathForFile(mFileName.get());
		
	NSFileManager* fileManager = [NSFileManager defaultManager];

	NSError* error = nil;

	if([fileManager fileExistsAtPath:backupFilePath])
	{
		NSDateFormatter* dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
		[dateFormatter setDateStyle:NSDateFormatterLongStyle];
		[dateFormatter setTimeStyle:NSDateFormatterLongStyle];

		NSString* archiveFileName = [NSString stringWithFormat:@"%@.archive.%@", mFileName.get(), [dateFormatter stringFromDate:[NSDate date]]];
		OFLog(@"Detected previous commit has failed on file: %@. Moving out of the way as timestamped archive: %@", mFileName.get(), archiveFileName);

		NSString* archiveFilePath = getSavePathForFile(archiveFileName);		
		[fileManager moveItemAtPath:backupFilePath toPath:archiveFilePath error:&error];
	}
	
	if(![fileManager moveItemAtPath:liveFilePath toPath:backupFilePath error:&error])
	{
		OFLog(@"Failed backing up %@. Error: %@", mFileName.get(), [error localizedDescription]);
	}
	
	if(![fileManager moveItemAtPath:newFilePath toPath:liveFilePath error:&error])
	{
		OFLog(@"Failed committing new version. Restoring previous. (%@) Error: %@", mFileName.get(), [error localizedDescription]);
		
		if(![fileManager moveItemAtPath:backupFilePath toPath:liveFilePath error:&error])
		{
			OFLog(@"Failed restoring previous version. A critical error has occurred. (%@) Error: %@", mFileName.get(), [error localizedDescription]);
		}
		
		return;
	}
	
	if(![fileManager removeItemAtPath:backupFilePath error:&error])
	{
		OFLog(@"Failed removing backup of %@. Error: %@", mFileName.get(), [error localizedDescription]);
		return;
	}
}

@implementation OFISerializerOCWrapper

@synthesize stream = mStream;

- (id) initWithStream:(OFISerializer*)stream
{
	if(self = [super init])
	{
		mStream = stream;
	}
	return self;
}

@end

