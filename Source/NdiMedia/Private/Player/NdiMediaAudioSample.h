// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaAudioSample.h"


/**
 * Implements a media audio sample for NdiMedia.
 */
class FNdiMediaAudioSample
	: public IMediaAudioSample
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InReceiverInstance The receiver instance that generated the sample.
	 * @param InTimecode The time code for which the sample was generated.
	 * @param InVideoFrame The video frame data.
	 * @param InSampleFormat The sample format.
	 * @param InFrameTimecodeValid Whether the time code embedded in the frame is valid.
	 */
	FNdiMediaAudioSample(void* InReceiverInstance, FTimespan InTimecode, const NDIlib_audio_frame_t& InAudioFrame, bool InFrameTimecodeValid)
		: Frame(InAudioFrame)
		, FrameInterleaved({ 0 })
		, FrameTimecodeValid(InFrameTimecodeValid)
		, ReceiverInstance(InReceiverInstance)
		, Timecode(InTimecode)
	{ }

	/** Virtual destructor. */
	virtual ~FNdiMediaAudioSample()
	{
		NDIlib_recv_free_audio(ReceiverInstance, &Frame);

		if (FrameInterleaved.p_data != nullptr)
		{
			delete[] FrameInterleaved.p_data;
		}
	}

public:

	//~ IMediaAudioSample interface

	virtual const void* GetBuffer() const override
	{
		if (FrameInterleaved.p_data == nullptr)
		{
			const int32 TotalSamples = Frame.no_samples * Frame.no_channels;

			FrameInterleaved.reference_level = 20;
			FrameInterleaved.p_data = new short[TotalSamples];

			NDIlib_util_audio_to_interleaved_16s(&Frame, &FrameInterleaved);
		}

		return FrameInterleaved.p_data;
	}

	virtual uint32 GetChannels() const override
	{
		return Frame.no_channels;
	}

	virtual FTimespan GetDuration() const override
	{
		return FTimespan(ETimespan::TicksPerSecond * Frame.no_samples / Frame.sample_rate);
	}

	virtual EMediaAudioSampleFormat GetFormat() const override
	{
		return EMediaAudioSampleFormat::Int16;
	}

	virtual uint32 GetFrames() const override
	{
		return Frame.no_samples;
	}

	virtual uint32 GetSampleRate() const override
	{
		return Frame.sample_rate;
	}

	virtual FTimespan GetTime() const override
	{
		return FTimespan(Frame.timecode);
	}

	virtual FTimespan GetTimecode(bool Locked) const
	{
		if (Locked && FrameTimecodeValid)
		{
			return GetTime();
		}

		return Timecode;
	}

private:

	/** The audio frame data. */
	NDIlib_audio_frame_t Frame;

	/** The interleaved audio frame data (will be populated on demand). */
	mutable NDIlib_audio_frame_interleaved_16s_t FrameInterleaved;

	/** Whether the time code embedded in the frame is valid. */
	bool FrameTimecodeValid;

	/** The receiver instance that generated this sample. */
	void* ReceiverInstance;

	/** The time code for which the sample is valid. */
	FTimespan Timecode;
};
