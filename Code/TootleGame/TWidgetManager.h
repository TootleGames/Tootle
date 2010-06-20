

#pragma once

#include <TootleCore/TManager.h>
#include <TootleInput/TUser.h>

#include <TootleCore/TRef.h>
#include <TootleCore/TKeyArray.h>

#include "TWidgetFactory.h"

namespace TLGui
{
	class TWidgetManager;

	extern TPtr<TWidgetManager>	g_pWidgetManager;
}

// Widget cache should help with searches within the widget system
//#define ENABLE_WIDGET_CACHE

class TLGui::TWidgetManager : public TLCore::TManager
{
private:
	
	class TActionRefData
	{
	public:
		TRef	m_ClickActionRef;
		TRef	m_MoveActionRef;
	};
	
	class TWidgetCache
	{
	public:
		
		TWidgetCache() :
			m_pWidgetGroup	( NULL ),
			m_pWidget		( NULL )
		{}
		
		FORCEINLINE	void			Clear()				{ Set(TRef(), NULL, NULL); }
		
		FORCEINLINE TRef			GetWidgetGroupRef()	const	{ return m_WidgetGroupRef; }
		FORCEINLINE TPtrArray<TWidget>*		GetWidgetGroup()	const	{ return m_pWidgetGroup; }
		FORCEINLINE TPtr<TWidget>		GetWidget()		const	{ return m_pWidget; }
	
#ifdef ENABLE_WIDGET_CACHE
		FORCEINLINE void		Set(TRef WidgetGroupRef, TPtrArray<TWidget>* pArray, TPtr<TWidget> pWidget) { m_WidgetGroupRef = WidgetGroupRef; m_pWidgetGroup = pArray; m_pWidget = pWidget; }
		FORCEINLINE void		SetWidget(TPtr<TWidget> pWidget)	{ m_pWidget = pWidget; }
#else
		FORCEINLINE void		Set(TRef WidgetGroupRef, TPtrArray<TWidget>* pArray, TPtr<TWidget> pWidget)	{ }
		FORCEINLINE void		SetWidget(TPtr<TWidget> pWidget)											{ }
#endif

	private:
		TRef				m_WidgetGroupRef;		// Last group used ref
		TPtrArray<TWidget>*		m_pWidgetGroup;			// Last group used
		TPtr<TWidget>			m_pWidget;				// Last widget used
	};

public:
	TWidgetManager(TRefRef ManagerRef) :
		TLCore::TManager(ManagerRef)
	{
	}
	
	TRef			CreateWidget(TRefRef WidgetGroupRef, TRefRef InstanceRef, TRefRef TypeRef, Bool bStrict=FALSE);
	FORCEINLINE TRef	CreateWidget(const TTypedRef& GroupInstanceRef, TRefRef TypeRef) 										{ return CreateWidget(GroupInstanceRef.GetRef(), GroupInstanceRef.GetTypeRef(), TypeRef); }

	
	FORCEINLINE Bool	RemoveWidget(TRefRef GroupInstanceRef, TRefRef InstanceRef);
	FORCEINLINE Bool	RemoveWidget(const TTypedRef& GroupInstanceRef)														{ return RemoveWidget(GroupInstanceRef.GetRef(), GroupInstanceRef.GetTypeRef()); }
	
	void			SendMessageToWidget(TRefRef WidgetGroupRef, TRefRef WidgetRef, TLMessaging::TMessage& Message);
	FORCEINLINE void	SendMessageToWidget(const TTypedRef& GroupInstanceRef, TLMessaging::TMessage& Message)				{ return SendMessageToWidget(GroupInstanceRef.GetRef(), GroupInstanceRef.GetTypeRef(), Message); }
	
	Bool			SubscribeToWidget(TRefRef WidgetGroupRef, TRefRef WidgetRef, TSubscriber* pSubscriber);
	FORCEINLINE Bool	SubscribeToWidget(const TTypedRef& GroupInstanceRef, TSubscriber* pSubscriber)						{ return SubscribeToWidget(GroupInstanceRef.GetRef(), GroupInstanceRef.GetTypeRef(), pSubscriber); }

	
	Bool			SubscribeWidgetTo(TRefRef WidgetGroupRef, TRefRef WidgetRef, TPublisher* pPublisher);
	FORCEINLINE Bool	SubscribeWidgetTo(const TTypedRef& GroupInstanceRef, TPublisher* pPublisher)						{ return SubscribeWidgetTo(GroupInstanceRef.GetRef(), GroupInstanceRef.GetTypeRef(), pPublisher); }


	
	FORCEINLINE Bool	IsClickActionRef(TRefRef ClickActionRef);
	FORCEINLINE Bool	IsMoveActionRef(TRefRef MoveActionRef);
	FORCEINLINE Bool	IsActionPair(TRefRef ClickActionRef, TRefRef MoveActionRef);

