// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaBinarySample.h"


/**
 * Implements a media binary data sample for NdiMedia.
 */
class FNdiMediaBinarySample
	: public IMediaBinarySample
{
public:

	/** Default constructor. */
	FNdiMediaBinarySample()
		: Frame({ 0 })
		, ReceiverInstance(nullptr)
		, Time(FTimespan::Zero())
	{ }

	/** Virtual destructor. */
	virtual ~FNdiMediaBinarySample()
	{
		FreeFrame();
	}

public:

	/**
	 * Initialize the sample.
	 *
	 * @param InReceiverInstance The receiver instance that generated the sample.
	 * @param InFrame The metadata frame data.
	 * @param InTime The sample time (in the player's own clock).
	 */
	bool Initialize(void* InReceiverInstance, const NDIlib_metadata_frame_t& InFrame, FTimespan InTime)
	{
		if ((InReceiverInstance == nullptr) || (InFrame.p_data == nullptr))
		{
			return false;
		}

		Frame = InFrame;
		ReceiverInstance = InReceiverInstance;
		Time = InTime;

		return true;
	}

public:

	//~ IMediaBinarySample interface

	virtual const void* GetData() override
	{
		return Frame.p_data;
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
		return Time;
	}

protected:

	/** Free the metadata frame data. */
	void FreeFrame()
	{
		if (ReceiverInstance != nullptr)
		{
			NDIlib_recv_free_metadata(ReceiverInstance, &Frame);

			ReceiverInstance = nullptr;
			Frame = { 0 };
		}
	}

private:

	/** The metadata frame data. */
	NDIlib_metadata_frame_t Frame;

	/** The receiver instance that generated this sample. */
	void* ReceiverInstance;

	/** Sample time. */
	FTimespan Time;
};
