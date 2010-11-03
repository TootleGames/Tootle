/*------------------------------------------------------
	
	Spotify interface
 
	Interface to a spotify session
 
 Lib limitation is 1 at a time atm, so best to implement this as a singleton
 
-------------------------------------------------------*/
#pragma once


#include <TootleCore/TKeyArray.h>
#include <TootleCore/TRef.h>
#include "LibSpotify/include/libspotify/api.h"

namespace TLSpotify
{
	class TSession;
	class TPlaylist;
	class TTrack;
	
	
	namespace Private
	{
		extern THeapArray<TSession*>	g_Sessions;
	}
};



//--------------------------------------------------------
//	track info
//--------------------------------------------------------
class TLSpotify::TTrack : public TNoCopy
{
public:
	TTrack();
	explicit TTrack(sp_track& Track);
	~TTrack();
	
	SyncBool		Initialise();		//	for loading tracks
	bool			IsValid() const		{	return m_pTrack!=NULL;	}
	bool			IsReady() const		{	return m_Loaded == SyncTrue;	}
	
	const TString&	GetTitle() const	{	return m_Title;	}
	const TString&	GetArtist() const	{	return m_Artist;	}
	
	inline bool		operator==(const sp_track& Track) const	{	return &Track == m_pTrack;	}
	inline bool		operator==(const TTrack& Track) const	{	return m_pTrack == Track.m_pTrack;	}
	
private:
	void			UpdateInfo();
	void			ReleaseTrack();
	void			SetTrack(sp_track& Track);
	
protected:
	sp_track*	m_pTrack;		//	track reference
	TString		m_Title;
	TString		m_Artist;
	//TString		m_Album;
	SyncBool	m_Loaded;
};



//--------------------------------------------------------
//	session handler
//--------------------------------------------------------
class TLSpotify::TSession
{
public:
	TSession(const TArray<u8>& Key,const TString& AgentName,const TString& UserName,const TString& Password);
	~TSession();
	
	bool		Initialise(TRef& Error);		//	init the session
	void		Update(float Timestep);			//	asynch updates
	void		Shutdown();						//	cleanup the session
	
	bool		IsLoggedIn() const				{	return m_pUser!=NULL;	}
	//	bool		UpdatePlaylists();
	//	bool		CreatePlaylist(const TString& PlaylistName);	
	
	TRef		DownloadTrack(const TString& Url);		//	fetch track info, and queue it up for download
	
	//	search interface
	TRef		FindTrack(const TString& URI);
	
	void		UnloadTrack();											//	unload current track
	TTrack*		GetTrack(TRefRef TrackRef);
	
	bool		operator==(const sp_session* pSession) const	{	return m_pSession == pSession;	}	
	
	SyncBool	LoadTrack(TRefRef Track);
	bool		SeekCurrentTrack(float TimeSecs);
	bool		PlayCurrentTrack(bool Play);		//	if Play is false, the track is paused
	
private:
	TRef		AddTrack(sp_track& Track);			//	add this track to our cache	(or return existing)
	
private:
	THeapArray<u8>	m_Key;				//	application key
	TString			m_AgentName;		//	name of the application
	TString			m_UserName;			//	spotify username
	TString			m_Password;			//	spotify password
	sp_user*		m_pUser;			//	user if we're logged in
	
	float						m_EventProcessTimeout;	//	timeout in secs until we're allowed to process session events again
	sp_session*					m_pSession;				//	session
	//	sp_playlistcontainer*		m_pPlaylistContainer;	
	TRef						m_CurrentTrack;		//	currently loaded track
	THeapArray<TRef>			m_DownloadTracks;	//	tracks queued for download
	
	TPtrKeyArray<TRef,TTrack>	m_Tracks;
	TRef						m_LastTrackKey;		//	running track key counter
};


/*
 //--------------------------------------------------------
 //	playlist info
 //--------------------------------------------------------
 class TLSpotify::TPlaylist
 {
 public:
 TPlaylist(sp_playlist& Playlist);
 
 //	information below may not be valid until fully loaded
 const TString&			GetName() const			{	return m_Name;	}
 TArray<TTrack>&			GetTracks()				{	return m_Tracks;	}
 const TArray<TTrack>&	GetTracks() const		{	return m_Tracks;	}
 
 static void			Callbacks();
 
 protected:
 TString				m_Name;
 THeapArray<TTrack>	m_Tracks;	//	tracks in the playlist
 sp_playlist*		m_pPlaylist;
 };
 */