	FORCEINLINE TRef	GetClickActionFromMoveAction(TRefRef MoveActionRef);
	FORCEINLINE TRef	GetMoveActionFromClickAction(TRefRef ClickActionRef);

	FORCEINLINE Bool	AddFactory(TPtr< TClassFactory<TWidget,FALSE> >& pFactory)	{ return m_WidgetFactories.Add(pFactory)!=-1; }
	
protected:
	virtual SyncBool Initialise();
	virtual SyncBool Shutdown();

	
	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);	

	void			OnInputDeviceAdded(TRefRef refDeviceID, TRefRef refDeviceType);
	void			OnInputDeviceRemoved(TRefRef refDeviceID, TRefRef refDeviceType);
	
	Bool			DoRemoveWidget(TRefRef RenderTargetRef, TRefRef InstanceRef);

	// Action mapping
#ifdef TL_TARGET_IPOD
	void			MapDeviceActions_TouchPad(TRefRef refDeviceID, TPtr<TLUser::TUser> pUser);
#else
	void			MapDeviceActions_Mouse(TRefRef refDeviceID, TPtr<TLUser::TUser> pUser);
#endif

	void			MapDeviceActions_Keyboard(TRefRef refDeviceID, TPtr<TLUser::TUser> pUser);
	
private:
	
	TPtr<TWidget>	FindWidget(TRefRef WidgetGroupRef, TRefRef WidgetRef);
	
private:
	
	TWidgetCache				m_WidgetCache;		// Widget cache for speeding up some routines that get called with the same data consecutively
	
	THeapArray<TActionRefData>	m_WidgetActionRefs;
	
	TKeyArray< TRef, TPtrArray<TWidget> >	m_WidgetRefs;			// Array of widget ref's organised with a TRenderTarget TRef as the key
	TPtrArray< TClassFactory<TWidget,FALSE> >	m_WidgetFactories;		//	array of widget factories.
};


FORCEINLINE Bool TLGui::TWidgetManager::RemoveWidget(TRefRef RenderTargetRef, TRefRef InstanceRef)
{ 
	if(InstanceRef.IsValid()) 
		return DoRemoveWidget(RenderTargetRef, InstanceRef); 
	
	TLDebug_Print("Passing invalid widget ref to TWidgetManager::RemoveWidget");
	return FALSE;
}





FORCEINLINE Bool TLGui::TWidgetManager::IsClickActionRef(TRefRef ClickActionRef)
{
	for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
	{
		if(m_WidgetActionRefs.ElementAt(uIndex).m_ClickActionRef == ClickActionRef)
			return TRUE;
	}
	return FALSE;
}

FORCEINLINE Bool TLGui::TWidgetManager::IsMoveActionRef(TRefRef MoveActionRef)
{
	for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
	{
		if(m_WidgetActionRefs.ElementAt(uIndex).m_MoveActionRef == MoveActionRef)
			return TRUE;
	}
	return FALSE;
}

FORCEINLINE Bool TLGui::TWidgetManager::IsActionPair(TRefRef ClickActionRef, TRefRef MoveActionRef)
{
	for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
	{
		const TActionRefData& ActionPair = m_WidgetActionRefs.ElementAt(uIndex);
		if( ActionPair.m_ClickActionRef == ClickActionRef)
		{
			if(ActionPair.m_MoveActionRef == MoveActionRef)
				return TRUE;

			// Actions will be unique so if we find the click ref 
			// but move is different then we should not find any further click with the same ref
			// therefore we can return
			return FALSE;
		}
	}
	return FALSE;
}


FORCEINLINE TRef TLGui::TWidgetManager::GetClickActionFromMoveAction(TRefRef MoveActionRef)
{
	for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
	{
		const TActionRefData& ActionPair = m_WidgetActionRefs.ElementAt(uIndex);
	
		if(ActionPair.m_MoveActionRef == MoveActionRef)
			return ActionPair.m_ClickActionRef;
	}
	
	return TRef_Invalid;
}


FORCEINLINE TRef TLGui::TWidgetManager::GetMoveActionFromClickAction(TRefRef ClickActionRef)
{
	for(u32 uIndex = 0; uIndex < m_WidgetActionRefs.GetSize(); uIndex++)
	{
		const TActionRefData& ActionPair = m_WidgetActionRefs.ElementAt(uIndex);
	
		if(ActionPair.m_ClickActionRef == ClickActionRef)
			return ActionPair.m_MoveActionRef;
	}
	
	return TRef_Invalid;
}



