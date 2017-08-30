// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "NdiMediaSettings.generated.h"


/**
 * Settings for the NdiMedia plug-in.
 */
UCLASS(config=Engine)
class NDIMEDIAFACTORY_API UNdiMediaSettings
	: public UObject
{
	GENERATED_UCLASS_BODY()

	/** Name of this product. */
	UPROPERTY(config, EditAnywhere, Category=Connection)
	FString ProductName;

	/** Optional longer description text for this product. */
	UPROPERTY(config, EditAnywhere, Category=Connection)
	FString ProductDescription;

	/** Manufacturer of this product. */
	UPROPERTY(config, EditAnywhere, Category=Connection)
	FString Manufacturer;

	/** Optional serial number (for product pairing). */
	UPROPERTY(config, EditAnywhere, Category=Connection, AdvancedDisplay)
	FString SerialNumber;

	/** Optional session name. */
	UPROPERTY(config, EditAnywhere, Category=Connection, AdvancedDisplay)
	FString SessionName;

	/** Custom XML metadata to attach to the connection. */
	UPROPERTY(config, EditAnywhere, Category=Connection, AdvancedDisplay, meta=(Multiline="true"))
	FString CustomMetaData;

public:

	/**
	 * Get the plug-in's version number.
	 *
	 * @return Version number.
	 */
	FString GetVersionName() const
	{
		return VersionName;
	}

private:

	/** Version number of this plug-in. */
	FString VersionName;
};
