// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "Containers/UnrealString.h"
#include "HAL/CriticalSection.h"
#include "IMediaCache.h"
#include "IMediaControls.h"
#include "IMediaPlayer.h"
#include "IMediaTracks.h"
#include "IMediaView.h"
#include "Math/IntPoint.h"
#include "Misc/Timespan.h"

class FMediaSamples;
class FNdiMediaAudioSamplePool;
class FNdiMediaBinarySamplePool;
class FNdiMediaTextureSamplePool;
class IMediaEventSink;

enum class EMediaTextureSampleFormat;

struct NDIlib_audio_frame_v2_t;
struct NDIlib_video_frame_v2_t;


/**
 * Implements a media player using Newtek's Network Device Interface (NDI).
 *
 * Audio samples are fetched on the high-frequency media ticker thread (TickTickable)
 * to prevent NDI audio queue overruns and UE4 sound buffer underruns.
 *
 * The processing of metadata and video frames is delayed until the fetch stage
 * (TickFetch) in order to increase the window of opportunity for receiving NDI
 * frames for the current render frame time code.
 *
 * Depending on whether the media source enables time code synchronization,
 * the player's current play time (CurrentTime) is derived either from the
 * time codes embedded in NDI frames or from the Engine's global time code.
 */
class FNdiMediaPlayer
	: public IMediaPlayer
	, protected IMediaCache
	, protected IMediaControls
	, protected IMediaTracks
	, protected IMediaView
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InEventSink The object that receives media events from this player.
	 */
	FNdiMediaPlayer(IMediaEventSink& InEventSink);

	/** Virtual destructor. */
	virtual ~FNdiMediaPlayer();

public:

	//~ IMediaPlayer interface

	virtual void Close() override;
	virtual IMediaCache& GetCache() override;
	virtual IMediaControls& GetControls() override;
	virtual FString GetInfo() const override;
	virtual FName GetPlayerName() const override;
	virtual IMediaSamples& GetSamples() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual IMediaView& GetView() override;
	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* Options) override;
	virtual void TickAudio() override;
	virtual void TickFetch(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;

protected:

	//~ IMediaControls interface

	virtual bool CanControl(EMediaControl Control) const override;
	virtual FTimespan GetDuration() const override;
	virtual float GetRate() const override;
	virtual EMediaState GetState() const override;
	virtual EMediaStatus GetStatus() const override;
	virtual TRangeSet<float> GetSupportedRates(EMediaRateThinning Thinning) const override;
	virtual FTimespan GetTime() const override;
	virtual bool IsLooping() const override;
	virtual bool Seek(const FTimespan& Time) override;
	virtual bool SetLooping(bool Looping) override;
	virtual bool SetRate(float Rate) override;

protected:

	//~ IMediaTracks interface

	virtual bool GetAudioTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaAudioTrackFormat& OutFormat) const override;
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override;
	virtual int32 GetNumTrackFormats(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual int32 GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual bool GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaVideoTrackFormat& OutFormat) const override;
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;
	virtual bool SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex) override;

protected:

	/**
	 * Process pending audio frames, and forward them to the audio sink.
	 *
	 * @see ProcessMetadataAndVideo
	 */
	void ProcessAudio();

	/**
	 * Process pending metadata and video frames, and forward them to the sinks.
	 *
	 * @see ProcessAudio
	 */
	void ProcessMetadataAndVideo();

	/**
	 * Send the given metadata to the connection.
	 *
	 * @param Metadata The metadata to send.
	 * @param Timecode Optional timecode (default = 0).
	 */
	void SendMetadata(const FString& Metadata, int64 Timecode = 0);

private:

	/** Audio sample object pool. */
	FNdiMediaAudioSamplePool* AudioSamplePool;

	/** Critical section for synchronizing access to receiver and sinks. */
	FCriticalSection CriticalSection;

	/** Current state of the media player. */
	EMediaState CurrentState;

	/** Current playback time. */
	FTimespan CurrentTime;

	/** The currently opened URL. */
	FString CurrentUrl;

	/** The media event handler. */
	IMediaEventSink& EventSink;

	/** Number of audio channels in the last received sample. */
	int32 LastAudioChannels;

	/** Audio sample rate in the last received sample. */
	int32 LastAudioSampleRate;

	/** Video bit rate based on the last received sample. */
	uint64 LastVideoBitRate;

	/** Video dimensions in the last received sample. */
	FIntPoint LastVideoDim;

	/** Video frame rate in the last received sample. */
	float LastVideoFrameRate;

	/** Whether the player is paused. */
	bool Paused;

	/** Reference level for received audio (cached from settings). */
	int32 ReceiveAudioReferenceLevel;

	/** The current receiver instance. */
	void* ReceiverInstance;

	/** The media sample cache. */
	FMediaSamples* Samples;

	/** Index of the selected audio track. */
	int32 SelectedAudioTrack;

	/** Index of the selected metadata track. */
	int32 SelectedMetadataTrack;

	/** Index of the selected video track. */
	int32 SelectedVideoTrack;

	/** Whether to use the time code embedded in NDI frames. */
	bool UseFrameTimecode;

	/** The current video sample format. */
	EMediaTextureSampleFormat VideoSampleFormat;
};
