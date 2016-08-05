// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MediaSource.h"
#include "NdiMediaSource.generated.h"


/**
 * NDI source stream bandwidth options.
 */
UENUM(BlueprintType)
enum class ENdiMediaBandwidth : uint8
{
	/** Highest quality audio and video. */
	Highest,

	/** Lowest quality audio and video. */
	Lowest,

	/** Receive audio stream only. */
	AudioOnly
};


/**
 * Media source for NDI streams.
 */
UCLASS(BlueprintType, MinimalAPI, hidecategories=(Playback))
class UNdiMediaSource
	: public UMediaSource
{
	GENERATED_BODY()

public:

	/** Desired bandwidth for the NDI stream (default = Highest. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	TEnumAsByte<ENdiMediaBandwidth> Bandwidth;

	/** The IP address and port number (Address:Port) of the NDI source to be played. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AssetRegistrySearchable)
	FString SourceAddress;

public:

	//~ IMediaOptions interface

	virtual int64 GetMediaOption(const FName& Key, int64 DefaultValue) const override;

public:

	//~ UMediaSource interface

	virtual FString GetUrl() const override;
	virtual bool Validate() const override;
};
