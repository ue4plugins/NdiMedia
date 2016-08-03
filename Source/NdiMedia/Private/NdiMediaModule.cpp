// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "NdiMediaPCH.h"
#include "INdiMediaModule.h"
//#include "Ndi.h"
#include "NdiMediaPlayer.h"
#include "NdiMediaSettings.h"


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
		// initialize NDI
		if (!NDIlib_is_supported_CPU())
		{
			UE_LOG(LogNdiMedia, Error, TEXT("Cannot initialize NDI: CPU is not supported"));

			return;
		}

		if (!NDIlib_initialize())
		{
			UE_LOG(LogNdiMedia, Error, TEXT("Failed to initialize NDI"));

			return;
		}

#if WITH_EDITOR
		// register settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "NdiMedia",
				LOCTEXT("NdiMediaSettingsName", "NDI Media"),
				LOCTEXT("NdiMediaSettingsDescription", "Configure the NDI Media plug-in."),
				GetMutableDefault<UNdiMediaSettings>()
			);

			if (SettingsSection.IsValid())
			{
				SettingsSection->OnModified().BindRaw(this, &FNdiMediaModule::HandleSettingsSaved);
			}
		}
#endif // WITH_EDITOR

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
		NDIlib_destroy();

#if WITH_EDITOR
		// unregister settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "NdiMedia");
		}
#endif // WITH_EDITOR
	}

private:

	/** Callback for when the settings were saved. */
	bool HandleSettingsSaved()
	{
		return true;
	}

private:

	/** Whether the module has been initialized. */
	bool Initialized;
};


IMPLEMENT_MODULE(FNdiMediaModule, NdiMedia);


#undef LOCTEXT_NAMESPACE
