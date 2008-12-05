#include "PCAudioItem.h"

using namespace TLAudio;

Platform::TAudioItem::TAudioItem(TRef refAudioItemID) :
	m_refAudioItemID(refAudioItemID),
	m_hBufferEndEvent( CreateEvent( NULL, FALSE, FALSE, NULL ) )
{
}

Platform::TAudioItem::~TAudioItem()
{ 
	CloseHandle( m_hBufferEndEvent ); 
}

void Platform::TAudioItem::OnStreamEnd() 
{ 
	SetEvent( m_hBufferEndEvent ); 

	//TODO: send a message to the subscribers to say the audio has finished
}