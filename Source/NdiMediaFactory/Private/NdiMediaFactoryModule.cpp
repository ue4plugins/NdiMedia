// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "IMediaPlayerFactory.h"
#include "IMediaModule.h"
#include "INdiMediaModule.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ModuleInterface.h"
#include "ModuleManager.h"
#include "NdiMediaSettings.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"


#define LOCTEXT_NAMESPACE "FNdiMediaFactoryModule"


/**
 * Implements the NdiMediaFactory module.
 */
class FNdiMediaFactoryModule
	: public IMediaPlayerFactory
	, public IModuleInterface
{
public:

	//~ IMediaPlayerFactory interface

	virtual bool CanPlayUrl(const FString& Url, const IMediaOptions& Options, TArray<FText>* OutWarnings, TArray<FText>* OutErrors) const override
	{
		FString Scheme;
		FString Location;

		// check scheme
		if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(LOCTEXT("NoSchemeFound", "No URI scheme found"));
			}

			return false;
		}

		if (!SupportedUriSchemes.Contains(Scheme))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(FText::Format(LOCTEXT("SchemeNotSupported", "The URI scheme '{0}' is not supported"), FText::FromString(Scheme)));
			}

			return false;
		}

		return true;
	}

	virtual TSharedPtr<IMediaPlayer> CreatePlayer() override
	{
		auto NdiMediaModule = FModuleManager::LoadModulePtr<INdiMediaModule>("NdiMedia");
		return (NdiMediaModule != nullptr) ? NdiMediaModule->CreatePlayer() : nullptr;
	}

	virtual FText GetDisplayName() const override
	{
		return LOCTEXT("MediaPlayerDisplayName", "NDI Media Player");
	}

	virtual FName GetName() const override
	{
		static FName PlayerName(TEXT("NdiMedia"));
		return PlayerName;
	}

	virtual const TArray<FString>& GetSupportedPlatforms() const override
	{
		return SupportedPlatforms;
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		// supported platforms
		SupportedPlatforms.Add(TEXT("Linux"));
		SupportedPlatforms.Add(TEXT("Mac"));
		SupportedPlatforms.Add(TEXT("Windows"));

		// supported schemes
		SupportedUriSchemes.Add(TEXT("ndi"));

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
				SettingsSection->OnModified().BindRaw(this, &FNdiMediaFactoryModule::HandleSettingsSaved);
			}
		}
#endif //WITH_EDITOR

		// register player factory
		auto MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");

		if (MediaModule != nullptr)
		{
			MediaModule->RegisterPlayerFactory(*this);
		}
	}

	virtual void ShutdownModule() override
	{
		// unregister player factory
		auto MediaModule = FModuleManager::GetModulePtr<IMediaModule>("Media");

		if (MediaModule != nullptr)
		{
			MediaModule->UnregisterPlayerFactory(*this);
		}

#if WITH_EDITOR
		// unregister settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "NdiMedia");
		}
#endif //WITH_EDITOR
	}

private:

	/** Callback for when the settings were saved. */
	bool HandleSettingsSaved()
	{
		return true;
	}

private:

	/** List of supported media file types. */
	TArray<FString> SupportedFileExtensions;

	/** List of platforms that the media player support. */
	TArray<FString> SupportedPlatforms;

	/** List of supported URI schemes. */
	TArray<FString> SupportedUriSchemes;
};


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNdiMediaFactoryModule, NdiMediaFactory);
