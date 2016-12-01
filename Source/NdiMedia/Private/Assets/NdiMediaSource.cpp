// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaSource.h"
#include "NdiMediaPrivate.h"

#include "Interfaces/IPv4/IPv4Endpoint.h"


static FName AudioChannelsName("AudioChannels");
static FName AudioSampleRateName("AudioSampleRate");
static FName BandwidthName("Bandwidth");
static FName FrameRateDenominatorName("FrameRateD");
static FName FrameRateNumeratorName("FrameRateN");
static FName ProgressiveName("Progressive");
static FName VideoHeightName("VideoHeight");
static FName VideoWidthName("VideoWidth");


/* UNdiMediaSource structors
 *****************************************************************************/

UNdiMediaSource::UNdiMediaSource()
	: Bandwidth(ENdiMediaBandwidth::Highest)
	, PreferredNumAudioChannels(2)
	, PreferredAudioSampleRate(48000)
	, PreferredVideoWidth(0)
	, PreferredVideoHeight(0)
	, PreferredFrameRateNumerator(0)
	, PreferredFrameRateDenominator(0)
	, PreferredFrameFormat(ENdiMediaFrameFormatPreference::NoPreference)
{ }


/* IMediaOptions interface
 *****************************************************************************/

FString UNdiMediaSource::GetMediaOption(const FName& Key, const FString& DefaultValue) const
{
	if (Key == ProgressiveName)
	{
		if (PreferredFrameFormat == ENdiMediaFrameFormatPreference::Fielded)
		{
			return TEXT("false");
		}

		if (PreferredFrameFormat == ENdiMediaFrameFormatPreference::Progressive)
		{
			return TEXT("true");
		}
	}

	return Super::GetMediaOption(Key, DefaultValue);
}


int64 UNdiMediaSource::GetMediaOption(const FName& Key, int64 DefaultValue) const
{
	if (Key == AudioChannelsName)
	{
		return PreferredNumAudioChannels;
	}

	if (Key == AudioSampleRateName)
	{
		return PreferredAudioSampleRate;
	}

	if (Key == BandwidthName)
	{
		switch (Bandwidth)
		{
		case ENdiMediaBandwidth::AudioOnly:
			return NDIlib_recv_bandwidth_e::NDIlib_recv_bandwidth_audio_only;

		case ENdiMediaBandwidth::Lowest:
			return NDIlib_recv_bandwidth_e::NDIlib_recv_bandwidth_lowest;

		default:
			return NDIlib_recv_bandwidth_e::NDIlib_recv_bandwidth_highest;
		}
	}

	if (Key == VideoHeightName)
	{
		return PreferredVideoHeight;
	}

	if (Key == VideoWidthName)
	{
		return PreferredVideoWidth;
	}

	if (Key == FrameRateDenominatorName)
	{
		return PreferredFrameRateDenominator;
	}

	if (Key == FrameRateNumeratorName)
	{
		return PreferredFrameRateNumerator;
	}

	return Super::GetMediaOption(Key, DefaultValue);
}


bool UNdiMediaSource::HasMediaOption(const FName& Key) const
{
	if ((Key == AudioChannelsName) ||
		(Key == AudioSampleRateName) ||
		(Key == BandwidthName) ||
		(Key == FrameRateDenominatorName) ||
		(Key == FrameRateNumeratorName) ||
		(Key == ProgressiveName) ||
		(Key == VideoHeightName) ||
		(Key == VideoWidthName))
	{
		return true;
	}

	return Super::HasMediaOption(Key);
}


/* UMediaSource interface
 *****************************************************************************/

FString UNdiMediaSource::GetUrl() const
{
	return FString(TEXT("ndi://")) + SourceEndpoint;
}


bool UNdiMediaSource::Validate() const
{
	FIPv4Endpoint Endpoint;

	return FIPv4Endpoint::Parse(SourceEndpoint, Endpoint);
}
