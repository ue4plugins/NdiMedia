// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaAudioSample.h"
#include "MediaObjectPool.h"


/**
 * Implements a media audio sample for NdiMedia.
 */
class FNdiMediaAudioSample
	: public IMediaAudioSample
	, public IMediaPoolable
{
public:

	/** Default constructor. */
	FNdiMediaAudioSample()
		: Duration(FTimespan::Zero())
		, Frame({ 0 })
		, FrameInterleaved({ 0 })
		, FrameInterleavedSize(0)
		, ReceiverInstance(nullptr)
		, Time(FTimespan::Zero())
	{ }

	/** Virtual destructor. */
	virtual ~FNdiMediaAudioSample()
	{
		FreeFrame();
		FreeFrameInterleaved();
	}

public:

	/**
	 * Initialize the sample.
	 *
	 * @param InReceiverInstance The receiver instance that generated the sample.
	 * @param InFrame The audio frame data.
	 * @param InTime The sample time (in the player's own clock).
	 * @result true on success, false otherwise.
	 */
	bool Initialize(void* InReceiverInstance, const NDIlib_audio_frame_v2_t& InFrame, FTimespan InTime)
	{
		FreeFrame();

		if (InReceiverInstance == nullptr)
		{
			return false;
		}

		if ((InFrame.p_data == nullptr) || (InFrame.no_channels <= 0) || (InFrame.no_samples <= 0))
		{
			return false;
		}

		Duration = ETimespan::TicksPerSecond * InFrame.no_samples / InFrame.sample_rate;
		Frame = InFrame;
		ReceiverInstance = InReceiverInstance;
		Time = InTime;

		return true;
	}

public:

	//~ IMediaAudioSample interface

	virtual const void* GetBuffer() override
	{
		if (FrameInterleaved.no_samples == 0)
		{
			if (Frame.p_data == nullptr)
			{
				return nullptr;
			}

			// try to reuse existing frame buffer if large enough
			const int32 TotalSamples = Frame.no_samples * Frame.no_channels;

			if (FrameInterleavedSize < TotalSamples)
			{
				FreeFrameInterleaved();
			}

			// allocate frame buffer
			if (FrameInterleaved.p_data == nullptr)
			{
				FrameInterleaved.reference_level = 20;
				FrameInterleaved.p_data = new short[TotalSamples];
				FrameInterleavedSize = TotalSamples;
			}

			NDIlib_util_audio_to_interleaved_16s_v2(&Frame, &FrameInterleaved);
		}

		return FrameInterleaved.p_data;
	}

	virtual uint32 GetChannels() const override
	{
		return Frame.no_channels;
	}

	virtual FTimespan GetDuration() const override
	{
		return Duration;
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
		return Time;
	}

public:

	//~ IMediaPoolable interface

	virtual void ShutdownPoolable() override
	{
		FreeFrame();
	}

protected:

	/** Free the audio frame data. */
	void FreeFrame()
	{
		if (ReceiverInstance != nullptr)
		{
			NDIlib_recv_free_audio_v2(ReceiverInstance, &Frame);

			ReceiverInstance = nullptr;
			Frame = { 0 };
		}
	}

	/** Free the interleaved audio frame data. */
	void FreeFrameInterleaved()
	{
		if (FrameInterleaved.p_data != nullptr)
		{
			delete[] FrameInterleaved.p_data;
			FrameInterleaved = { 0 };
			FrameInterleavedSize = 0;
		}
	}

private:

	/** Duration for which the sample is valid. */
	FTimespan Duration;

	/** The audio frame data. */
	NDIlib_audio_frame_v2_t Frame;

	/** The interleaved audio frame data (will be populated on demand). */
	mutable NDIlib_audio_frame_interleaved_16s_t FrameInterleaved;

	/** Current size of the interleaved audio frame buffer (in number of samples). */
	int32 FrameInterleavedSize;

	/** The receiver instance that generated this sample. */
	void* ReceiverInstance;

	/** Sample time. */
	FTimespan Time;
};


/** Implements a pool for NDI audio sample objects. */
class FNdiMediaAudioSamplePool : public TMediaObjectPool<FNdiMediaAudioSample> { };
