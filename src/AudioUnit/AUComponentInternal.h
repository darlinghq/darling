#ifndef AUCOMPONENTINTERNAL_H
#define AUCOMPONENTINTERNAL_H
#include "AUComponent.h"
#include <CoreServices/ComponentsInternal.h>

class AudioUnitComponent : public CarbonComponent
{
private:
	AudioUnitComponent(int cardIndex, char* cardName);
public:
	static AudioUnitComponent* create(int cardIndex);
	virtual ~AudioUnitComponent();
	
	inline int cardIndex() const { return m_cardIndex; }
private:
	int m_cardIndex;
	char* m_cardName;
};

#endif

