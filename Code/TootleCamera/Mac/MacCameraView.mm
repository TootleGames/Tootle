/*
 *  MacCameraView.cpp
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "MacCameraView.h"


@implementation MacCameraView


@end


@implementation MacCameraViewDelegate



-(id)init
{
	m_pPublisherProxy = new TPublisherProxy;
	
	return [super init];
}

-(void)dealloc
{
	if(m_pPublisherProxy)
	{
		delete m_pPublisherProxy;
		m_pPublisherProxy = nil;
	}
	
	[super dealloc];
}

// Intercept the image from the camera that will be displayed.
// We can do some post processing on the image data at this point and display something else
// or if we return nil it will display the image as-is
- (CIImage *)view:(QTCaptureView *)view willDisplayImage:(CIImage *)image
{
	// Convert the CIImage into some binary data we can use as part of a texture asset
	// Send the data out to the camera manager so it can update the camera texture
	//TBinary ImageData("data");
	
	
	//NSImage* pImage = [self getNSImageFromCIImage: image];
	//NSBitmapImageRep* pImage = [self  RGBABitmapImageRepFromCImage: image];
	NSBitmapImageRep* pImage = [[NSBitmapImageRep alloc] initWithCIImage: image];

	if(pImage)
	{
		//NSData* pData = [pImage TIFFRepresentation];
		TLMessaging::TMessage ImageMessage("image");
		
		// Set alpha
		Bool bAlpha = [pImage hasAlpha];
		ImageMessage.ExportData("Alpha", bAlpha);

		// Set width and height
		u32 uWidth = [pImage pixelsWide];
		
		// NOTE: Must be square atm
		u32 uHeight = [pImage pixelsHigh];
		
		ImageMessage.ExportData("Width", uWidth );
		ImageMessage.ExportData("Height", uHeight );	
	
		// Add pixel data to the binary tree
		TPtr<TBinary> pChild = ImageMessage.AddChild("TexData");
		
		if(pChild.IsValid())
		{
			
			u8* pData = [pImage bitmapData];
/*
			// Duplicate the way PNG import works
			TArray<u8>& PixelDataArray = pChild->GetDataArray();
			PixelDataArray.SetSize(0);
 
			u32 uImageSize = uWidth * uHeight;
			TArray<u8*> RowData;

			// Alloc row data
			for(u32 uIndex = 0; uIndex < uHeight; uIndex++)
			{
				RowData.Add(new u8[uWidth]);
			}

			// Copy data from image to RowData array
			//TLMemory::CopyData(*RowData.GetData(), pData, uImageSize);

			for(u32 uIndex = 0; uIndex < uHeight; uIndex++)
			{
				TLMemory::CopyData(RowData[uIndex], &pData[uHeight], uWidth);
			}

			 
				/*
				TArray<u8> RowData;
				
				// Add new row data
				for(u32 uWidthIndex = 0; uWidthIndex < uWidth; uWidthIndex++)
				{
					RowData.Add(&pData[uIndex + uWidthIndex], sizeof(u8));
				}
				
				// Add the row data to the pixel data array
				PixelDataArray.Add(RowData);
				//*

				// Add the row data to the pixel data array
				//PixelDataArray.Add(&pData[uIndex], uWidth * sizeof(u8));
			
			// Copy data from array to binary tree data
			for ( u32 r=0;	r<RowData.GetSize();	r++ )
			{
				//	add data to texture data
				PixelDataArray.Add( RowData[r], uWidth );
			}
			
			// Delete the temp data
			for ( u32 i=0;	i<RowData.GetSize();	i++ )
				TLMemory::DeleteArray( RowData[i] );
			
			RowData.Empty();
 */
			u32 uBitsPerPixel = [pImage bitsPerPixel];
			
			u32 uDataSize = (uWidth * uHeight) * 4;
			pChild->WriteData(pData, uDataSize);	
		}
		
		// Send message to update texture
		
		m_pPublisherProxy->PublishMessage(ImageMessage);
		
		// NSImage not needed anymore so we can release it now
		[pImage release];
		
	}
	
	return nil;
}

/*
- (NSImage *)getNSImageFromCIImage:(CIImage*)image
{
    NSImage *newimage;
    NSCIImageRep *ir;
    
    ir = [NSCIImageRep imageRepWithCIImage:image];
	
	CGRect rect = [image extent]; 
	
	NSSize size = NSMakeSize(rect.size.width, rect.size.height);
	
    newimage = [[NSImage alloc] initWithSize:size];
    
	[newimage addRepresentation:ir];
	
    return newimage;
}


/*
- (NSBitmapImageRep *)RGBABitmapImageRepFromCImage:(CIImage *) ciImage
{
    int width = [ciImage extent].size.width;
    int rows = [ciImage extent].size.height;
    int rowBytes = (width * 4);
    
    NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil pixelsWide:width pixelsHigh:rows bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bitmapFormat:0 bytesPerRow:rowBytes bitsPerPixel:0];
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName ( kCGColorSpaceGenericRGB );
    CGContextRef context = CGBitmapContextCreate( [rep bitmapData], width, rows, 8, rowBytes, colorSpace, kCGImageAlphaPremultipliedLast );
    
    CIContext* ciContext = [CIContext contextWithCGContext:context options:nil];
    [ciContext drawImage:ciImage atPoint:CGPointZero fromRect:[ciImage extent]];
    
	CGContextRelease( context );
	CGColorSpaceRelease( colorSpace );
	
	return rep;
}
 */


- (void) addSubscriber:(TLMessaging::TSubscriber*) pSubscriber
{
	m_pPublisherProxy->Subscribe(pSubscriber);
}


@end