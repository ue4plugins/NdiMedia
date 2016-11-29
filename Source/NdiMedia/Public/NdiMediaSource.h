// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
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
 * NDI source stream progressive video options.
 */
UENUM(BlueprintType)
enum class ENdiMediaFrameFormatPreference : uint8
{
	NoPreference,
	Fielded,
	Progressive
};


/**
 * Media source for NDI streams.
 */
UCLASS(BlueprintType, MinimalAPI, hidecategories=(Overrides, Playback))
class UNdiMediaSource
	: public UMediaSource
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UNdiMediaSource();

public:

	/** Desired bandwidth for the NDI stream (default = Highest). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	ENdiMediaBandwidth Bandwidth;

	/** The IP address and port number (e.g. "1.2.3.4:5678") of the NDI source to be played. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AssetRegistrySearchable)
	FString SourceEndpoint;

	/** Preferred number of audio channels (0 = no preference, default = 2). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	int32 PreferredNumAudioChannels;

	/** Preferred audio sample rate (in samples per second, 0 = no preference, default = 48000). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	int32 PreferredAudioSampleRate;

	/** Preferred width of the video stream (in pixels, 0 = no preference). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	int32 PreferredVideoWidth;

	/** Preferred height of the video stream (in pixels, 0 = no preference). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	int32 PreferredVideoHeight;

	/** Numerator of preferred video frame rate, i.e. '30000' in 30000/1001 = 29.97 fps (0 = no preference). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	int32 PreferredFrameRateNumerator;

	/** Denominator of preferred video frame rate, i.e. '1001' in 30000/1001 = 29.97 fps (0 = no preference). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	int32 PreferredFrameRateDenominator;

	/** Preferred video frame format type (default = NoPreference). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=NDI, AdvancedDisplay)
	ENdiMediaFrameFormatPreference PreferredFrameFormat;

public:

	//~ IMediaOptions interface

	virtual FString GetMediaOption(const FName& Key, const FString& DefaultValue) const override;
	virtual int64 GetMediaOption(const FName& Key, int64 DefaultValue) const override;

public:

	//~ UMediaSource interface

	virtual FString GetUrl() const override;
	virtual bool Validate() const override;
};
