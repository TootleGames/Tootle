#pragma once

#include <TootleCore/TLGraph.h>
#include <TootleCore/TFlags.h>

namespace TLAudio
{
	class TAudioNode;
	class TAudiograph;

	class TAudioProperties;
};


class TLAudio::TAudioProperties
{
public:

private:
	float m_fVolume;
	float m_fFrequency;
};


class TLAudio::TAudioNode : public TLGraph::TGraphNode<TAudioNode>
{
	friend class TLAudio::TAudiograph;
public:
	TAudioNode(TRefRef NodeRef,TRefRef TypeRef);

	TRefRef				GetSourceRef()		const { return m_SourceRef; }
public:
	virtual void		UpdateAll(float Timestep)		{}	//	gr: no updates for audio nodes

private:
	TAudioProperties	m_Properties;
	
	TRef				m_SourceRef;		// OpenAL Source ID
};

