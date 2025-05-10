#include "stdafx.h"
#include "Sound.h"
#include "SoundRecorderA.h"
#include "SpeexPreprocess.h"
#include "VoicePacketsPacker.h"
#include "ISoundRecorder.h"

namespace
{
	union ShortByteUnion {
		signed  short asShort;
		unsigned char asBytes[2];
	};
}

ALenum CheckOpenALError()
{
	auto error = alGetError();
	switch (error)
	{
		case AL_NO_ERROR:
		{
			Msg("Create Voice Recorder Device: reason: AL_NO_ERROR");
		}break;
		case AL_INVALID_NAME:
		{
			Msg("~~~ Cannot Create Voice Recorder Device: reason: AL_INVALID_NAME");
		}break;
		case AL_INVALID_ENUM:
		{
			Msg("~~~ Cannot Create Voice Recorder Device: reason: AL_INVALID_ENUM");
		}break;
		case AL_INVALID_VALUE:
		{
			Msg("~~~ Cannot Create Voice Recorder Device: reason: AL_INVALID_VALUE");
		}break;
		case AL_INVALID_OPERATION:
		{
			Msg("~~~ Cannot Create Voice Recorder Device: reason: AL_INVALID_OPERATION");

		}break;
		case AL_OUT_OF_MEMORY:
		{
			Msg("~~~ Cannot Create Voice Recorder Device: reason: AL_OUT_OF_MEMORY");
		}break;

		default: {

			Msg("~~~ Cannot Create Voice Recorder Device: Uncached: %u", error);
		}break;

	};

	return error;
}

CSoundRecorderA::CSoundRecorderA(ALuint sampleRate, ALenum format, ALuint samplesPerBuffer)
	: m_sampleRate(sampleRate), m_format(format), m_samplesPerBuffer(samplesPerBuffer)
{
	m_bytesPerSample = 1;

	switch (format)
	{
	case AL_FORMAT_MONO16:
		m_bytesPerSample = 2;
		break;
	case AL_FORMAT_STEREO8:
		m_bytesPerSample = 2;
		break;
	case AL_FORMAT_STEREO16:
		m_bytesPerSample = 4;
		break;
	default:
		break;
	}

	m_buffer = new ALbyte[samplesPerBuffer * m_bytesPerSample];
	m_speexPreprocess = xr_new<CSpeexPreprocess>(sampleRate, samplesPerBuffer);
	m_speexPreprocess->EnableAGC(psSoundRecorderMode);
	m_speexPreprocess->EnableDenoise(psSoundRecorderDenoise);
}


CSoundRecorderA::~CSoundRecorderA()
{
	Destroy();

	delete[] m_buffer;
	m_buffer = nullptr;

	xr_delete(m_speexPreprocess);
	m_speexPreprocess = nullptr;
}

bool CSoundRecorderA::Init(CVoicePacketsPacker* packetsPacker)
{
	Msg("OpenAl Initialize Capture Device");
 	m_packetsPacker = packetsPacker;
  	m_pCaptureDevice = alcCaptureOpenDevice(0, m_sampleRate, m_format, m_samplesPerBuffer * 2);
 	
 	ALenum error = CheckOpenALError();
	if (error == AL_NO_ERROR)
	{ 
		Msg("Start Capture Device: %p", m_pCaptureDevice);
		alcCaptureStart(m_pCaptureDevice);
		return true;
	}
	 
	Msg("Cant Open Capture Device: %p", m_pCaptureDevice);
	return false;
}

void CSoundRecorderA::Destroy()
{
	if (m_pCaptureDevice == nullptr)
		return;

	alcCaptureStop(m_pCaptureDevice);
	alcCaptureCloseDevice(m_pCaptureDevice);
	m_pCaptureDevice = nullptr;
}

void CSoundRecorderA::Start()
{
	m_started = true;
}

void CSoundRecorderA::Stop()
{
	m_started = false;
}

void CSoundRecorderA::Update()
{
	if (m_pCaptureDevice == nullptr)
	{
		Msg("~~~~ NO Device to Capture Voice: %p", m_pCaptureDevice);
		return;
	}
	 
	ALint samples = 0;
	alcGetIntegerv(m_pCaptureDevice, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &samples);

	if (samples < m_samplesPerBuffer)
		return;

	alcCaptureSamples(m_pCaptureDevice, m_buffer, m_samplesPerBuffer);

	if (m_started && m_packetsPacker)
	{
		ALuint availableBytes = m_samplesPerBuffer * m_bytesPerSample;

		if (psSoundRecorderMode)
		{
			if (!m_speexPreprocess->IsAGCEnabled())
				m_speexPreprocess->EnableAGC(true);
		}
		else
		{
			if (m_speexPreprocess->IsAGCEnabled())
				m_speexPreprocess->EnableAGC(false);
		}

		if (psSoundRecorderDenoise)
		{
			if (!m_speexPreprocess->IsDenoiseEnabled())
				m_speexPreprocess->EnableDenoise(true);
		}
		else
		{
			if (m_speexPreprocess->IsDenoiseEnabled())
				m_speexPreprocess->EnableDenoise(false);
		}

		if (psSoundRecorderMode == 0)
		{
			ChangeGain(m_buffer, availableBytes);
		}

		if (psSoundRecorderMode || psSoundRecorderDenoise)
		{
			m_speexPreprocess->RunPreprocess((short*)m_buffer);
		}

		m_packetsPacker->AddPacket(m_buffer, availableBytes);
	}
}

void CSoundRecorderA::ChangeGain(ALbyte* buffer, ALint length)
{
	const float modifier = psSoundVRecorder;

	for (int i = 0; i < length; i += 2)
	{
		ShortByteUnion ab;
		ab.asBytes[0] = buffer[i];
		ab.asBytes[1] = buffer[i + 1];

		if ((float)ab.asShort >= 32676.f / modifier)
			ab.asShort = 32676;
		else
			ab.asShort = (short)((float)ab.asShort * modifier);

		buffer[i] = ab.asBytes[0];
		buffer[i + 1] = ab.asBytes[1];
	}
}
