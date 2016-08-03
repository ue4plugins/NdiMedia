// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#if WITH_EDITOR
	#include "Developer/Settings/Public/ISettingsModule.h"
	#include "Developer/Settings/Public/ISettingsSection.h"
#endif

#include "Runtime/Core/Public/Core.h"
#include "Runtime/Core/Public/Containers/Ticker.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Runtime/CoreUObject/Public/CoreUObject.h"
#include "Runtime/Media/Public/IMediaAudioSink.h"
#include "Runtime/Media/Public/IMediaOptions.h"
#include "Runtime/Media/Public/IMediaStringSink.h"
#include "Runtime/Media/Public/IMediaTextureSink.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Runtime/RenderCore/Public/RenderingThread.h"

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
