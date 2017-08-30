// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPrivate.h"

#include "Modules/ModuleManager.h"

#include "INdiMediaModule.h"
#include "Ndi.h"
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

	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) override
	{
		if (!Initialized)
		{
			return nullptr;
		}
		return MakeShared<FNdiMediaPlayer, ESPMode::ThreadSafe>(EventSink);
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		// initialize NDI
		if (!FNdi::Initialize())
		{
			UE_LOG(LogNdiMedia, Error, TEXT("Failed to initialize NDI"));
			return;
		}

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

		// shut down NDI
		FNdi::Shutdown();
	}

private:

	/** Whether the module has been initialized. */
	bool Initialized;
};


IMPLEMENT_MODULE(FNdiMediaModule, NdiMedia);


#undef LOCTEXT_NAMESPACE
