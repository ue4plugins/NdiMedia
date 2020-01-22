// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Processing.NDI.Lib.h"

class FNdi
{
public:

	static const NDIlib_v3* Lib;

	static bool Initialize();
	static bool IsInitialized();
	static void Shutdown();

private:

	static void* LibHandle;
};
