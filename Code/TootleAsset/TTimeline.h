/*
 *  TTimeline.h
 *  TootleAsset
 *
 *  Created by Duane Bradbury on 15/03/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TAsset.h>
#include <TootleCore/TKeyArray.h>


namespace TLAsset
{
	class TTimeline;

	class TTempKeyframeData;
	class TKeyframe;

	class TTimelineCommand;
	class TTimelineCommandList;
};


// The timeline command is a special type of message so we can send it as-is 
// without having to generate a message and keep track of command specific data
// I have added an interp method as I suspect quite a few of the most common commands 
// will require an interp such as translation, rotation, scale but if this becomes less common 
// a property for most commands then it can simply be added to the message as a piece of data instead.
class TLAsset::TTimelineCommand : public TLMessaging::TMessage
{
public:
	TTimelineCommand()
	{
	}

	TTimelineCommand(TRefRef CommandRef) :
		TMessage(CommandRef)
	{
	}
};


// The script command list is a container for a node ref and graph ref and list of commands that
// for the specified node
class TLAsset::TTimelineCommandList
{
public:

	TTimelineCommandList() 
	{
	}


	TTimelineCommandList(TRefRef NodeRef, TRefRef NodeGraphRef) :
		m_NodeRef(NodeRef),
		m_NodeGraphRef(NodeGraphRef)
	{
	}

	TTimelineCommand*			AddCommand(TRefRef CommandRef)
	{
		TTimelineCommand cmd(CommandRef);

		s32 Index = m_Commands.Add(cmd);

		if(Index == -1)
			return NULL;

		return &m_Commands.ElementAt(Index);
	}

	inline Bool			operator==(const TRef& NodeRef)						const	{	return GetNodeRef() == NodeRef;	}
	inline Bool			operator==(const TTimelineCommandList& ascmd)	const 	{	return GetNodeRef() == ascmd.GetNodeRef();	}


	// Accessors
	FORCEINLINE void				SetNodeRef(TRefRef NodeRef)				{ m_NodeRef = NodeRef; }
	FORCEINLINE TRefRef				GetNodeRef()					const	{ return m_NodeRef; }

	FORCEINLINE void				SetNodeGraphRef(TRefRef NodeGraphRef)	{ m_NodeGraphRef = NodeGraphRef; }
	FORCEINLINE TRefRef				GetNodeGraphRef()				const	{ return m_NodeGraphRef; }

	TArray<TTimelineCommand>&		GetCommands()			{ return m_Commands; }

	SyncBool						ImportData(TBinaryTree& Data);	//	load asset data out binary data
	SyncBool						ExportData(TBinaryTree& Data);	//	save asset data to binary data

private:
	TRef							m_NodeRef;			// Node associated with the commands
	TRef							m_NodeGraphRef;		// Node graph to use 

	THeapArray<TTimelineCommand>	m_Commands;			// Array of command messages
};



// The keyframe is essentially a list of commands for nodes organised by node
class TLAsset::TKeyframe : public TPtrArray<TLAsset::TTimelineCommandList>
{
public:
	TKeyframe()
	{
	}

	SyncBool						ImportData(TBinaryTree& Data);	//	load asset data out binary data
	SyncBool						ExportData(TBinaryTree& Data);	//	save asset data to binary data
};


class TLAsset::TTempKeyframeData
{
public:
	float				m_fTime;			// Keyframe time
	TKeyframe*			m_pKeyframe;		// keyframe data
};


class TLAsset::TTimeline : public TLAsset::TAsset
{
public:
	TTimeline(TRefRef AssetRef);

	static TRef				GetAssetType_Static()						{	return TRef_Static(T,i,m,e,l);	}
	
	//TLAsset::TKeyframe&	Addkeyframe(TRef KeyRef, float fTime)	// DB - may want an ID for each keyframe so we could do things like loop back to a specific keyframe
	FORCEINLINE TKeyframe*	AddKeyframe(float fTime);

	Bool					GetKeyframes(const float& fTimeFrom,const float& fTimeStep, TArray<TTempKeyframeData>& pKeyframes, Bool bAllowNoTimestep);

	float					GetLastKeyFrameTime();

protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data


	Bool	GetKeyframes_Forward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes);
	Bool	GetKeyframes_Backward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes);


private:
	TPtrKeyArray<float, TKeyframe>		m_Keyframes;
};



FORCEINLINE TLAsset::TKeyframe* TLAsset::TTimeline::AddKeyframe(float fTime)
{
	TPtr<TLAsset::TKeyframe> pKey = new TLAsset::TKeyframe();

	if(m_Keyframes.Add(fTime, pKey) != NULL)
		return pKey.GetObjectPointer();

	return NULL;
}


