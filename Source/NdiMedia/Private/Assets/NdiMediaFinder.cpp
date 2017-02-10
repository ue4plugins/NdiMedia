// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaFinder.h"

#include "Ndi.h"
#include "NdiMediaPrivate.h"


/* UNdiMediaFinder structors
 *****************************************************************************/

UNdiMediaFinder::UNdiMediaFinder()
	: ShowLocalSources(true)
	, FindInstance(nullptr)
{ }


/* UNdiMediaFinder interface
 *****************************************************************************/

void UNdiMediaFinder::AddExtraAddress(const FString& Address)
{
	if (!ExtraAddresses.Contains(Address))
	{
		ExtraAddresses.Add(Address);
		Initialize();
	}
}


void UNdiMediaFinder::AddGroupFilter(const FString& GroupName)
{
	if (!GroupFilters.Contains(GroupName))
	{
		GroupFilters.Add(GroupName);
		Initialize();
	}
}


void UNdiMediaFinder::ClearExtraAddresses()
{
	if (ExtraAddresses.Num() > 0)
	{
		ExtraAddresses.Empty();
		Initialize();
	}
}


void UNdiMediaFinder::ClearGroupFilters()
{
	if (GroupFilters.Num() > 0)
	{
		GroupFilters.Empty();
		Initialize();
	}
}


bool UNdiMediaFinder::GetSources(TArray<FNdiMediaSourceId>& OutSources) const
{
	if (!FNdi::IsInitialized() || (FindInstance == nullptr))
	{
		return false;
	}

	int NumSources = 0;
	const NDIlib_source_t* Sources = NDIlib_find_get_sources(FindInstance, &NumSources, 0);

	for (int SourceIndex = 0; SourceIndex < NumSources; ++SourceIndex)
	{
		const NDIlib_source_t& Source = Sources[SourceIndex];
		OutSources.Add(FNdiMediaSourceId(
			ANSI_TO_TCHAR(Source.p_ip_address),
			ANSI_TO_TCHAR(Source.p_ndi_name)
		));
	}

	return true;
}


bool UNdiMediaFinder::Initialize()
{
	Shutdown();

	if (!FNdi::IsInitialized())
	{
		return false;
	}

	FString ExtraAddressesString = FString::Join(ExtraAddresses, TEXT(","));
	FString GroupsString = FString::Join(GroupFilters, TEXT(","));

	NDIlib_find_create_t FindCreate;
	{
		FindCreate.show_local_sources = ShowLocalSources;
		FindCreate.p_extra_ips = TCHAR_TO_ANSI(*ExtraAddressesString);
		FindCreate.p_groups = TCHAR_TO_ANSI(*GroupsString);
	}

	FindInstance = NDIlib_find_create2(&FindCreate);

	if (FindInstance == nullptr)
	{
		UE_LOG(LogNdiMedia, Warning, TEXT("Failed to create NDI Find instance"));
		return false;
	}

	return true;
}


void UNdiMediaFinder::RemoveExtraAddress(const FString& Address)
{
	if (ExtraAddresses.Remove(Address) > 0)
	{
		Initialize();
	}
}


void UNdiMediaFinder::RemoveGroupFilter(const FString& GroupName)
{
	if (GroupFilters.Remove(GroupName) > 0)
	{
		Initialize();
	}
}


void UNdiMediaFinder::SetShowLocalSources(bool NewShowLocal)
{
	if (NewShowLocal != ShowLocalSources)
	{
		ShowLocalSources = NewShowLocal;
		Initialize();
	}
}


void UNdiMediaFinder::Shutdown()
{
	if (FindInstance == nullptr)
	{
		return;
	}

	if (FNdi::IsInitialized())
	{
		NDIlib_find_destroy(FindInstance);
	}

	FindInstance = nullptr;
}


/* UObject interface
 *****************************************************************************/

void UNdiMediaFinder::BeginDestroy()
{
	Super::BeginDestroy();
	Shutdown();
}


#if WITH_EDITOR

void UNdiMediaFinder::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Initialize();
}

#endif //WITH_EDITOR
