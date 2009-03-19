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


namespace TLAsset
{
	class TAssetScript;

	class TKeyframe;
	class TAssetScriptCommand;
};


// The asset script command is a special type of message
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

	TAssetScriptCommand(TRef CommandRef) :
		TMessage(CommandRef),
		m_uInterpMethod(None)
	{
	}

	FORCEINLINE void				SetInterpMethod(InterpMethod Method)	{ m_uInterpMethod = Method; }
	FORCEINLINE InterpMethod		GetInterpMethod()				const	{ return m_uInterpMethod; }

private:
	InterpMethod					m_uInterpMethod;	// Interp method for the command
};



class TLAsset::TKeyframe
{
public:
	TKeyframe()
	{
	}
	TKeyframe(float fTime) :
		m_fTime(fTime)
	{
	}

	TAssetScriptCommand*			AddCommand(TRef CommandRef)
	{
		TAssetScriptCommand cmd(CommandRef);

		s32 Index = m_Commands.Add(cmd);

		if(Index == -1)
			return NULL;

		return &m_Commands.ElementAt(Index);
	}

	// Accessors
	TArray<TAssetScriptCommand>&	GetCommands()			{ return m_Commands; }

	FORCEINLINE void				SetTime(float fTime)	{ m_fTime = fTime; }
	FORCEINLINE float				GetTime()		const	{ return m_fTime; }

private:
	float									m_fTime;			// Keyframe time

	TArray<TAssetScriptCommand>				m_Commands;			// Array of command messages for the keyframe
};


class TLAsset::TAssetScript : public TLAsset::TAsset
{
public:
	TAssetScript(const TRef& AssetRef);

	//TLAsset::TKeyframe&		Addkeyframe(TRef KeyRef, float fTime)	// DB - may want an ID for each keyframe so we could do things like loop back to a specific keyframe
	TKeyframe*		AddKeyframe(float fTime)
	{
		TKeyframe key(fTime);

		s32 Index = m_Keyframes.Add(key);

		if(Index == -1)
			return NULL;

		return &m_Keyframes.ElementAt(Index);
	}

	Bool		GetKeyframes(const float& m_fTime,const float& fTimeStep, TArray<TKeyframe*>& pKeyframes);

protected:
	virtual SyncBool		ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool		ExportData(TBinaryTree& Data);	//	save asset data to binary data

private:

	TArray<TKeyframe>		m_Keyframes;
};