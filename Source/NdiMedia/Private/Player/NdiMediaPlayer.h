// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaControls.h"
#include "IMediaPlayer.h"
#include "IMediaOutput.h"
#include "IMediaTracks.h"


/**
 * Implements a media player using the Windows Media Foundation framework.
 */
class FNdiMediaPlayer
	: public FTickerObjectBase
	, public IMediaControls
	, public IMediaPlayer
	, public IMediaOutput
	, public IMediaTracks
{
public:

	/** Default constructor. */
	FNdiMediaPlayer();

	/** Virtual destructor. */
	virtual ~FNdiMediaPlayer();

public:

	//~ FTickerObjectBase interface

	virtual bool Tick(float DeltaTime) override;

public:

	//~ IMediaControls interface

	virtual FTimespan GetDuration() const override;
	virtual float GetRate() const override;
	virtual EMediaState GetState() const override;
	virtual TRange<float> GetSupportedRates(EMediaPlaybackDirections Direction, bool Unthinned) const override;
	virtual FTimespan GetTime() const override;
	virtual bool IsLooping() const override;
	virtual bool Seek(const FTimespan& Time) override;
	virtual bool SetLooping(bool Looping) override;
	virtual bool SetRate(float Rate) override;
	virtual bool SupportsRate(float Rate, bool Unthinned) const override;
	virtual bool SupportsScrubbing() const override;
	virtual bool SupportsSeeking() const override;

public:

	//~ IMediaPlayer interface

	virtual void Close() override;
	virtual IMediaControls& GetControls() override;
	virtual FString GetInfo() const override;
	virtual IMediaOutput& GetOutput() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual bool Open(const FString& Url, const IMediaOptions& Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions& Options) override;

	DECLARE_DERIVED_EVENT(FMfMediaPlayer, IMediaPlayer::FOnMediaEvent, FOnMediaEvent);
	virtual FOnMediaEvent& OnMediaEvent() override
	{
		return MediaEvent;
	}

public:

	//~ IMediaOutput interface

	virtual void SetAudioSink(IMediaAudioSink* Sink) override;
	virtual void SetCaptionSink(IMediaStringSink* Sink) override;
	virtual void SetImageSink(IMediaTextureSink* Sink) override;
	virtual void SetVideoSink(IMediaTextureSink* Sink) override;

public:

	//~ IMediaTracks interface

	virtual uint32 GetAudioTrackChannels(int32 TrackIndex) const override;
	virtual uint32 GetAudioTrackSampleRate(int32 TrackIndex) const override;
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override;
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual uint32 GetVideoTrackBitRate(int32 TrackIndex) const override;
	virtual FIntPoint GetVideoTrackDimensions(int32 TrackIndex) const override;
	virtual float GetVideoTrackFrameRate(int32 TrackIndex) const override;
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;

protected:

	/** Capture the latest audio frame data and forward it to the sink. */
	void CaptureAudioFrame();

	/** Capture the latest video frame data and forward it to the sink. */
	void CaptureVideoFrame();

private:

	/** The currently used audio sink. */
	IMediaAudioSink* AudioSink;

	/** The currently used video sink. */
	IMediaTextureSink* VideoSink;

private:

	/** Whether the current source is connected. */
	bool Connected;

	/** Critical section for synchronizing access to receiver and sinks. */
	FCriticalSection CriticalSection;

	/** The currently opened URL. */
	FString CurrentUrl;

	int32 LastAudioChannels;
	int32 LastAudioSampleRate;
	FIntPoint LastVideoDimensions;
	float LastVideoFrameRate;

	/** Event delegate that is invoked when a media event occurred. */
	FOnMediaEvent MediaEvent;

	/** The current receiver instance. */
	void* Receiver;
};
