// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaTextureSample.h"


/**
 * Implements a media texture sample for NdiMedia.
 */
class FNdiMediaTextureSample
	: public IMediaTextureSample
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InReceiverInstance The receiver instance that generated the sample.
	 * @param InTimecode The time code for which the sample was generated.
	 * @param InFrame The video frame data.
	 * @param InSampleFormat The sample format.
	 * @param InFrameTimecodeValid Whether the time code embedded in the frame is valid.
	 */
	FNdiMediaTextureSample(void* InReceiverInstance, FTimespan InTimecode, const NDIlib_video_frame_t& InFrame, EMediaTextureSampleFormat InSampleFormat, bool InFrameTimecodeValid)
		: Frame(InFrame)
		, FrameTimecodeValid(InFrameTimecodeValid)
		, ReceiverInstance(InReceiverInstance)
		, SampleFormat(InSampleFormat)
		, Timecode(InTimecode)
	{ }

	/** Virtual destructor. */
	virtual ~FNdiMediaTextureSample()
	{
		NDIlib_recv_free_video(ReceiverInstance, &Frame);
	}

public:

	//~ IMediaTextureSample interface

	virtual const void* GetBuffer() const override
	{
		return Frame.p_data;
	}

	virtual FIntPoint GetDim() const override
	{
		return FIntPoint(Frame.line_stride_in_bytes / 4, Frame.yres);
	}

	virtual FTimespan GetDuration() const override
	{
		return FTimespan::Zero();
	}

	virtual EMediaTextureSampleFormat GetFormat() const override
	{
		return SampleFormat;
	}

	virtual FIntPoint GetOutputDim() const override
	{
		return FIntPoint(Frame.xres, Frame.yres);
	}

	virtual uint32 GetStride() const override
	{
		return Frame.line_stride_in_bytes;
	}

#if WITH_ENGINE
	virtual FRHITexture* GetTexture() const override
	{
		return nullptr;
	}
#endif //WITH_ENGINE

	virtual FTimespan GetTime() const override
	{
		return FTimespan(Frame.timecode);
	}

	virtual FTimespan GetTimecode(bool Locked) const override
	{
		if (Locked && FrameTimecodeValid)
		{
			return GetTime();
		}

		return Timecode;
	}

	virtual bool IsOutputSrgb() const override
	{
		return true;
	}

private:

	/** Video frame data. */
	NDIlib_video_frame_t Frame;

	/** Whether the time code embedded in the frame is valid. */
	bool FrameTimecodeValid;

	/** The receiver instance that generated this sample. */
	void* ReceiverInstance;

	/** Sample format. */
	EMediaTextureSampleFormat SampleFormat;

	/** The time code for which the sample is valid. */
	FTimespan Timecode;
};
