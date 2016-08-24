// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#include "Runtime/Core/Public/Core.h"
#include "Runtime/Core/Public/Containers/Ticker.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Runtime/CoreUObject/Public/CoreUObject.h"
#include "Runtime/Media/Public/IMediaAudioSink.h"
#include "Runtime/Media/Public/IMediaOptions.h"
#include "Runtime/Media/Public/IMediaStringSink.h"
#include "Runtime/Media/Public/IMediaTextureSink.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Runtime/RenderCore/Public/RenderingThread.h"

#include "../../NdiMediaFactory/Public/NdiMediaSettings.h"

#if PLATFORM_LINUX || PLATFORM_MAC
	#include "Processing.NDI.Lib.h"
#elif PLATFORM_WINDOWS
	#include "AllowWindowsPlatformTypes.h"
	#include "Processing.NDI.Lib.h"
	#include "HideWindowsPlatformTypes.h"
#else
	#error "NdiMedia does not support this platform"
#endif


DECLARE_LOG_CATEGORY_EXTERN(LogNdiMedia, Log, All);
