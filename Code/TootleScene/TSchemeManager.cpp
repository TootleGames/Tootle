/*
 *  TSchemeManager.cpp
 *  TootleScene
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TSchemeManager.h"

#include <TootleAsset/TAsset.h>
#include <TootleCore/TEventChannel.h>
#include "TScenegraph.h"
#include "TSchemeNode.h"

namespace TLScheme
{
	TPtr<TSchemeManager> g_pSchemeManager = NULL;
}


using namespace TLScheme;

SyncBool TSchemeManager::Initialise() 
{	
	return SyncTrue; 
}

void TSchemeManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the update messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


SyncBool TSchemeManager::Shutdown()
{ 
	// While we still have schemes loaded request them to be unloaded
	if(m_Schemes.GetSize())
	{
		//NOTE: We may not need to do this as we can assume the scenegraph will shutdown and remove all nodes
		UnloadAllSchemes();

		// Call the update to be able to update the modes and requests
		Update(0.0f);
		
		// Wait for all schemes to have been unloaded.
		return SyncWait;
	}	
	
	// Remove all request objects
	for(u32 uIndex = 0; uIndex < m_SchemeUpdateRequests.GetSize(); uIndex++)
	{
		m_SchemeUpdateRequests.ElementAt(uIndex) = NULL;
	}

	// Empty request list
	m_SchemeUpdateRequests.Empty();
	
	// Empty the list of remove requests
	m_SchemeUpdateRemove.Empty();
	
	return SyncTrue; 
}

SyncBool TSchemeManager::Update(float fTimeStep)		
{
	u32 uNumberOfRequests = m_SchemeUpdateRequests.GetSize();
	
	if(uNumberOfRequests > 0)
	{
		// Update all of the scheme update requests
		for(u32 uIndex = 0; uIndex < uNumberOfRequests; uIndex++)
		{
			TPtr<TSchemeUpdateRequest> pRequest = m_SchemeUpdateRequests.ElementAt(uIndex);			

			pRequest->Update( fTimeStep );
		}
	}

	// Now go through the list of update requests that need removing if any
	uNumberOfRequests = m_SchemeUpdateRemove.GetSize();

	if(uNumberOfRequests)
	{
		for(u32 uIndex = 0; uIndex < uNumberOfRequests; uIndex++)
		{
			TRefRef SchemeRef = m_SchemeUpdateRemove.ElementAt(uIndex);
			
			s32 sIndex = m_SchemeUpdateRequests.FindIndex(SchemeRef);
			
			if(sIndex != -1)
			{
				// remove from the list
				m_SchemeUpdateRequests.RemoveAt(sIndex);				
			}
			else
			{
				TLDebug_Break("Failed to find scheme update request object for removal");
			}
		}

		// Empty the list of remove requests
		m_SchemeUpdateRemove.Empty();
	}
	
	return TManager::Update(fTimeStep);
}


Bool TSchemeManager::IsSchemeInState(TRefRef SchemeRef, TSchemeState SchemeState)
{
	if(SchemeState == Loaded)
	{
		return (m_Schemes.Find(SchemeRef) != NULL);
	}
	else if(SchemeState == UnLoaded)
	{
		return (m_Schemes.Find(SchemeRef) == NULL);
	}
	else
	{
		TPtr<TSchemeUpdateRequest> pRequest = m_SchemeUpdateRequests.FindPtr(SchemeRef);

		// Any request to load or unload? If not we are not loading or unloading the scheme
		if(!pRequest)
			return FALSE;

		// If the request is a load then return TRUE is the state check was Loading
		if((pRequest->GetUpdateType() == Load) && (SchemeState == Loading))
			return TRUE;
		else if(SchemeState == UnLoading)
			return TRUE;
			
		return FALSE;
	}
}

// Main request handling routine
Bool TSchemeManager::RequestUpdateScheme(TRefRef SchemeRef, TRefRef SchemeAssetRef, TSchemeUpdateType UpdateType)
{
	// Check to see if the scheme specified is alreayd in the state required
	if(UpdateType == Load)
	{
		// Laoded already?
		if(IsSchemeLoaded(SchemeRef))
			return TRUE;
		
		// Loading requested already?
		if(IsSchemeLoading(SchemeRef))
			return TRUE;
		
		// Unload requested?
		// Scheme loading?
		if(IsSchemeUnloading(SchemeRef))
		{
			// Cancel the unload and start the load
			TLDebug_Break("TODO");
			return FALSE;
		}
		
	}	
	else
	{
		// Unloaded already?
		if(IsSchemeUnLoaded(SchemeRef))
			return TRUE;

		// Unloading already?
		if(IsSchemeUnloading(SchemeRef))
			return TRUE;
		
		// Scheme loading?
		if(IsSchemeLoading(SchemeRef))
		{
			// Cancel the load and start the unload
			TLDebug_Break("TODO");
			return FALSE;
		}
		
	}
	
	
	// Begin the load/unload of the scheme
	TPtr<TSchemeUpdateRequest> pRequest = new TSchemeUpdateRequest(SchemeRef, SchemeAssetRef, UpdateType);
	
	if(pRequest)
	{
		if(m_SchemeUpdateRequests.Add(pRequest) != -1)
		{
			// Subscribe to the update request
			return SubscribeTo(pRequest.GetObject());
		}
	}
	

	
	return FALSE;
}

void TSchemeManager::UnloadAllSchemes()
{
	for(u32 uIndex = 0; uIndex < m_Schemes.GetSize(); uIndex++)
	{
		TRefRef SchemeRef = m_Schemes.ElementAt(uIndex);
		RequestUpdateScheme(SchemeRef, "", UnLoad);
	}
}

void TSchemeManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRef MessageRef = Message.GetMessageRef();

	// Handle request update messages
	if(MessageRef == "LOAD")
	{
		TRef SchemeRef;
		
		if(Message.ImportData("SCHEME", SchemeRef))
		{
			// If complete then add the scheme to our list of instanced schemes
			// and remove from the schemeupdate list
			m_Schemes.Add(SchemeRef);
			
			m_SchemeUpdateRemove.Add(SchemeRef);
				
		}
		
		return;
	}
	else if(MessageRef == "UNLOAD")
	{
		// If complete then remove the scheme from our list of instanced schemes
		// and remove from the schemeupdate list
		TRef SchemeRef;
		
		if(Message.ImportData("SCHEME", SchemeRef))
		{
			// If complete then add the scheme to our list of instanced schemes
			// and remove from the schemeupdate list			
			m_Schemes.Remove(SchemeRef);
			
			m_SchemeUpdateRemove.Add(SchemeRef);
		}
		
		return;
	}
	
	TManager::ProcessMessage(Message);
}








// Scheme update request class
TSchemeManager::TSchemeUpdateRequest::TSchemeUpdateRequest(TRefRef SchemeRef, TRefRef SchemeAssetRef, TSchemeUpdateType UpdateType) :
	m_SchemeRef(SchemeRef),
	m_SchemeAssetRef(SchemeAssetRef),
	m_Type(UpdateType)
{
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Init>("Init");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Loading>("Loading");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_UnLoading>("UnLoading");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Finished>("Finished");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Cancel>("Cancel");
}


void TSchemeManager::TSchemeUpdateRequest::PublishFinishedLoadMessage(Bool Success)
{
	TLMessaging::TMessage Message("LOAD", TLScheme::g_pSchemeManager->GetManagerRef() );
	Message.ExportData("SCHEME", GetSchemeRef() );
	PublishMessage(Message);
}


void TSchemeManager::TSchemeUpdateRequest::PublishFinishedUnloadMessage()
{
	TLMessaging::TMessage Message("UNLOAD", TLScheme::g_pSchemeManager->GetManagerRef() );
	Message.ExportData("SCHEME", GetSchemeRef() );
	PublishMessage(Message);
}



TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_Init::Update(float Timestep)
{	
	if(GetRequest().GetUpdateType() == Load)
	{
		// Begin sync load
		SyncBool LoadingState = TLAsset::LoadAsset( GetRequest().GetSchemeAssetRef(), "Scheme", FALSE );

		//	failed to load
		if ( LoadingState == SyncFalse )
		{
			// Broadcast message to say we are done loading - effectively bail out
			GetRequest().PublishFinishedLoadMessage(FALSE);
			return "Finished";
		}
		else
		{
			// Asset is loading
			return "Loading";
		}
	}
	else
	{
		// Begin unload
		return "UnLoading";
	}
}	


TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_Loading::Update(float Timestep)
{
	//	wait for scheme to load
	SyncBool LoadingState = TLAsset::LoadAsset( GetRequest().GetSchemeAssetRef(), "Scheme", FALSE );
	if ( LoadingState == SyncWait )
	{
		return TRef();
	}
	else if ( LoadingState == SyncFalse )
	{
		// Broadcast message to say we are done loading - effectively bail out
		GetRequest().PublishFinishedLoadMessage(FALSE);
		return "Finished";
	}

	//	get the now-loaded scheme
	TLAsset::TScheme* pScheme = TLAsset::GetAsset<TLAsset::TScheme>( GetRequest().GetSchemeAssetRef() );
	if ( !pScheme )
	{
		TLDebug_Break("Scheme Asset is no longer loaded");
		
		// Broadcast message to say we are done loading - effectively bail out
		GetRequest().PublishFinishedLoadMessage(FALSE);
		return "Finished";
	}

	//TODO:
	// Load the schemes required files
	// Wait for files to load

	//	Instance the scheme node we will be attaching the scheme contents to
	TRef SchemeRootNode = TLScene::g_pScenegraph->DoCreateNode( GetRequest().GetSchemeRef(), "Scheme" );
	if(!SchemeRootNode.IsValid())
	{
		// Failed
		TLDebug_Break("Failed to instance scheme node");

		// Broadcast message to say we are done loading - effectively bail out
		GetRequest().PublishFinishedLoadMessage(FALSE);
		return "Finished";
	}

	// Import scheme to scenegraph attached to the scheme node
	TLScene::g_pScenegraph->ImportScheme( pScheme, SchemeRootNode );

	// TODO:
	// From  past experience...
	// At this point we would need to go through link nodes and alter the link ID's so that they are specific to 
	// the parent scheme ID along with altering a nodes link reference to match - all links must reference unique 
	// nodes and when saved would be static (non-changeable) so we would make a temp change at runtime.
	// In the past however we didn't have control of the editor so we may be able to use alternative
	// solutions that are built into the system rather than this sort of 'hack'
	//	gr: from past expierence... use a different system than "links". Links will obviously be fine if ref's are strict...

	// Broadcast message to say we are done loading
	GetRequest().PublishFinishedLoadMessage(TRUE);

	// All done	
	return "Finished";	
}	




TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_UnLoading::Update(float Timestep)
{	
	// Find scheme node in scenegraph
	// Shutdown the scheme node
	// and remove from graph
	
	
	// Broadcast message to say we are done loading
	GetRequest().PublishFinishedUnloadMessage();
	
	// All done
	return "Finished";	
}	


TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_Cancel::Update(float Timestep)
{	
	return TRef();	
}	




