#pragma once

#include <TootleCore/TPublisher.h>

namespace TLInput
{
	class TInputEffect;
}

class TLInput::TInputEffect : public TLMessaging::TPublisher
{
public:

	TInputEffect()					{}
	virtual ~TInputEffect()			{}

	virtual void Init() = 0;
	virtual void Update() = 0;
};

/*
	Varieties of effectss that will be specialised

// Force feedback (rumble) effect - will produce a force feedback response
class TLInput::TLInputEffectForceFeedback
{
	float m_fAmount;
}

*/