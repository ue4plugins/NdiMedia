// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Misc/Timespan.h"


/**
 * Interface for objects that can be ticked from the audio ticker.
 */
class INdiMediaAudioTickable
{
public:

	/**
	 * Called from the audio ticker.
	 *
	 * @param Timecode The current media time code.
	 */
	virtual void TickAudio(FTimespan Timecode) = 0;
};
