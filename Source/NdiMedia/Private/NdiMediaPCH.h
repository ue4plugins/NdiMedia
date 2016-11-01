// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#define NDIMEDIA_DLL_PLATFORM (PLATFORM_LINUX || PLATFORM_MAC || PLATFORM_WINDOWS)


#include "NdiMediaAllowPlatformTypes.h"
	#include "Processing.NDI.Lib.h"
#include "NdiMediaHidePlatformTypes.h"

#include "Runtime/Core/Public/Core.h"
#include "Runtime/Core/Public/Containers/Ticker.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Runtime/CoreUObject/Public/CoreUObject.h"
#include "Runtime/Media/Public/IMediaAudioSink.h"
#include "Runtime/Media/Public/IMediaOptions.h"
#include "Runtime/Media/Public/IMediaOverlaySink.h"
#include "Runtime/Media/Public/IMediaTextureSink.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Runtime/RenderCore/Public/RenderingThread.h"

#include "../../NdiMediaFactory/Public/NdiMediaSettings.h"


DECLARE_LOG_CATEGORY_EXTERN(LogNdiMedia, Log, All);
