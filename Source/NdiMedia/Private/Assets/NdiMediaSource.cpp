// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaSource.h"
#include "NdiMediaPrivate.h"

#include "Interfaces/IPv4/IPv4Endpoint.h"


/* UNdiMediaSource structors
 *****************************************************************************/

UNdiMediaSource::UNdiMediaSource()
	: Bandwidth(ENdiMediaBandwidth::Highest)
	, ColorFormat(ENdiMediaColorFormat::UYVY)
	, PreferredNumAudioChannels(2)
	, PreferredAudioSampleRate(48000)
	, PreferredVideoWidth(0)
	, PreferredVideoHeight(0)
	, PreferredFrameRateNumerator(0)
	, PreferredFrameRateDenominator(0)
	, PreferredFrameFormat(ENdiMediaFrameFormatPreference::NoPreference)
	, UseTimecode(false)
{ }


/* IMediaOptions interface
 *****************************************************************************/

bool UNdiMediaSource::GetMediaOption(const FName& Key, bool DefaultValue) const
{
	if (Key == NdiMedia::UseTimecodeOption)
	{
		return UseTimecode;
	}

	return Super::GetMediaOption(Key, DefaultValue);
}


FString UNdiMediaSource::GetMediaOption(const FName& Key, const FString& DefaultValue) const
{
	if (Key == NdiMedia::ProgressiveOption)
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
	if (Key == NdiMedia::AudioChannelsOption)
	{
		return PreferredNumAudioChannels;
	}

	if (Key == NdiMedia::AudioSampleRateOption)
	{
		return PreferredAudioSampleRate;
	}

	if (Key == NdiMedia::BandwidthOption)
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

	if (Key == NdiMedia::ColorFormatOption)
	{
		return (int64)ColorFormat;
	}

	if (Key == NdiMedia::VideoHeightOption)
	{
		return PreferredVideoHeight;
	}

	if (Key == NdiMedia::VideoWidthOption)
	{
		return PreferredVideoWidth;
	}

	if (Key == NdiMedia::FrameRateDOption)
	{
		return PreferredFrameRateDenominator;
	}

	if (Key == NdiMedia::FrameRateNOption)
	{
		return PreferredFrameRateNumerator;
	}

	return Super::GetMediaOption(Key, DefaultValue);
}


bool UNdiMediaSource::HasMediaOption(const FName& Key) const
{
	if ((Key == NdiMedia::AudioChannelsOption) ||
		(Key == NdiMedia::AudioSampleRateOption) ||
		(Key == NdiMedia::BandwidthOption) ||
		(Key == NdiMedia::ColorFormatOption) ||
		(Key == NdiMedia::FrameRateDOption) ||
		(Key == NdiMedia::FrameRateNOption) ||
		(Key == NdiMedia::ProgressiveOption) ||
		(Key == NdiMedia::UseTimecodeOption) ||
		(Key == NdiMedia::VideoHeightOption) ||
		(Key == NdiMedia::VideoWidthOption))
	{
		return true;
	}

	return Super::HasMediaOption(Key);
}


/* UMediaSource interface
 *****************************************************************************/

FString UNdiMediaSource::GetUrl() const
{
	return FString(TEXT("ndi://")) + (SourceName.IsEmpty() ? SourceEndpoint : SourceName);
}


bool UNdiMediaSource::Validate() const
{
	if (!SourceName.IsEmpty())
	{
		return true;
	}

	FIPv4Endpoint Endpoint;

	return FIPv4Endpoint::Parse(SourceEndpoint, Endpoint);
}
