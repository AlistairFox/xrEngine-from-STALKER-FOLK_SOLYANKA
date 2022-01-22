#include "stdafx.h"
#include "voice.h" 
 

xr_vector<voiceData> datas;
xr_vector<voiceData> datas_recived;
bool Capture = false;
bool stopPlay = false;
float gain = 0.0f;



void UpdateMicro(void* args)
{
 	ALCdevice* dev[2];
	ALint CaptureSize;
	char data[5000];

	dev[1] = alcCaptureOpenDevice(NULL, 22050, AL_FORMAT_MONO16, sizeof(data) / 2);
	alcCaptureStart(dev[1]);
	
	CTimer Timer;
	Timer.Start();
 
	while (Capture)
	{	
		//—мотрим сколько самплов 	
		alcGetIntegerv(dev[1], ALC_CAPTURE_SAMPLES, 1, &CaptureSize);
	 	   
		if (CaptureSize <= 0)
			continue;

		voiceData voice_data;
		/* —читываем запись */
		alcCaptureSamples(dev[1], data, CaptureSize);

		//Msg("CaptureSize %d, ms[%d]", CaptureSize, Timer.GetElapsed_ms());
  
		memcpy(voice_data.data, data, sizeof(data) );
		voice_data.size = CaptureSize;
		datas.push_back(voice_data);

		Sleep(100);
	}
    
	alcCaptureStop(dev[1]);
	alcCaptureCloseDevice(dev[1]);

}

void UpdateListner(void* args)
{
	char data[5000];
	ALuint source, buffers[3];
	ALuint buf;

	alGenSources(1, &source);
	alGenBuffers(3, buffers);
   
	/* Setup some initial silent data to play out of the source */
	alBufferData(buffers[0], AL_FORMAT_MONO16, data, sizeof(data), 22050);
	alBufferData(buffers[1], AL_FORMAT_MONO16, data, sizeof(data), 22050);
	alBufferData(buffers[2], AL_FORMAT_MONO16, data, sizeof(data), 22050);

	alSourceQueueBuffers(source, 3, buffers);

	 
	//alDistanceModel(AL_NONE);
	alSourcePlay(source);

	ALint CaptureD;

	while (datas_recived.size() > 1)
	{
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &CaptureD);

		if (CaptureD <= 0)
		{
			Sleep(50);
			continue;
		}

		Msg("SizeRecive [%d]", datas_recived.size());

		voiceData recived = datas_recived.back();

		datas_recived.pop_back();
	    
		alSourceUnqueueBuffers(source, 1, &buf);
		alBufferData(buf, AL_FORMAT_MONO16, recived.data, recived.size * 2, 22050);/* bytes here, not frames */
		/*
		alSource3f(source, AL_POSITION, recived.pos.x, recived.pos.y, recived.pos.z);
		alSource3f(source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
		alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	
		alSourcef(source, AL_GAIN, gain);
		alSourcef(source, AL_PITCH, 1);
		alSourcef(source, AL_REFERENCE_DISTANCE, 5.0f);
 		alSourcef(source, AL_MAX_DISTANCE, 30.0f);
		*/

		alSourceQueueBuffers(source, 1, &buf);
 		
		alGetSourcei(source, AL_SOURCE_STATE, &CaptureD);
	

		if (CaptureD != AL_PLAYING)
		{
			alSourcePlay(source);
		}
		 
	}

	alSourceStop(source);
	alDeleteSources(1, &source);
	alDeleteBuffers(3, buffers);  

	stopPlay = true;
}


xrVoice::xrVoice()
{
	Voice_Export = xr_new<xrVoice>();
 	thread_spawn(UpdateListner, "xrVoicePlay", 0, 0);
	stopPlay = true;
}


xrVoice::~xrVoice()
{
	//DestroyVoice();
}


void xrVoice::createVoice()
{
	Capture = true;
	thread_spawn(UpdateMicro,   "xrVoice", 0, 0);	
}

voiceData xrVoice::CapturedVoice()
{
	if (datas.size() > 0)
	{
		voiceData dataVoice = datas.back();
		//Msg("Size[%d]", datas.size());
		datas.pop_back();
		return dataVoice;
	}
	else
	{
		return voiceData();
	}
	 
}

void xrVoice::UpdateCapture(bool value)
{
	Capture = value;
}

void xrVoice::PlayCapture(voiceData dataPlay)
{
	/*

	ALint CaptureD;
	ALuint buf;

	alGetSourcei(source, AL_BUFFERS_PROCESSED, &CaptureD);

	if (CaptureD <= 0)
	{
		Msg("Cant Play Alrady played [%d]", CaptureD);
		return;
	}

	Msg("net_import Capture [%d] / Sizeof[%d]", dataPlay.size, xr_strlen(dataPlay.data) );

	alSourceUnqueueBuffers(source, 1, &buf);
	alBufferData(buf, AL_FORMAT_MONO16, dataPlay.data, dataPlay.size * 2, 22050);/* bytes here, not frames
	alSourceQueueBuffers(source, 1, &buf);


	alGetSourcei(source, AL_SOURCE_STATE, &CaptureD);

	if (CaptureD != AL_PLAYING)
	{
		Msg("Start Play Source");
		alSourcePlay(source);
	}

	*/

	datas_recived.push_back(dataPlay);
   
	if (datas_recived.size() > 2 && stopPlay)
	{
		thread_spawn(UpdateListner, "xrVoicePlay", 0, 0); 
		stopPlay = false;
	}

}

bool xrVoice::getCaptureState()
{
	return Capture; 
}

u32 xrVoice::sizeCapture()
{
	return datas.size();
}

void xrVoice::setGain(float value)
{
	gain = value;
}
