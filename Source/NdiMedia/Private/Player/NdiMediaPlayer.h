// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IMediaControls.h"
#include "IMediaPlayer.h"
#include "IMediaOutput.h"
#include "IMediaTickable.h"
#include "IMediaTracks.h"
#include "INdiMediaAudioTickable.h"

class FNdiMediaAudioSampler;
class FRunnableThread;

enum class EMediaTextureSampleFormat;

struct NDIlib_audio_frame_t;
struct NDIlib_video_frame_t;


/**
 * Implements a media player using the Windows Media Foundation framework.
 */
class FNdiMediaPlayer
	: public IMediaControls
	, public IMediaPlayer
	, public IMediaOutput
	, public IMediaTickable
	, public IMediaTracks
	, public INdiMediaAudioTickable
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
	virtual TRangeSet<float> GetSupportedRates(EMediaRateThinning Thinning) const override;
	virtual FTimespan GetTime() const override;
	virtual bool IsLooping() const override;
	virtual bool Seek(const FTimespan& Time) override;
	virtual bool SetLooping(bool Looping) override;
	virtual bool SetRate(float Rate) override;
	virtual bool SupportsFeature(EMediaFeature Feature) const override;
	virtual bool SupportsRate(float Rate, EMediaRateThinning Thinning) const override;

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

	DECLARE_DERIVED_EVENT(FMfMediaPlayer, IMediaPlayer::FOnMediaEvent, FOnMediaEvent);
	virtual FOnMediaEvent& OnMediaEvent() override
	{
		return MediaEvent;
	}

public:

	//~ IMediaOutput interface

	virtual bool SetAudioNative(bool Enabled) override;
	virtual void SetAudioNativeVolume(float Volume) override;
	virtual void SetAudioSink(TSharedPtr<IMediaAudioSink, ESPMode::ThreadSafe> Sink) override;
	virtual void SetMetadataSink(TSharedPtr<IMediaBinarySink, ESPMode::ThreadSafe> Sink) override;
	virtual void SetOverlaySink(TSharedPtr<IMediaOverlaySink, ESPMode::ThreadSafe> Sink) override;
	virtual void SetVideoSink(TSharedPtr<IMediaTextureSink, ESPMode::ThreadSafe> Sink) override;

public:

	//~ IMediaTickable interface

	virtual void TickInput(FTimespan Timecode, FTimespan DeltaTime, bool Locked) override;

public:

	//~ IMediaTracks interface

	virtual uint32 GetAudioTrackChannels(int32 TrackIndex) const override;
	virtual uint32 GetAudioTrackSampleRate(int32 TrackIndex) const override;
	virtual bool GetCacheState(EMediaTrackType TrackType, EMediaCacheState State, TRangeSet<FTimespan>& OutCachedTimes) const override;
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override;
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual uint64 GetVideoTrackBitRate(int32 TrackIndex) const override;
	virtual FIntPoint GetVideoTrackDimensions(int32 TrackIndex) const override;
	virtual float GetVideoTrackFrameRate(int32 TrackIndex) const override;
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;

public:

	//~ INdiMediaAudioTickable interface

	virtual void TickAudio(FTimespan Timecode) override;

protected:

	/**
	 * Flush the output sinks.
	 *
	 * @param Shutdown Whether the sinks should be shut down.
	 */
	void FlushSinks(bool Shutdown);

	/**
	 * Process pending audio frames, and forward them to the audio sink.
	 *
	 * @param Timecode The current media time code.
	 * @see ProcessMetadataAndVideo
	 */
	void ProcessAudio(FTimespan Timecode);

	/**
	 * Process pending metadata and video frames, and forward them to the sinks.
	 *
	 * @param Timecode The current media time code.
	 * @see ProcessAudio
	 */
	void ProcessMetadataAndVideo(FTimespan Timecode);

	/**
	 * Send the given metadata to the connection.
	 *
	 * @param Metadata The metadata to send.
	 * @param Timecode Optional timecode (default = 0).
	 */
	void SendMetadata(const FString& Metadata, int64 Timecode = 0);

private:

	/** The currently used audio sink. */
	TWeakPtr<IMediaAudioSink, ESPMode::ThreadSafe> AudioSinkPtr;

	/** The currently used metadata sink. */
	TWeakPtr<IMediaBinarySink, ESPMode::ThreadSafe> MetadataSinkPtr;

	/** The currently used video sink. */
	TWeakPtr<IMediaTextureSink, ESPMode::ThreadSafe> VideoSinkPtr;

private:

	/** Index of the selected audio track. */
	int32 SelectedAudioTrack;

	/** Index of the selected metadata track. */
	int32 SelectedMetadataTrack;

	/** Index of the selected video track. */
	int32 SelectedVideoTrack;

private:

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

	/** The last time code received from any sample. */
	FTimespan LastTimecode;

	/** Video bit rate based on the last received sample. */
	uint64 LastVideoBitRate;

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

	/** Whether to use the time code embedded in the NDI stream. */
	bool UseTimecode;

	/** The current video sample format. */
	EMediaTextureSampleFormat VideoSampleFormat;
};
