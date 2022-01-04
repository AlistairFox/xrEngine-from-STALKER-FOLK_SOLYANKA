#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
 
struct voiceData
{
	char data[5000];
	ALint size;
	Fvector3 pos;
};



class XRSOUND_API xrVoice
{
public:
	xrVoice();
	~xrVoice();
	void createVoice();
 
	voiceData CapturedVoice();
	void UpdateCapture(bool value);
	void PlayCapture(voiceData dataPlay);
	bool getCaptureState();
	u32 sizeCapture() ;
	void setGain(float value);
};
 
XRSOUND_API xrVoice* Voice_Export;