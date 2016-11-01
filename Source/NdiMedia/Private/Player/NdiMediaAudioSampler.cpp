// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPCH.h"
#include "NdiMediaAudioSampler.h"


/* FNdiMediaAudioSampler structors
 *****************************************************************************/

FNdiMediaAudioSampler::FNdiMediaAudioSampler()
	: ReceiverInstance(nullptr)
	, Stopping(false)
{
	Thread = FRunnableThread::Create(this, TEXT("FNdiMediaAudioSampler"));
}


FNdiMediaAudioSampler::~FNdiMediaAudioSampler()
{
	Thread->Kill(true);
	delete Thread;
	Thread = nullptr;
}


/* FNdiMediaAudioSampler interface
 *****************************************************************************/

void FNdiMediaAudioSampler::SetReceiverInstance(void* InReceiverInstance)
{
	FScopeLock Lock(&CriticalSection);
	ReceiverInstance = InReceiverInstance;
}


/* FRunnable interface
 *****************************************************************************/

bool FNdiMediaAudioSampler::Init()
{
	return true;
}


uint32 FNdiMediaAudioSampler::Run()
{
	while (!Stopping)
	{
		SampleAudio(1000);
	}
	
	return 0;
}


void FNdiMediaAudioSampler::Stop()
{
	Stopping = true;
}


/* FNdiMediaAudioSampler implementation
 *****************************************************************************/

void FNdiMediaAudioSampler::SampleAudio(uint32 Timeout)
{
	NDIlib_audio_frame_t AudioFrame;
	{
		FScopeLock Lock(&CriticalSection);

		if (ReceiverInstance == nullptr)
		{
			return;
		}

		NDIlib_frame_type_e FrameType = NDIlib_recv_capture(ReceiverInstance, nullptr, &AudioFrame, nullptr, Timeout);

		if (FrameType == NDIlib_frame_type_error)
		{
			UE_LOG(LogNdiMedia, Verbose, TEXT("Failed to receive audio frame"));
			return;
		}

		if (FrameType != NDIlib_frame_type_audio)
		{
			return;
		}
	}

	SamplesDelegate.ExecuteIfBound(AudioFrame);
	NDIlib_recv_free_audio(ReceiverInstance, &AudioFrame);
}
