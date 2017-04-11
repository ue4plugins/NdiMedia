// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "INdiMediaModule.h"
#include "NdiMediaPrivate.h"

#include "ModuleManager.h"
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

	virtual TSharedPtr<IMediaPlayer> CreatePlayer() override
	{
		if (!Initialized)
		{
			return nullptr;
		}

		return MakeShareable(new FNdiMediaPlayer());
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
