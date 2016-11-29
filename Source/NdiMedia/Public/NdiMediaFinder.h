// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"

#include "NdiMediaFinder.generated.h"


/**
 * Identifies an NDI media source.
 */
USTRUCT(BlueprintType)
struct FNdiMediaSourceId
{
	GENERATED_BODY()

	/** Default constructor. */
	FNdiMediaSourceId() { }

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InEndpoint The NDI source's IP endpoint.
	 * @param InName The NDI source's name.
	 * @param InUrl The NDI source's URL.
	 */
	FNdiMediaSourceId(const FString& InEndpoint, const FString& InName, const FString& InUrl)
		: Endpoint(InEndpoint)
		, Name(InName)
		, Url(InUrl)
	{ }

	/** The IP endpoint of the NDI media source, i.e. "1.2.3.4:12345". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=NDI)
	FString Endpoint;

	/** The name of the NDI media source, i.e. "My NDI Stream". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=NDI)
	FString Name;

	/** The MediaPlayer compatible URL, i.e. "ndi://1.2.3.4:12345". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=NDI)
	FString Url;

public:

	/**
	 * Get a string representation of this source.
	 *
	 * @return String representation, i.e. "My Stream (1.2.3.4:12345)".
	 * @see ToMediaUrl
	 */
	FString ToString() const
	{
		return Name + TEXT(" [") + Endpoint + TEXT("]");
	}
};


//
// Asset for finding NDI streams.
//
UCLASS(BlueprintType)
class NDIMEDIA_API UNdiMediaFinder
	: public UObject
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UNdiMediaFinder();

public:

	/**
	 * Get the list of NDI media sources currently available on the network.
	 *
	 * @param OutSources Will contain the collection of found NDI source names and their URLs.
	 * @return true on success, false if the finder wasn't initialized.
	 * @see Initialize, Shutdown
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	bool GetSources(TArray<FNdiMediaSourceId>& OutSources) const;

	/**
	 * Initialize this finder and start discovering NDI sources on the network.
	 *
	 * @return true on success, false otherwise.
	 * @see GetSources, Shutdown
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	bool Initialize();

	/**
	 * Shut down this finder and stop discovering NDI sources on the network.
	 *
	 * @see GetSources, Initialize
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void Shutdown();

public:

	/**
	 * Whether NDI sources running on the local machine should be discovered.
	 *
	 * @return true if local sources are discovered, false otherwise.
	 * @see GetExtraAddresses, GetGroupFilters, GetSources, SetShowLocalSources
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	bool GetShowLocalSources() const
	{
		return ShowLocalSources;
	}

	/**
	 * Set whether NDI sources running on the local machine should be returned.
	 *
	 * @param ShowLocal Whether to show local sources.
	 * @see GetShowLocalSources
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void SetShowLocalSources(bool NewShowLocal);

public:

	/**
	 * Add an IP address that should be searched for NDI sources.
	 *
	 * @param Address The address to add.
	 * @see AddGroupFilter, ClearExtraAddresses, GetExtraAddresses, RemoveExtraAddress
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void AddExtraAddress(const FString& Address);

	/**
	 * Clear the list of IP addresses to be searched for NDI sources.
	 *
	 * @see AddExtraAddress, ClearGroupFilters, GetExtraAddresses, RemoveExtraAddress
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void ClearExtraAddresses();

	/**
	 * Get the list of extra IP addresses to search for NDI sources.
	 *
	 * @return Collection of IP group strings.
	 * @see AddExtraAddress, ClearExtraAddresses, GetGroupFilters, GetShowLocalSources, RemoveExtraAddress
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	const TArray<FString>& GetExtraAddresses() const
	{
		return ExtraAddresses;
	}

	/**
	 * Remove an IP address that should no longer be searched for NDI sources.
	 *
	 * @param Address The address to remove.
	 * @see AddExtraAddress, ClearExtraAddresses, GetExtraAddresses, RemoveGroupFilter
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void RemoveExtraAddress(const FString& Address);

public:

	/**
	 * Add an NDI group to return NDI sources for.
	 *
	 * @param GroupName The group to add.
	 * @see AddExtraAddress, ClearGroupFilters, GetGroupFilters, RemoveGroupFilter
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void AddGroupFilter(const FString& GroupName);

	/**
	 * Clear the list of NDI groups to return NDI sources for.
	 *
	 * @see AddGroupFilter, ClearExtraAddresses, GetGroupFilters, RemoveGroupFilter
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void ClearGroupFilters();

	/**
	 * Get the list of NDI group names to return sources for.
	 *
	 * @return Collection of group names.
	 * @see AddGroupFilter, ClearGroupFilters, GetExtraAddresses, GetShowLocalSources, RemoveGroupFilter
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	const TArray<FString>& GetGroupFilters() const
	{
		return GroupFilters;
	}

	/**
	 * Remove an NDI group to return sources for.
	 *
	 * @param GroupName The group to remove.
	 * @see AddGroupFilter, ClearGroupFilters, GetGroupFilters, RemoveExtraAddress
	 */
	UFUNCTION(BlueprintCallable, Category=NDI)
	void RemoveGroupFilter(const FString& GroupName);

public:

	//~ UObject interface

	virtual void BeginDestroy() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:

	/**
	 * The IP addresses of additional NDI sources to search.
	 *
	 * Use this setting to search additional IP addresses that cannot be discovered
	 * automatically via the built-in mDNS discovery, i.e. computers in a separate
	 * subnet or on the internet.
	 */
	UPROPERTY(EditAnywhere, Category=Discovery, AdvancedDisplay)
	TArray<FString> ExtraAddresses;

	/**
	 * Optional list of NDI groups for which to find sources.
	 *
	 * Use this setting to limit the search for NDI media sources to a particular group.
	 * If this field is empty, all groups will be searched.
	 */
	UPROPERTY(EditAnywhere, Category=Discovery, AdvancedDisplay)
	TArray<FString> GroupFilters;

	/** Whether to include sources running on the local machine (default = true). */
	UPROPERTY(EditAnywhere, Category=Discovery)
	bool ShowLocalSources;

private:

	/** The NDI source finder instance. */
	void* FindInstance;
};
