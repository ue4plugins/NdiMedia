// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "IMediaTickable.h"

class FEvent;
class FNdiMediaPlayer;
class INdiMediaAudioTickable;


/**
 * High frequency ticker for audio frame processing.
 */
class FNdiMediaAudioTicker
	: public FRunnable
	, public IMediaTickable
{
public:

	/** Default constructor. */
	FNdiMediaAudioTicker();

	/** Destructor. */
	virtual ~FNdiMediaAudioTicker();

public:

	/**
	 * Register an object to be ticked from the audio ticker.
	 *
	 * @param Tickable The object to be registered.
	 */
	void RegisterTickable(const TSharedRef<INdiMediaAudioTickable, ESPMode::ThreadSafe>& Tickable);

public:

	//~ FRunnable interface

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override { }

public:

	//~ IMediaTickable interface

	virtual void TickInput(FTimespan Timecode, FTimespan DeltaTime, bool Locked) override;

private:

	/** Critical section for synchronizing access to the player collection. */
	FCriticalSection CriticalSection;

	/** The current media time code. */
	FTimespan CurrentTimecode;

	/** Holds a flag indicating that the thread is stopping. */
	bool Stopping;

	/** Collection of players to be ticked. */
	TArray<TWeakPtr<INdiMediaAudioTickable, ESPMode::ThreadSafe>> Tickables;

	/** Holds an event signaling the thread to wake up. */
	FEvent* WakeupEvent;
};