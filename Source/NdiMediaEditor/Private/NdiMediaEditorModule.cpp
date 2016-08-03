// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaEditorPCH.h"
#include "ModuleInterface.h"


/**
 * Implements the NdiMediaEditor module.
 */
class FNdiMediaEditorModule
	: public IModuleInterface
{
public:

	//~ IModuleInterface interface

	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }
};


IMPLEMENT_MODULE(FNdiMediaEditorModule, NdiMediaEditor);
