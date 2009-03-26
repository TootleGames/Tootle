/*
 *  TAssetScript.h
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
	class TAssetScript;

	class TTempKeyframeData;
	class TKeyframe;

	class TAssetScriptCommand;
	class TAssetScriptCommandList;
};


// The asset script command is a special type of message so we can send it as-is 
// without having to generate a message and keep track of commnand specific data
// I have added an interp method as I suspect quite a few of the most common commands 
// will require an interp such as translation, rotation, scale but if this becomes less common 
// a property for most commands then it can simply be added to the message as a piece of data instead.
class TLAsset::TAssetScriptCommand : public TLMessaging::TMessage
{
public:
	enum InterpMethod
	{
		None = 0,
		Linear,
		SLERP,
	};

	TAssetScriptCommand() :
		m_uInterpMethod(None)
	{
	}

	TAssetScriptCommand(TRefRef CommandRef) :
		TMessage(CommandRef),
		m_uInterpMethod(None)
	{
	}

	FORCEINLINE void				SetInterpMethod(InterpMethod Method)	{ m_uInterpMethod = Method; }
	FORCEINLINE InterpMethod		GetInterpMethod()				const	{ return m_uInterpMethod; }


	//SyncBool						ImportData(TBinaryTree& Data);	//	load asset data out binary data
	//SyncBool						ExportData(TBinaryTree& Data);	//	save asset data to binary data


private:
	InterpMethod					m_uInterpMethod;	// Interp method for the command
};


// The script command list is a container for a node ref and graph ref and list of commands that
// for the specified node
class TLAsset::TAssetScriptCommandList
{
public:

	TAssetScriptCommandList() 
	{
	}


	TAssetScriptCommandList(TRefRef NodeRef, TRefRef NodeGraphRef) :
		m_NodeRef(NodeRef),
		m_NodeGraphRef(NodeGraphRef)
	{
	}

	TAssetScriptCommand*			AddCommand(TRefRef CommandRef)
	{
		TAssetScriptCommand cmd(CommandRef);

		s32 Index = m_Commands.Add(cmd);

		if(Index == -1)
			return NULL;

		return &m_Commands.ElementAt(Index);
	}

	inline Bool			operator==(const TRef& NodeRef)						const	{	return GetNodeRef() == NodeRef;	}
	inline Bool			operator==(const TAssetScriptCommandList& ascmd)	const 	{	return GetNodeRef() == ascmd.GetNodeRef();	}


	// Accessors
	FORCEINLINE void				SetNodeRef(TRefRef NodeRef)				{ m_NodeRef = NodeRef; }
	FORCEINLINE TRefRef				GetNodeRef()					const	{ return m_NodeRef; }

	FORCEINLINE void				SetNodeGraphRef(TRefRef NodeGraphRef)	{ m_NodeGraphRef = NodeGraphRef; }
	FORCEINLINE TRefRef				GetNodeGraphRef()				const	{ return m_NodeGraphRef; }

	TArray<TAssetScriptCommand>&	GetCommands()			{ return m_Commands; }

	SyncBool						ImportData(TBinaryTree& Data);	//	load asset data out binary data
	SyncBool						ExportData(TBinaryTree& Data);	//	save asset data to binary data

private:
	TRef							m_NodeRef;			// Node associated with the commands
	TRef							m_NodeGraphRef;		// Node graph to use 

	TArray<TAssetScriptCommand>				m_Commands;			// Array of command messages
};



// The keyframe is essentially a list of commands for nodes organised by node
class TLAsset::TKeyframe : public TPtrArray<TLAsset::TAssetScriptCommandList>
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


class TLAsset::TAssetScript : public TLAsset::TAsset
{
public:
	TAssetScript(const TRef& AssetRef);

	//TLAsset::TKeyframe&		Addkeyframe(TRef KeyRef, float fTime)	// DB - may want an ID for each keyframe so we could do things like loop back to a specific keyframe
	TKeyframe*		AddKeyframe(float fTime)
	{
		TPtr<TKeyframe> pKey = new TKeyframe();

		if(m_Keyframes.Add(fTime, pKey) != NULL)
			return pKey.GetObject();

		return NULL;
	}

	Bool		GetKeyframes(const float& fTimeFrom,const float& fTimeStep, TArray<TTempKeyframeData>& pKeyframes);

protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data


	Bool	GetKeyframes_Forward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes);
	Bool	GetKeyframes_Backward(const float& fTimeFrom,const float& fTimeTo, TArray<TTempKeyframeData>& pKeyframes);


private:

	TPtrKeyArray<float, TKeyframe>		m_Keyframes;
};