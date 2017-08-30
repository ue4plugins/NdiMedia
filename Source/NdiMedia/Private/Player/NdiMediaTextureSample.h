// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaTextureSample.h"


/**
 * Implements a media texture sample for NdiMedia.
 */
class FNdiMediaTextureSample
	: public IMediaTextureSample
{
public:

	/** Default constructor. */
	FNdiMediaTextureSample()
		: Duration(FTimespan::Zero())
		, Frame({ 0 })
		, ReceiverInstance(nullptr)
		, SampleFormat(EMediaTextureSampleFormat::Undefined)
		, Time(FTimespan::Zero())
	{ }

	/** Virtual destructor. */
	virtual ~FNdiMediaTextureSample()
	{
		FreeFrame();
	}

public:

	/**
	 * Initialize the sample.
	 *
	 * @param InReceiverInstance The receiver instance that generated the sample.
	 * @param InFrame The video frame data.
	 * @param InSampleFormat The sample format.
	 * @param InTime The sample time (in the player's own clock).
	 */
	bool Initialize(void* InReceiverInstance, const NDIlib_video_frame_v2_t& InFrame, EMediaTextureSampleFormat InSampleFormat, FTimespan InTime)
	{
		if ((InReceiverInstance == nullptr) || (InSampleFormat == EMediaTextureSampleFormat::Undefined))
		{
			return false;
		}

		if ((InFrame.p_data == nullptr) || (InFrame.frame_rate_D == 0) || (InFrame.frame_rate_N == 0))
		{
			return false;
		}

		Duration = FTimespan::FromMicroseconds((InFrame.frame_rate_D * 1000000) / InFrame.frame_rate_N);
		Frame = InFrame;
		ReceiverInstance = InReceiverInstance;
		SampleFormat = InSampleFormat;
		Time = InTime;

		return true;
	}

public:

	//~ IMediaTextureSample interface

	virtual const void* GetBuffer() override
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
		return Time;
	}

	virtual bool IsCacheable() const override
	{
		return true;
	}

	virtual bool IsOutputSrgb() const override
	{
		return true;
	}

protected:

	/** Free the video frame data. */
	void FreeFrame()
	{
		if (ReceiverInstance != nullptr)
		{
			NDIlib_recv_free_video_v2(ReceiverInstance, &Frame);

			ReceiverInstance = nullptr;
			Frame = { 0 };
		}
	}

private:

	/** Duration for which the sample is valid. */
	FTimespan Duration;

	/** The video frame data. */
	NDIlib_video_frame_v2_t Frame;

	/** The receiver instance that generated this sample. */
	void* ReceiverInstance;

	/** Sample format. */
	EMediaTextureSampleFormat SampleFormat;

	/** Sample time. */
	FTimespan Time;
};
