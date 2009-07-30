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

class OFISerializer;

class OFTransactionalSaveFile : public OFSmartObject
{
public:
	OFTransactionalSaveFile() {}
	
	OFTransactionalSaveFile(
		OFRetainedPtr<NSString> fileName,
		OFRetainedPtr<NSObject> delegate,
		SEL onLoad,
		SEL onSave,
		SEL onFailedPreviousSave
	);
	
	void attemptLoad();
	void writeAndCommit();

	void writeNewFile();
	void commitNewFile();
	
	static NSString* getSavePathForFile(NSString* fileName);
	
private:
	NSString* getNewFileFullPath() const;
	NSString* getBackupPath() const;
	
	OFRetainedPtr<NSString> mFileName;
	OFRetainedPtr<NSObject> mDelegate;
	SEL mOnLoad;
	SEL mOnSave;
	SEL mOnFailedPreviousSave;
};

///////////////////////////////////////////////////////////////////////////

@interface OFISerializerOCWrapper : NSObject
{
	OFISerializer* mStream;
}

@property (readonly) OFISerializer* stream;

- (id) initWithStream:(OFISerializer*)stream;

@end