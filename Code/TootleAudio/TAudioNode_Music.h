


#pragma once

#include "TAudioNode.h"

namespace TLAudio
{
	class TAudioNode_Music;
}


class TLAudio::TAudioNode_Music : public TLAudio::TAudioNode
{
public:
	TAudioNode_Music(TRefRef NodeRef,TRefRef TypeRef);

protected:
	virtual float GetGlobalVolume();

};