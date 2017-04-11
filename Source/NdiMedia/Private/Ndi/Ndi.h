// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

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
