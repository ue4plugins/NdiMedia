// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once


class FNdi
{
public:

	static bool Initialize();
	static bool IsInitialized();
	static void Shutdown();

private:

	static void* LibHandle;
};
