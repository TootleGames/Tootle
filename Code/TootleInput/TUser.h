#pragma once

#include <TootleCore/TPtrArray.h>
#include <TootleCore/TRelay.h>
#include <TootleCore/TManager.h>
#include <TootleCore/TString.h>

#include "TAction.h"

namespace TLUser
{
	class TUser;
	class TUserManager;

	extern TPtr<TUserManager> g_pUserManager;
};

/*
	User class
*/
class TLUser::TUser : public TLMessaging::TRelay
{
	friend class TLUser::TUserManager;

public:
	TUser(TRefRef refUserID);

	// Action Mapping
	Bool						AddAction(TRefRef refActionType, TRefRef refActionID);
	Bool						AddAction(TRefRef refActionType, TRefRef refActionID, TRefRef refParentActionID);
	Bool						AddAction(TRefRef refActionType, TRefRef refActionID, TArray<TRef>& refParentActionIDs);
	FORCEINLINE Bool			RemoveAction(TRefRef refActionID)			{	return m_ActionMap.Remove( refActionID );	}

	Bool						MapAction(TRefRef refActionID, TRefRef refDeviceID, TRefRef SensorRef);
	Bool						MapActionCondition(TRefRef refActionID, TLInput::TActionCondition uCondition, float fThreshold);
	Bool						MapActionParent(TRefRef refActionID, TRefRef refParentActionID, Bool bCondition = TRUE);

	// User information access
	FORCEINLINE TRefRef				GetUserID()		const					{	return m_refUserID;	}
	FORCEINLINE u8					GetUserIndex()	const					{	return m_uLocalUserIndex;	}
	FORCEINLINE const Type2<s32>&	GetCursorPosition() const				{	return m_CursorPosition;	}

	FORCEINLINE const TString&		GetUserName() const 					{	return m_strUserName;	}
	FORCEINLINE void				SetUserName(const TString& strUserName)	{	m_strUserName = strUserName;	}

	TRef							GetUnusedActionRef(TRef BaseRef=TRef()) const;		//	get a ref for an action not currently in use

protected:
	FORCEINLINE s32						FindActionIndex(TRefRef refActionID)		{	return m_ActionMap.FindIndex( refActionID );	}
	FORCEINLINE TPtr<TLInput::TAction>&	GetAction(TRefRef refActionID)				{	return m_ActionMap.FindPtr( refActionID );	}

	virtual void				ProcessMessage(TLMessaging::TMessage& Message);

	void						UpdateCursorPosition(u8 uCursorIndex);

	inline void					SetUserIndex(u8 uIndex)				{ m_uLocalUserIndex = uIndex; }

	Bool						MapAction(TRefRef refActionID,TPtr<TLInput::TInputSensor>& pSensor);	

private:
	void						RemoveAllActions()				{}

	Bool						MapActionParentPtr(TLInput::TAction* pAction,TLInput::TAction* pParentAction, Bool bCondition = TRUE);

private:
	TRef							m_refUserID;	// Unique ID of the user

	TPtrArray<TLInput::TAction>		m_ActionMap;	// Maps actions to inputs for a user
	int2							m_CursorPosition;	// Users cursor position

	TString							m_strUserName;

	u8								m_uLocalUserIndex;		// User index (i.e. player 1, 2 3 etc) on local machine
};


class TLUser::TUserManager : public TLCore::TManager
{
public:
	TUserManager(TRefRef ManagerRef) :
		TLCore::TManager	( ManagerRef )
	{
	}

	Bool			RegisterUser(TRef refUserID);
	Bool			UnregisterUser(TRef refUserID);

 	inline TPtr<TUser>		GetUser(TRef refUserID)		{ return FindUser(refUserID); }

	inline u32				GetNumberOfUsers()	const	{ return m_Users.GetSize(); }

protected:
	virtual SyncBool Initialise();
	virtual SyncBool Update(float /*fTimeStep*/);		
	virtual SyncBool Shutdown();

	virtual void	ProcessMessage(TLMessaging::TMessage& Message);
	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	// Internal user access
	inline Bool HasUser(TRef refUserID)  
	{ 
		TPtr<TUser> pUser = FindUser(refUserID);
		return pUser.IsValid(); 
	}

	s32 FindUserIndex(TRef refUserID);
	TPtr<TUser> FindUser(TRef refUserID);

private:
	void	UnregisterAllUsers();

private:
	TPtrArray<TUser>			m_Users;		// The lsit of users
};