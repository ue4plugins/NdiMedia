// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"


struct NDIlib_audio_frame_v2_t;


/** Delegate that is executed when new audio samples are ready for playback. */
DECLARE_DELEGATE_OneParam(FOnNdiMediaAudioSamplerSamples, const NDIlib_audio_frame_v2_t& /*AudioFrame*/);


class FNdiMediaAudioSampler
	: public FRunnable
{
public:

	/** Default constructor. */
	FNdiMediaAudioSampler();

	/** Destructor. */
	virtual ~FNdiMediaAudioSampler();

public:

	/**
	 * Get a delegate that is executed when new audio samples are ready for playback.
	 *
	 * @return The delegate.
	 */
	FOnNdiMediaAudioSamplerSamples& OnSamples()
	{
		return SamplesDelegate;
	}

	/**
	 * Set the receiver instance.
	 *
	 * @param InReceiverInstance The receiver instance to sample, or nullptr to suspend sampling.
	 */
	void SetReceiverInstance(void* InReceiverInstance);

public:

	//~ FRunnable interface

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override { }

protected:

	/**
	 * Sample the current audio frame.
	 *
	 * @param Timeout How long to wait for samples (in milliseconds).
	 */
	void SampleAudio(uint32 Timeout);

private:

	/** Critical section for synchronizing access to receiver. */
	FCriticalSection CriticalSection;

	/** The current receiver instance. */
	void* ReceiverInstance;

	/** Delegate that is executed when new audio samples are ready for playback. */
	FOnNdiMediaAudioSamplerSamples SamplesDelegate;

	/** Holds a flag indicating that the thread is stopping. */
	bool Stopping;

	/** Holds the thread object. */
	FRunnableThread* Thread;
};