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
	static FName AudioChannelsOption("AudioChannels");

	/** Name of the AudioSampleRate media option. */
	static FName AudioSampleRateOption("AudioSampleRate");

	/** Name of the Bandwidth media option. */
	static FName BandwidthOption("Bandwidth");

	/** Name of the FrameRateDenominator media option. */
	static FName FrameRateDOption("FrameRateD");

	/** Name of the FrameRateNumerator media option. */
	static FName FrameRateNOption("FrameRateN");

	/** Name of the Progressive media option. */
	static FName ProgressiveOption("Progressive");

	/** Name of the VideoHeight media option. */
	static FName VideoHeightOption("VideoHeight");

	/** Name of the VideoWidth media option. */
	static FName VideoWidthOption("VideoWidth");
}