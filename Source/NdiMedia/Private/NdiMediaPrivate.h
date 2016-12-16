// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#define NDIMEDIA_DLL_PLATFORM (PLATFORM_LINUX || PLATFORM_MAC || PLATFORM_WINDOWS)


#include "NdiMediaAllowPlatformTypes.h"
	#include "Processing.NDI.Lib.h"
#include "NdiMediaHidePlatformTypes.h"

#include "Runtime/Core/Public/CoreMinimal.h"

#include "../../NdiMediaFactory/Public/NdiMediaSettings.h"


DECLARE_LOG_CATEGORY_EXTERN(LogNdiMedia, Log, All);


namespace NdiMedia
{
	/** Name of the AudioChannels media option. */
	static const FName AudioChannelsOption("AudioChannels");

	/** Name of the AudioSampleRate media option. */
	static const FName AudioSampleRateOption("AudioSampleRate");

	/** Name of the Bandwidth media option. */
	static const FName BandwidthOption("Bandwidth");

	/** Name of the ColorFormat media option. */
	static const FName ColorFormatOption("ColorFormat");

	/** Name of the FrameRateDenominator media option. */
	static const FName FrameRateDOption("FrameRateD");

	/** Name of the FrameRateNumerator media option. */
	static const FName FrameRateNOption("FrameRateN");

	/** Name of the Progressive media option. */
	static const FName ProgressiveOption("Progressive");

	/** Name of the VideoHeight media option. */
	static const FName VideoHeightOption("VideoHeight");

	/** Name of the VideoWidth media option. */
	static const FName VideoWidthOption("VideoWidth");
}