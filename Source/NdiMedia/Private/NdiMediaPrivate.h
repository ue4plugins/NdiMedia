// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#define NDIMEDIA_DLL_PLATFORM (PLATFORM_LINUX || PLATFORM_MAC || PLATFORM_WINDOWS)


#include "NdiMediaAllowPlatformTypes.h"
	#include "Processing.NDI.Lib.h"
#include "NdiMediaHidePlatformTypes.h"

#include "Runtime/Core/Public/CoreMinimal.h"

#include "../../NdiMediaFactory/Public/NdiMediaSettings.h"


DECLARE_LOG_CATEGORY_EXTERN(LogNdiMedia, Log, All);
