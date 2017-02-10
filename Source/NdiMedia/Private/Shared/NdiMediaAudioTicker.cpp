// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaAudioTicker.h"

#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ScopeLock.h"
#include "INdiMediaAudioTickable.h"


/* FNdiMediaAudioTicker structors
 *****************************************************************************/

FNdiMediaAudioTicker::FNdiMediaAudioTicker()
	: CurrentTimecode(FTimespan::Zero())
	, Stopping(false)
{
	WakeupEvent = FPlatformProcess::GetSynchEventFromPool(true);
}


FNdiMediaAudioTicker::~FNdiMediaAudioTicker()
{
	FPlatformProcess::ReturnSynchEventToPool(WakeupEvent);
	WakeupEvent = nullptr;
}


/* FRunnable interface
 *****************************************************************************/

void FNdiMediaAudioTicker::RegisterTickable(const TSharedRef<INdiMediaAudioTickable, ESPMode::ThreadSafe>& Tickable)
{
	FScopeLock Lock(&CriticalSection);
	Tickables.AddUnique(Tickable);
	WakeupEvent->Trigger();
}


/* FRunnable interface
 *****************************************************************************/

bool FNdiMediaAudioTicker::Init()
{
	return true;
}


uint32 FNdiMediaAudioTicker::Run()
{
	while (!Stopping)
	{
		if (WakeupEvent->Wait(FTimespan::MaxValue()))
		{
			FScopeLock Lock(&CriticalSection);

			for (int32 TickableIndex = Tickables.Num() - 1; TickableIndex >= 0; --TickableIndex)
			{
				auto Tickable = Tickables[TickableIndex].Pin();

				if (Tickable.IsValid())
				{
					Tickable->TickAudio(CurrentTimecode);
				}
				else
				{
					Tickables.RemoveAtSwap(TickableIndex);
				}
			}

			if (Tickables.Num() == 0)
			{
				WakeupEvent->Reset();
			}
			else
			{
				FPlatformProcess::Sleep(0.005f);
			}
		}
	}
	
	return 0;
}


void FNdiMediaAudioTicker::Stop()
{
	Stopping = true;
	WakeupEvent->Trigger();
}


/* IMediaTickable interface
 *****************************************************************************/

void FNdiMediaAudioTicker::TickInput(FTimespan Timecode, FTimespan DeltaTime, bool Locked)
{
	CurrentTimecode = Timecode;
}
