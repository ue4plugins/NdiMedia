// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Ndi.h"
#include "NdiMediaPrivate.h"

#include "Containers/StringConv.h"
#include "IPluginManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"


/* Static initialization
 *****************************************************************************/

const NDIlib_v3* FNdi::Lib = nullptr;
void* FNdi::LibHandle = nullptr;


/* FVlc static functions
 *****************************************************************************/

bool FNdi::Initialize()
{
#if NDIMEDIA_DLL_PLATFORM
	// determine runtime library path
	TCHAR RedistDir[4096];
	FPlatformMisc::GetEnvironmentVariable(ANSI_TO_TCHAR(NDILIB_REDIST_FOLDER), RedistDir, ARRAY_COUNT(RedistDir));
	const FString LibPath = FPaths::Combine(RedistDir, ANSI_TO_TCHAR(NDILIB_LIBRARY_NAME));

	if (!FPaths::FileExists(LibPath))
	{
		UE_LOG(LogNdiMedia, Warning, TEXT("Failed to find NDI runtime library %s: Please install the NDI Redist from %s."), *LibPath, ANSI_TO_TCHAR(NDILIB_REDIST_URL));
		return false;
	}

	// load runtime library
	LibHandle = FPlatformProcess::GetDllHandle(*LibPath);

	if (LibHandle == nullptr)
	{
		UE_LOG(LogNdiMedia, Warning, TEXT("Failed to load NDI runtime library %s: Please reinstall the NDI Redist from %s."), *LibPath, ANSI_TO_TCHAR(NDILIB_REDIST_URL));
		return false;
	}
#endif //NDIMEDIA_DLL_PLATFORM

	typedef const NDIlib_v3* (*NDIlib_v3_load_fn)();
	auto NDIlib_v3_load = (NDIlib_v3_load_fn)FPlatformProcess::GetDllExport(LibHandle, TEXT("NDIlib_v3_load"));

	if (NDIlib_v3_load == nullptr)
	{
		UE_LOG(LogNdiMedia, Error, TEXT("Failed to initialize NDI: The main DLL entry point could not be found"));
		return false;
	}

	Lib = NDIlib_v3_load();

	if (!Lib->NDIlib_is_supported_CPU())
	{
		UE_LOG(LogNdiMedia, Error, TEXT("Failed to initialize NDI: Your CPU is not supported"));
		return false;
	}

	if (!Lib->NDIlib_initialize())
	{
		UE_LOG(LogNdiMedia, Error, TEXT("Failed to initialize NDI: Unknown error"));
		return false;
	}

	return true;
}


bool FNdi::IsInitialized()
{
	return (LibHandle != nullptr);
}


void FNdi::Shutdown()
{
	if (LibHandle != nullptr)
	{
		Lib->NDIlib_destroy();
		FPlatformProcess::FreeDllHandle(LibHandle);

		LibHandle = nullptr;
		Lib = nullptr;
	}
}
