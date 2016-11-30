// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IMediaControls.h"
#include "IMediaPlayer.h"
#include "IMediaOutput.h"
#include "IMediaTracks.h"


class FNdiMediaAudioSampler;
struct NDIlib_audio_frame_t;
struct NDIlib_video_frame_t;


/**
 * Implements a media player using the Windows Media Foundation framework.
 */
class FNdiMediaPlayer
	: public IMediaControls
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
	virtual FName GetName() const override;
	virtual IMediaOutput& GetOutput() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual bool Open(const FString& Url, const IMediaOptions& Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions& Options) override;
	virtual void TickPlayer(float DeltaTime) override;
	virtual void TickVideo(float DeltaTime) override;

	DECLARE_DERIVED_EVENT(FMfMediaPlayer, IMediaPlayer::FOnMediaEvent, FOnMediaEvent);
	virtual FOnMediaEvent& OnMediaEvent() override
	{
		return MediaEvent;
	}

public:

	//~ IMediaOutput interface

	virtual void SetAudioSink(IMediaAudioSink* Sink) override;
	virtual void SetMetadataSink(IMediaBinarySink* Sink) override;
	virtual void SetOverlaySink(IMediaOverlaySink* Sink) override;
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

	/** Capture the latest metdata frame and forward it to the sink. */
	void CaptureMetadataFrame();

	/** Capture the latest video frame data and forward it to the sink. */
	void CaptureVideoFrame();

	/**
	 * Process a received audio frame.
	 *
	 * @param AudioFrame The audio frame to process.
	 * @see ProcessVideoFrame
	 */
	void ProcessAudioFrame(const NDIlib_audio_frame_t& AudioFrame);

	/**
	 * Process a received audio frame.
	 *
	 * @param AudioFrame The audio frame to process.
	 * @see ProcessVideoFrame
	 */
	void ProcessVideoFrame(const NDIlib_video_frame_t& VideoFrame);

	/**
	 * Send the given metadata to the connection.
	 *
	 * @param Metadata The metadata to send.
	 * @param Timecode Optional timecode (default = 0).
	 */
	void SendMetadata(const FString& Metadata, int64 Timecode = 0);

	/** Update the audio sampler's receiver instance. */
	void UpdateAudioSampler();

private:

	/** Callback for new samples from the audio sampler thread. */
	void HandleAudioSamplerSample(const NDIlib_audio_frame_t& AudioFrame);

private:

	/** The currently used audio sink. */
	IMediaAudioSink* AudioSink;

	/** The currently used metadata sink. */
	IMediaBinarySink* MetadataSink;

	/** The currently used video sink. */
	IMediaTextureSink* VideoSink;

private:

	/** Index of the selected audio track. */
	int32 SelectedAudioTrack;

	/** Index of the selected metadata track. */
	int32 SelectedMetadataTrack;

	/** Index of the selected video track. */
	int32 SelectedVideoTrack;

private:

	/** The audio sampler thread. */
	FNdiMediaAudioSampler* AudioSampler;

	/** Critical section for synchronizing access to receiver and sinks. */
	FCriticalSection CriticalSection;

	/** Current state of the media player. */
	EMediaState CurrentState;

	/** The currently opened URL. */
	FString CurrentUrl;

	/** Number of audio channels in the last received sample. */
	int32 LastAudioChannels;

	/** Audio sample rate in the last received sample. */
	int32 LastAudioSampleRate;

	/** Buffer dimensions in the last received sample. */
	FIntPoint LastBufferDim;

	/** Video dimensions in the last received sample. */
	FIntPoint LastVideoDim;

	/** Video frame rate in the last received sample. */
	float LastVideoFrameRate;

	/** Event delegate that is invoked when a media event occurred. */
	FOnMediaEvent MediaEvent;

	/** Whether the player is paused. */
	bool Paused;

	/** The current receiver instance. */
	void* ReceiverInstance;
};
