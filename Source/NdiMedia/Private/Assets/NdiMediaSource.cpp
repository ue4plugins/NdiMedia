// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPCH.h"
#include "NdiMediaSource.h"


static FName BandwidthName("Bandwidth");


int64 UNdiMediaSource::GetMediaOption(const FName& Key, int64 DefaultValue) const
{
	if (Key != BandwidthName)
	{
		return DefaultValue;
	}

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


FString UNdiMediaSource::GetUrl() const
{
	return FString(TEXT("ndi://")) + SourceAddress;
}


bool UNdiMediaSource::Validate() const
{
	FIPv4Endpoint Endpoint;

	return FIPv4Endpoint::Parse(SourceAddress, Endpoint);
}
