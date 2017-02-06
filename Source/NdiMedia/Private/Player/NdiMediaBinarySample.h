// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaBinarySample.h"


/**
 * Implements a media binary data sample for NdiMedia.
 */
class FNdiMediaBinarySample
	: public IMediaBinarySample
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InReceiverInstance The receiver instance that generated the sample.
	 * @param InTimecode The time code for which the sample was generated.
	 * @param InFrame The metadata frame data.
	 * @param InFrameTimecodeValid Whether the time code embedded in the frame is valid.
	 */
	FNdiMediaBinarySample(void* InReceiverInstance, FTimespan InTimecode, const NDIlib_metadata_frame_t& InFrame, bool InFrameTimecodeValid)
		: Frame(InFrame)
		, FrameTimecodeValid(InFrameTimecodeValid)
		, ReceiverInstance(InReceiverInstance)
		, Timecode(InTimecode)
	{ }

	/** Virtual destructor. */
	virtual ~FNdiMediaBinarySample()
	{
		NDIlib_recv_free_metadata(ReceiverInstance, &Frame);
	}

public:

	//~ IMediaBinarySample interface

	virtual const uint8& GetData() const override
	{
		return *((uint8*)Frame.p_data);
	}

	virtual FTimespan GetDuration() const override
	{
		return FTimespan::Zero();
	}

	virtual uint32 GetSize() const override
	{
		return Frame.length;
	}

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

public:

	/** Metadata frame data. */
	NDIlib_metadata_frame_t Frame;

	/** Whether the time code embedded in the frame is valid. */
	bool FrameTimecodeValid;

	/** The receiver instance that generated this sample. */
	void* ReceiverInstance;

	/** The time code for which the sample is valid. */
	FTimespan Timecode;
};
