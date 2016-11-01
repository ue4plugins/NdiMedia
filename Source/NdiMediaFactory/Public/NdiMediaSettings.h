// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NdiMediaSettings.generated.h"


/**
 * Settings for the NdiMedia module.
 */
UCLASS(config=Engine)
class NDIMEDIAFACTORY_API UNdiMediaSettings
	: public UObject
{
	GENERATED_UCLASS_BODY()

	/** Name of this product. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Connection)
	FString ProductName;

	/** Optional longer description text for this product. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Connection)
	FString ProductDescription;

	/** Manufacturer of this product. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Connection)
	FString Manufacturer;

	/** Optional serial number (for product pairing). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Connection, AdvancedDisplay)
	FString SerialNumber;

	/** Optional session name. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Connection, AdvancedDisplay)
	FString SessionName;

	/** Custom XML metadata to attach to the connection. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Connection, AdvancedDisplay, meta=(Multiline="true"))
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
