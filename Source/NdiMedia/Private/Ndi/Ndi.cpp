// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "Ndi.h"
#include "NdiMediaPrivate.h"
#include "IPluginManager.h"


/* Static initialization
 *****************************************************************************/

void* FNdi::LibHandle = nullptr;


/* FVlc static functions
 *****************************************************************************/

bool FNdi::Initialize()
{
#if NDIMEDIA_DLL_PLATFORM

	// determine directory paths
	const FString BaseDir = IPluginManager::Get().FindPlugin("NdiMedia")->GetBaseDir();
	const FString LibDir = FPaths::Combine(*BaseDir, TEXT("ThirdParty"), TEXT("lib"));

#if PLATFORM_LINUX
	const FString Lib = FPaths::Combine(*LibDir, TEXT("linux"), TEXT("x86_64-linux-gnu-5.4"), TEXT("libndi.so.1.0.1"));
#elif PLATFORM_MAC
	const FString Lib = FPaths::Combine(*LibDir, TEXT("apple"), TEXT("x64"), TEXT("libndi.dylib"));
#elif PLATFORM_WINDOWS
	#if PLATFORM_64BITS
		const FString Lib = FPaths::Combine(*LibDir, TEXT("windows"), TEXT("x64"), TEXT("Processing.NDI.Lib.x64.dll"));
	#else
		const FString Lib = FPaths::Combine(*LibDir, TEXT("windows"), TEXT("x86"), TEXT("Processing.NDI.Lib.x86.dll"));
	#endif
#endif

	LibHandle = FPlatformProcess::GetDllHandle(*Lib);

	if (LibHandle == nullptr)
	{
		UE_LOG(LogNdiMedia, Warning, TEXT("Failed to load required library %s. Plug-in will not be functional."), *Lib);
		return false;
	}

#endif //NDIMEDIA_DLL_PLATFORM

	if (!NDIlib_is_supported_CPU())
	{
		UE_LOG(LogNdiMedia, Error, TEXT("Cannot initialize NDI: CPU is not supported"));
		return false;
	}

	if (!NDIlib_initialize())
	{
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
		NDIlib_destroy();

		FPlatformProcess::FreeDllHandle(LibHandle);
		LibHandle = nullptr;
	}
}
