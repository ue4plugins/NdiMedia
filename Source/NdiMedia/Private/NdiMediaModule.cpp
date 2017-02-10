// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPrivate.h"

#include "HAL/RunnableThread.h"
#include "IMediaModule.h"
#include "Modules/ModuleManager.h"

#include "INdiMediaModule.h"
#include "Ndi.h"
#include "NdiMediaAudioTicker.h"
#include "NdiMediaFinder.h"
#include "NdiMediaPlayer.h"


DEFINE_LOG_CATEGORY(LogNdiMedia);

#define LOCTEXT_NAMESPACE "FNdiMediaModule"


/**
 * Implements the NdiMedia module.
 */
class FNdiMediaModule
	: public INdiMediaModule
{
public:

	/** Default constructor. */
	FNdiMediaModule()
		: Initialized(false)
	{ }

public:

	//~ INdiMediaModule interface

	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer() override
	{
		if (!Initialized)
		{
			return nullptr;
		}

		IMediaModule* MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");

		if (MediaModule == nullptr)
		{
			return nullptr;
		}

		TSharedRef<FNdiMediaPlayer, ESPMode::ThreadSafe> NewPlayer = MakeShareable(new FNdiMediaPlayer());
		
		MediaModule->RegisterTickable(NewPlayer);
		AudioTicker->RegisterTickable(NewPlayer);

		return NewPlayer;
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		if (!FNdi::Initialize())
		{
			UE_LOG(LogNdiMedia, Error, TEXT("Failed to initialize NDI"));
			return;
		}

		IMediaModule* MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");

		if (MediaModule == nullptr)
		{
			return;
		}

		// create audio ticker thread
		AudioTicker = MakeShareable(new FNdiMediaAudioTicker());
		AudioTickerThread = FRunnableThread::Create(AudioTicker.Get(), TEXT("FNdiMediaAudioTicker"));
		MediaModule->RegisterTickable(AudioTicker.ToSharedRef());

		// initialize NDI finder
		GetMutableDefault<UNdiMediaFinder>()->Initialize();

		Initialized = true;
	}

	virtual void ShutdownModule() override
	{
		if (!Initialized)
		{
			return;
		}

		Initialized = false;

		// destroy audio ticker thread
		AudioTickerThread->Kill(true);
		delete AudioTickerThread;
		AudioTickerThread = nullptr;
		AudioTicker.Reset();

		// shut down NDI
		FNdi::Shutdown();
	}

private:

	/** The audio ticker runnable. */
	TSharedPtr<FNdiMediaAudioTicker, ESPMode::ThreadSafe> AudioTicker;

	/** The audio ticker thread. */
	FRunnableThread* AudioTickerThread;

	/** Whether the module has been initialized. */
	bool Initialized;
};


IMPLEMENT_MODULE(FNdiMediaModule, NdiMedia);


#undef LOCTEXT_NAMESPACE
