// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleInterface.h"


class IMediaPlayer;


/**
 * Interface for the NdiMedia module.
 */
class INdiMediaModule
	: public IModuleInterface
{
public:

	/**
	 * Creates an NDI based media player.
	 *
	 * @return A new media player, or nullptr if a player couldn't be created.
	 */
	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer() = 0;

public:

	/** Virtual destructor. */
	virtual ~INdiMediaModule() { }
};
