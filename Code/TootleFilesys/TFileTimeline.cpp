/*
 *  TFileTimeline.cpp
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TFileTimeline.h"
#include <TootleAsset/TAssetTimeline.h>
#include "TLFile.h"



TLFileSys::TFileTimeline::TFileTimeline(TRefRef FileRef,TRefRef FileTypeRef) :
TFileXml			( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML and convert from SVG to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileTimeline::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	Supported = TRUE;

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root tag
	TPtr<TXmlTag> pTasTag = m_XmlData.GetChild("timeline");

	//	malformed AssetScript
	if ( !pTasTag )
	{
		TLDebug_Print("TTL file missing root <Timeline> tag");
		return SyncFalse;
	}

	//	do specific importing
	TPtr<TLAsset::TAsset> pNewAsset = new TLAsset::TAssetTimeline( GetFileRef() );

	ImportResult = ImporTAssetTimeline( pNewAsset, pTasTag );

	//	failed to import
	if ( ImportResult != SyncTrue )
	{
		return SyncFalse;
	}

	//	assign resulting asset
	pAsset = pNewAsset;

	return SyncTrue;
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileTimeline::ImporTAssetTimeline(TPtr<TLAsset::TAssetTimeline> pAssetTimeline, TPtr<TXmlTag>& pTag)
{
	/*
	<assetscript>
		<keyframe time="0.0">
			<Node NodeRef="rarm1">		
				<Data DataRef="Rotate" Interp="TRUE" Method="Linear"><float3>0,0,0</float3></Data>
			</Node>
		</keyframe>
	</assetscript>
	*/
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "keyframe" )
		{
			TagImportResult = ImporTAssetTimeline_ImportKeyframeTag( pAssetTimeline, pChildTag );
		}
		else
		{
			TLDebug_Break("Unsupported tag in asset script import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TTL import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}


//--------------------------------------------------------
//	generate mesh TAM tag
//--------------------------------------------------------
SyncBool TLFileSys::TFileTimeline::ImporTAssetTimeline_ImportKeyframeTag(TPtr<TLAsset::TAssetTimeline>& pAssetTimeline,TPtr<TXmlTag>& pImportTag)
{
	/*
	<keyframe time="0.0">
		<Node NodeRef="rarm1">		
			<Data DataRef="Rotate" Interp="TRUE" Method="Linear"><float3>0,0,0</float3></Data>
		</Node>
	</keyframe>
	*/
	float keyframetime;
	const TString* pImportkeyframeString = pImportTag->GetProperty("Time");
	if ( pImportkeyframeString )
	{
		if(!pImportkeyframeString->GetFloat(keyframetime))
		{
			return SyncFalse;
		}
	}
	else
	{
		TLDebug_Break("gr: use of uninitialised keyframe time! - keyframe property is required I assume?");
		return SyncFalse;
	}

	// Create a new keyframe
	TLAsset::TKeyframe* pKeyframe = pAssetTimeline->AddKeyframe(keyframetime);

	if(!pKeyframe)
	{
		TLDebug_Print("Unable to create keyframe for TTL asset");
		return SyncFalse;
	}

	//	find out what we need to do
	for ( u32 c=0;	c<pImportTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(c);
		
		SyncBool TagImportResult = SyncFalse;

		// Deal with "node" tags
		if ( pChildTag->GetTagName() == "command" )
		{
			TagImportResult = ImporTAssetTimeline_ImportCommandTag( pAssetTimeline, pKeyframe, pChildTag );
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TTL import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}

SyncBool TLFileSys::TFileTimeline::ImporTAssetTimeline_ImportCommandTag(TPtr<TLAsset::TAssetTimeline>& pAssetTimeline, TLAsset::TKeyframe* pKeyframe, TPtr<TXmlTag>& pImportTag)
{
	/*
	<Node NodeRef="rarm1">		
		<Data DataRef="Rotate" Interp="TRUE" Method="Linear"><float3>0,0,0</float3></Data>
	</Node>
	*/

	TRef CommandRef, NodeRef, NodeGraphRef;

	const TString* pImportkeyframeString = pImportTag->GetProperty("CommandRef");
	if ( pImportkeyframeString )
		CommandRef.Set(*pImportkeyframeString);

	if(!CommandRef.IsValid())
	{
		TLDebug_Print("Failed to get valid command ref from TTL file");
		return SyncFalse;
	}

	// Get the node ref
	// TODO: Make this optional?
	pImportkeyframeString = pImportTag->GetProperty("NodeRef");
	if ( pImportkeyframeString )
		NodeRef.Set(*pImportkeyframeString);

	/*
	// Node ref is now optional
	if(!NodeRef.IsValid())
	{
		TLDebug_Print("Failed to get valid node ref from TTL file");
		return SyncFalse;
	}
	*/

	// Get the node graph ref
	pImportkeyframeString = pImportTag->GetProperty("NodeGraphRef");
	if ( pImportkeyframeString )
		NodeGraphRef.Set(*pImportkeyframeString);

	/*
	// Graph ref is now optional
	if(!NodeGraphRef.IsValid())
	{
		TLDebug_Print("Failed to get valid node graph ref from TTL file");
		return SyncFalse;
	}
	*/

	TPtr<TLAsset::TAssetTimelineCommandList> pTimelineCommandList = pKeyframe->FindPtr(NodeRef);

	// Check to see if we have a command list for the specified node.  If so use it, otherwise create a new 
	// command list with the specified node ref
	if(!pTimelineCommandList)
	{
		// Create the asset script command list object
		pTimelineCommandList = new TLAsset::TAssetTimelineCommandList(NodeRef, NodeGraphRef);

		if(!pTimelineCommandList || (pKeyframe->Add(pTimelineCommandList) == -1))
		{
			TLDebug_Print("Failed to add new script command list");
			return SyncFalse;
		}
	}


	// Create the command and attach it to the asset timeline
	TLAsset::TAssetTimelineCommand*	pCommand = pTimelineCommandList->AddCommand(CommandRef);

	if(!pCommand)
	{
		TLDebug_Print("Failed to create command for TTL file");
		return SyncFalse;
	}



	//	find out what we need to do
	for ( u32 c=0;	c<pImportTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = ImporTAssetTimeline_ImportCommandData( pAssetTimeline, pCommand, pChildTag);

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TTL import");
			return SyncFalse;
		}
	}

	return SyncTrue;

}


SyncBool TLFileSys::TFileTimeline::ImporTAssetTimeline_ImportCommandData(TPtr<TLAsset::TAssetTimeline>& pAssetTimeline, TLAsset::TAssetTimelineCommand* pTimelineCommand, TPtr<TXmlTag>& pImportTag)
{
	TPtr<TBinaryTree> pCommandChildData = NULL;

	// Get data tag and all data properties
	for ( u32 c=0;	c<pImportTag->GetPropertyCount();	c++ )
	{
		const TXmlTag::TProperty& Property = pImportTag->GetPropertyAt(c);

		const TStringLowercase<TTempString>& PropertyName = Property.m_Key;
		const TString& PropertyData = Property.m_Item;

		if ( PropertyName == "DataRef" )
		{
			// Add the data as a child of the command
			pCommandChildData = pTimelineCommand->AddChild(PropertyData);

			if(!pCommandChildData)
			{
				TLDebug_Print("Failed to create command child data");
				return SyncFalse;
			}
		}
		else if(PropertyName == "InterpMethod")
		{
			// Interp method.  Linear, SLERP etc.  Mainly for transform messages, rotation, translation and scale
			pCommandChildData->ExportData(PropertyName, TRef(PropertyData));
		}
		else if(PropertyName == "Mode")
		{
			// Mode - absolute, relative.  Mainly for transforms, rotation, translation and scale
			pCommandChildData->ExportData(PropertyName, TRef(PropertyData));
		}

		// Now get any extra data once we reach the end of the tag info
		if(pCommandChildData.IsValid() && (c == (pImportTag->GetPropertyCount() - 1)))
		{
			TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(0);

			if(pChildTag)
			{
				TRef DataTypeRef = TLFile::GetDataTypeFromString( pChildTag->GetTagName() );

				//	update type of data
				//	gr: this SHOULD be redundant and set by ImportBinaryData
				pCommandChildData->SetDataTypeHint( DataTypeRef );

				SyncBool TagImportResult = SyncFalse;

				// Special case for rotations
				if(pCommandChildData->GetDataRef() == "Rotate")
				{
					//if(DataTypeRef == TLBinary::GetDataTypeRef<float4>())
					if(DataTypeRef == TLBinary::GetDataTypeRef<TLMaths::TQuaternion>())
					{
						// Raw Quaternion format so import as a quaternion
						// TODO: Float 4... could be axis-angle or quaternion format.  Could use 
						// for axis angle with radians for angle seeing as that isn't supported
						// at the moment, or we could simply add an 'AngleFormat' property 
						// to determine the angle format which would probably be more flexible.
						TagImportResult = TLFile::ImportBinaryData( pChildTag, *pCommandChildData, DataTypeRef );
					}
					else if(DataTypeRef == TLBinary::GetDataTypeRef<TLMaths::TAxisAngle>())
					{
						// Axis and Angle format so import as an AxisAngle and convert to a Quaternion
						TBinary Data;
						TagImportResult = TLFile::ImportBinaryData( pChildTag, Data, DataTypeRef );

						if(TagImportResult == SyncTrue)
						{
							// Convert the data to a quaternion
							Data.ResetReadPos();

							float4 vector;
							if(Data.Read(vector))
							{
								TLMaths::TQuaternion qRot;

								TLMaths::TAxisAngle axisangle(vector);
			
#ifdef _DEBUG
								TTempString str;
								str.Appendf("Axis: %.2f %.2f %.2f", vector.x, vector.y, vector.z);
								TLDebug_Print(str);
								str.Empty();

								str.Appendf("Angle: %.2f", axisangle.GetAngle(FALSE));
								TLDebug_Print(str);
								str.Empty();


								str.Appendf("Radians: %.2f", axisangle.GetAngle());
								TLDebug_Print(str);
								str.Empty();
#endif
								qRot.Set(axisangle.GetAxis(), axisangle.GetAngle());
								qRot.Normalise();
#ifdef _DEBUG
								str.Appendf("Quat: %.2f %.2f %.2f %.2f", qRot.GetData().x, qRot.GetData().y, qRot.GetData().z, qRot.GetData().w );
								TLDebug_Print(str);
								str.Empty();
#endif

								pCommandChildData->Write(qRot); 

								// Change the data type hint to quaternion
								//	gr: this SHOULD be redundant and set by Write() above
								//pCommandChildData->SetDataTypeHint(DataTypeRef);
								pCommandChildData->SetDataTypeHint(TLBinary::GetDataTypeRef<TLMaths::TQuaternion>());
							}
						}

					}
					else if( (DataTypeRef == TLBinary::GetDataTypeRef<float3>()) ||
							 (DataTypeRef == TLBinary::GetDataTypeRef<TLMaths::TEuler>()) )
					{
						// float3 or Euler format so import as Euler angles
						// NOTE: float3 is assumed to be the same as euler but we could have 
						// the float3 as an explicit euler in radians angle format?
						// TODO: Support for both degrees and radians for the angle format.

						// Get the data and convert from a float3 euler data to a quaternion
						TBinary Data;
						TagImportResult = TLFile::ImportBinaryData( pChildTag, Data, DataTypeRef );

						if(TagImportResult == SyncTrue)
						{
							// Convert the data to a quaternion
							Data.ResetReadPos();

							float3 vector;
							if(Data.Read(vector))
							{
								TLMaths::TQuaternion qRot;

								// Vector is in degrees.  Change to radians.
								TLMaths::TEuler euler(vector);


#ifdef _DEBUG
								TTempString str;
								str.Appendf("Degrees: %.2f %.2f %.2f", euler.GetPitch(), euler.GetYaw(), euler.GetRoll());
								TLDebug_Print(str);
								str.Empty();

								str.Appendf("Radians: %.2f %.2f %.2f", euler.GetPitch(FALSE), euler.GetYaw(FALSE), euler.GetRoll(FALSE));
								TLDebug_Print(str);
								str.Empty();
#endif

								qRot.SetEuler(euler.GetAngles());
								qRot.Normalise();

#ifdef _DEBUG
								str.Appendf("Quat: %.2f %.2f %.2f %.2f", qRot.GetData().x, qRot.GetData().y, qRot.GetData().z, qRot.GetData().w );
								TLDebug_Print(str);
								str.Empty();
#endif

								pCommandChildData->Write(qRot); 

								// Change the data type hint to quaternion
								//	gr: this SHOULD be redundant and set by Write() above
								//pCommandChildData->SetDataTypeHint(DataTypeRef);
								pCommandChildData->SetDataTypeHint(TLBinary::GetDataTypeRef<TLMaths::TQuaternion>());
							}
						}
						else
						{
							// Failed to copy the data form the XML file
							TLDebug_Print("Failed to get (rotation) command data from TTL file");
							return SyncFalse;
						}
					}
				}
				else if(pCommandChildData->GetDataRef() == "Colour")
				{
					//if(DataTypeRef == TLBinary::GetDataTypeRef<float4>())
					if(DataTypeRef == TLBinary::GetDataTypeRef<TColour>())
					{
						// Raw Colour format so import as a TColour
						TagImportResult = TLFile::ImportBinaryData( pChildTag, *pCommandChildData, DataTypeRef );
					}
					else if( (DataTypeRef == TLBinary::GetDataTypeRef<TColour24>()) ||
							 (DataTypeRef == TLBinary::GetDataTypeRef<TColour32>()) ||
//							 (DataTypeRef == TLBinary::GetDataTypeRef<TColour48>()) ||
							 (DataTypeRef == TLBinary::GetDataTypeRef<TColour64>()) )
					{
						// Alternative colour format format
						// Import the data
						TBinary Data;
						TagImportResult = TLFile::ImportBinaryData( pChildTag, Data, DataTypeRef );

						if(TagImportResult == SyncTrue)
						{
							TColour newcol;
							Bool bSuccess = FALSE;

							// Convert the data to a TColour
							Data.ResetReadPos();

							if(DataTypeRef == TLBinary::GetDataTypeRef<TColour24>())
							{
								TColour24 col;
								bSuccess = Data.Read(col);

								if(bSuccess)
									newcol = col;
							}
							else if(DataTypeRef == TLBinary::GetDataTypeRef<TColour32>())
							{
								TColour32 col;
								bSuccess = Data.Read(col);

								if(bSuccess)
		 							newcol = col;
							}
							/*
							// Not supported yet?
							else if(DataTypeRef == TLBinary::GetDataTypeRef<TColour48>())
							{
								TColour48 col;
								bSuccess = Data.Read(col);

								if(bSuccess)
		 							newcol = col;
							}
							*/
							else if(DataTypeRef == TLBinary::GetDataTypeRef<TColour64>())
							{
								TColour64 col;
								bSuccess = Data.Read(col);

								if(bSuccess)
		 							newcol = col;
							}

							if(bSuccess)
							{
								pCommandChildData->Write(newcol); 

								// Change the data type hint to TColour
								//	gr: this SHOULD be redundant and set by Write() above
								pCommandChildData->SetDataTypeHint(TLBinary::GetDataTypeRef<TColour>());						
							}
						}
						else
						{
							// Failed to copy the data form the XML file
							TLDebug_Print("Failed to get (colour) command data from TTL file");
							return SyncFalse;
						}
					}

					//TODO: Add support for other colour types :-
					// TColour48 rgb 16-bit
					// float3 rgb ?
					// flaot4 rgba ?
				}
				else
					TagImportResult = TLFile::ImportBinaryData( pChildTag, *pCommandChildData, DataTypeRef );


				if(TagImportResult != SyncTrue)
				{
					// Failed to copy the data form the XML file
					TLDebug_Print("Failed to get command data from TTL file");
					return SyncFalse;
				}
			}
		}
	}


	
	return SyncTrue;
}


