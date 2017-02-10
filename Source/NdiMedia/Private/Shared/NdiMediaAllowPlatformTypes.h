// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#ifndef NDI_PLATFORM_TYPES_GUARD
	#define NDI_PLATFORM_TYPES_GUARD
#else
	#error Nesting NdiAllowPlatformTypes.h is not allowed!
#endif

#ifndef PLATFORM_WINDOWS
	#include "Processing.NDI.compat.h"
#endif

#define DWORD ::DWORD
#define FLOAT ::FLOAT

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif
