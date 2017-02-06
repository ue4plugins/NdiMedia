#pragma once


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
