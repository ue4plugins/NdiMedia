// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPlayer.h"
#include "NdiMediaPrivate.h"

#include "HAL/PlatformProcess.h"
#include "IMediaAudioSink.h"
#include "IMediaBinarySink.h"
#include "IMediaOptions.h"
#include "IMediaTextureSink.h"
#include "Misc/ScopeLock.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"

#include "NdiMediaAudioSample.h"
#include "NdiMediaBinarySample.h"
#include "NdiMediaSettings.h"
#include "NdiMediaSource.h"
#include "NdiMediaTextureSample.h"

#include "NdiMediaAllowPlatformTypes.h"


#define LOCTEXT_NAMESPACE "FNdiMediaPlayer"


/* FNdiVideoPlayer structors
 *****************************************************************************/

FNdiMediaPlayer::FNdiMediaPlayer()
	: SelectedAudioTrack(INDEX_NONE)
	, SelectedMetadataTrack(INDEX_NONE)
	, SelectedVideoTrack(INDEX_NONE)
	, CurrentState(EMediaState::Closed)
	, LastAudioChannels(0)
	, LastAudioSampleRate(0)
	, LastTimecode(FTimespan::Zero())
	, LastVideoBitRate(0)
	, LastVideoDim(FIntPoint::ZeroValue)
	, LastVideoFrameRate(0.0f)
	, Paused(false)
	, ReceiverInstance(nullptr)
	, UseTimecode(false)
	, VideoSampleFormat(EMediaTextureSampleFormat::CharUYVY)
{ }


FNdiMediaPlayer::~FNdiMediaPlayer()
{
	Close();
}


/* IMediaControls interface
 *****************************************************************************/

FTimespan FNdiMediaPlayer::GetDuration() const
{
	return (CurrentState == EMediaState::Playing) ? FTimespan::MaxValue() : FTimespan::Zero();
}


float FNdiMediaPlayer::GetRate() const
{
	return (CurrentState == EMediaState::Playing) ? 1.0f : 0.0f;
}


EMediaState FNdiMediaPlayer::GetState() const
{
	return CurrentState;
}


TRangeSet<float> FNdiMediaPlayer::GetSupportedRates(EMediaRateThinning /*Thinning*/) const
{
	TRangeSet<float> Result;
	Result.Add(TRange<float>(0.0f));
	Result.Add(TRange<float>(1.0f));

	return Result;
}


FTimespan FNdiMediaPlayer::GetTime() const
{
	return (CurrentState == EMediaState::Playing) ? LastTimecode : FTimespan::Zero();
}


bool FNdiMediaPlayer::IsLooping() const
{
	return false; // not supported
}


bool FNdiMediaPlayer::Seek(const FTimespan& Time)
{
	return false; // not supported
}


bool FNdiMediaPlayer::SetLooping(bool Looping)
{
	return false; // not supported
}


bool FNdiMediaPlayer::SetRate(float Rate)
{
	if (Rate == 0.0f)
	{
		FlushSinks(false);
		Paused = true;
	}
	else if (Rate == 1.0f)
	{
		Paused = false;
	}
	else
	{
		return false;
	}

	return true;
}


bool FNdiMediaPlayer::SupportsFeature(EMediaFeature Feature) const
{
	return ((Feature == EMediaFeature::AudioSink) ||
			(Feature == EMediaFeature::MetadataSink) ||
			(Feature == EMediaFeature::VideoSink));
}


bool FNdiMediaPlayer::SupportsRate(float Rate, EMediaRateThinning Thinning) const
{
	return ((Rate == 0.0f) || (Rate == 1.0f));
}


/* IMediaPlayer interface
 *****************************************************************************/

void FNdiMediaPlayer::Close()
{
	{
		FScopeLock Lock(&CriticalSection);

		if (ReceiverInstance != nullptr)
		{
			NDIlib_recv_destroy(ReceiverInstance);
			ReceiverInstance = nullptr;
		}

		LastAudioChannels = 0;
		LastAudioSampleRate = 0;
	}

	CurrentState = EMediaState::Closed;
	CurrentUrl.Empty();

	LastTimecode = FTimespan::Zero();
	LastVideoBitRate = 0;
	LastVideoDim = FIntPoint::ZeroValue;
	LastVideoFrameRate = 0.0f;

	SelectedMetadataTrack = INDEX_NONE;
	SelectedVideoTrack = INDEX_NONE;
	SelectedAudioTrack = INDEX_NONE;

	FlushSinks(true);

	MediaEvent.Broadcast(EMediaEvent::TracksChanged);
	MediaEvent.Broadcast(EMediaEvent::MediaClosed);
}


IMediaControls& FNdiMediaPlayer::GetControls()
{
	return *this;
}


FString FNdiMediaPlayer::GetInfo() const
{
	return FString(); // @todo gmp: implement NDI info
}


FName FNdiMediaPlayer::GetName() const
{
	static FName PlayerName(TEXT("NdiMedia"));
	return PlayerName;
}


IMediaOutput& FNdiMediaPlayer::GetOutput()
{
	return *this;
}


FString FNdiMediaPlayer::GetStats() const
{
	NDIlib_recv_performance_t PerfDropped, PerfTotal;
	NDIlib_recv_get_performance(ReceiverInstance, &PerfTotal, &PerfDropped);

	NDIlib_recv_queue_t Queue;
	NDIlib_recv_get_queue(ReceiverInstance, &Queue);

	FString StatsString;
	{
		StatsString += TEXT("Total Frames\n");
		StatsString += FString::Printf(TEXT("    Audio: %i\n"), PerfTotal.m_audio_frames);
		StatsString += FString::Printf(TEXT("    Video: %i\n"), PerfTotal.m_video_frames);
		StatsString += FString::Printf(TEXT("    Metadata: %i\n"), PerfTotal.m_metadata_frames);
		StatsString += TEXT("\n");

		StatsString += TEXT("Dropped Frames\n");
		StatsString += FString::Printf(TEXT("    Audio: %i\n"), PerfDropped.m_audio_frames);
		StatsString += FString::Printf(TEXT("    Video: %i\n"), PerfDropped.m_video_frames);
		StatsString += FString::Printf(TEXT("    Metadata: %i\n"), PerfDropped.m_metadata_frames);
		StatsString += TEXT("\n");

		StatsString += TEXT("Queue Depth\n");
		StatsString += FString::Printf(TEXT("    Audio: %i\n"), Queue.m_audio_frames);
		StatsString += FString::Printf(TEXT("    Video: %i\n"), Queue.m_video_frames);
		StatsString += FString::Printf(TEXT("    Metadata: %i\n"), Queue.m_metadata_frames);
		StatsString += TEXT("\n");
	}

	return StatsString;
}


IMediaTracks& FNdiMediaPlayer::GetTracks()
{
	return *this;
}


FString FNdiMediaPlayer::GetUrl() const
{
	return CurrentUrl;
}


bool FNdiMediaPlayer::Open(const FString& Url, const IMediaOptions& Options)
{
	Close();

	if (Url.IsEmpty() || !Url.StartsWith(TEXT("ndi://")))
	{
		return false;
	}

	FString SourceStr = Url.RightChop(6);

	// determine sink format
	auto ColorFormat = (NDIlib_recv_color_format_e)Options.GetMediaOption(NdiMedia::ColorFormatOption, 0LL);

	if (ColorFormat == NDIlib_recv_color_format_e_BGRX_BGRA)
	{
		VideoSampleFormat = EMediaTextureSampleFormat::CharBGRA;
	}
	else if (ColorFormat == NDIlib_recv_color_format_e_UYVY_BGRA)
	{
		VideoSampleFormat = EMediaTextureSampleFormat::CharUYVY;
	}
	else
	{
		UE_LOG(LogNdiMedia, Warning, TEXT("Unsupported ColorFormat option in media source %s. Falling back to UYVY."), *SourceStr);

		ColorFormat = NDIlib_recv_color_format_e_UYVY_BGRA;
		VideoSampleFormat = EMediaTextureSampleFormat::CharUYVY;
	}

	UseTimecode = Options.GetMediaOption(NdiMedia::UseTimecodeOption, false);

	// create receiver
	int64 Bandwidth = Options.GetMediaOption(NdiMedia::BandwidthOption, (int64)NDIlib_recv_bandwidth_highest);

	NDIlib_source_t Source;
	{
		if (SourceStr.Find(TEXT(":")) != INDEX_NONE)
		{
			Source.p_ip_address = TCHAR_TO_ANSI(*SourceStr);
			Source.p_ndi_name = nullptr;
		}
		else
		{
			if (SourceStr.StartsWith(TEXT("localhost ")))
			{
				SourceStr.ReplaceInline(TEXT("localhost"), FPlatformProcess::ComputerName());
			}

			Source.p_ip_address = nullptr;
			Source.p_ndi_name = TCHAR_TO_ANSI(*SourceStr);
		}
	}

	NDIlib_recv_create_t RcvCreateDesc;
	{
		RcvCreateDesc.source_to_connect_to = Source;
		RcvCreateDesc.color_format = ColorFormat;
		RcvCreateDesc.bandwidth = (NDIlib_recv_bandwidth_e)Bandwidth;
		RcvCreateDesc.allow_video_fields = true;
	};
	
	{
		FScopeLock Lock(&CriticalSection);

		ReceiverInstance = NDIlib_recv_create2(&RcvCreateDesc);
	}

	if (ReceiverInstance == nullptr)
	{
		UE_LOG(LogNdiMedia, Error, TEXT("Failed to open NDI media source %s: couldn't create receiver"), *SourceStr);

		return false;
	}

	// send product metadata
	auto Settings = GetDefault<UNdiMediaSettings>();

	SendMetadata(
		FString::Printf(TEXT("<ndi_product short_name=\"%s\" long_name=\"%s\" manufacturer=\"%s\" version=\"%s\" serial_number=\"%s\" session_name=\"%s\" />"),
			*Settings->ProductName,
			*Settings->ProductDescription,
			*Settings->Manufacturer,
			*Settings->GetVersionName(),
			*Settings->SerialNumber,
			*Settings->SessionName
	));

	// send format metadata
	FString AudioFormatString;
	FString VideoFormatString;

	const int64 AudioChannels = Options.GetMediaOption(NdiMedia::AudioChannelsOption, (int64)0);
	
	if (AudioChannels > 0)
	{
		AudioFormatString += FString::Printf(TEXT(" no_channels=\"%i\""), AudioChannels);
	}

	const int64 AudioSampleRate = Options.GetMediaOption(NdiMedia::AudioSampleRateOption, (int64)0);

	if (AudioSampleRate > 0)
	{
		AudioFormatString += FString::Printf(TEXT(" sample_rate=\"%i\""), AudioSampleRate);
	}

	const int64 FrameRateD = Options.GetMediaOption(NdiMedia::FrameRateDOption, (int64)0);

	if (FrameRateD > 0)
	{
		VideoFormatString += FString::Printf(TEXT(" frame_rate_d=\"%i\""), FrameRateD);
	}

	const int64 FrameRateN = Options.GetMediaOption(NdiMedia::FrameRateDOption, (int64)0);

	if (FrameRateN > 0)
	{
		VideoFormatString += FString::Printf(TEXT(" frame_rate_n=\"%i\""), FrameRateN);
	}

	const FString Progressive = Options.GetMediaOption(NdiMedia::ProgressiveOption, FString());

	if (!Progressive.IsEmpty())
	{
		VideoFormatString += FString::Printf(TEXT(" progressive=\"%s\""), *Progressive);
	}

	const int64 VideoHeight = Options.GetMediaOption(NdiMedia::VideoHeightOption, (int64)0);

	if (VideoHeight > 0)
	{
		VideoFormatString += FString::Printf(TEXT(" yres=\"%i\""), VideoHeight);
	}

	const int64 VideoWidth = Options.GetMediaOption(NdiMedia::VideoWidthOption, (int64)0);

	if (VideoWidth > 0)
	{
		VideoFormatString += FString::Printf(TEXT(" xres=\"%i\""), VideoWidth);
	}

	if (!AudioFormatString.IsEmpty() || !VideoFormatString.IsEmpty())
	{
		SendMetadata(
			FString::Printf(TEXT("<ndi_format><audio_format %s /><video_format %s /></ndi_format>"),
				*AudioFormatString,
				*VideoFormatString
		));
	}

	// send custom metadata
	FString CustomMetadata = Settings->CustomMetaData;
	{
		CustomMetadata.Trim();
		CustomMetadata.TrimTrailing();
	}

	if (!CustomMetadata.IsEmpty())
	{
		SendMetadata(CustomMetadata);
	}

	// finalize
	CurrentUrl = Url;

	MediaEvent.Broadcast(EMediaEvent::TracksChanged);
	MediaEvent.Broadcast(EMediaEvent::MediaOpened);

	return true;
}


bool FNdiMediaPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions& Options)
{
	return false; // not supported
}


/* IMediaOutput interface
 *****************************************************************************/

bool FNdiMediaPlayer::SetAudioNative(bool Enabled)
{
	return false; // not supported
}


void FNdiMediaPlayer::SetAudioNativeVolume(float Volume)
{
	// not supported
}


void FNdiMediaPlayer::SetAudioSink(TSharedPtr<IMediaAudioSink, ESPMode::ThreadSafe> Sink)
{
	if (Sink != AudioSinkPtr)
	{
		if (AudioSinkPtr.IsValid())
		{
			AudioSinkPtr.Pin()->FlushAudioSink(true);
		}

		AudioSinkPtr = Sink;
	}
}


void FNdiMediaPlayer::SetMetadataSink(TSharedPtr<IMediaBinarySink, ESPMode::ThreadSafe> Sink)
{
	if (Sink != MetadataSinkPtr)
	{
		if (MetadataSinkPtr.IsValid())
		{
			MetadataSinkPtr.Pin()->FlushBinarySink(true);
		}

		MetadataSinkPtr = Sink;
	}
}


void FNdiMediaPlayer::SetOverlaySink(TSharedPtr<IMediaOverlaySink, ESPMode::ThreadSafe> Sink)
{
	// not supported
}


void FNdiMediaPlayer::SetVideoSink(TSharedPtr<IMediaTextureSink, ESPMode::ThreadSafe> Sink)
{
	if (Sink != VideoSinkPtr)
	{
		if (VideoSinkPtr.IsValid())
		{
			VideoSinkPtr.Pin()->FlushTextureSink(true);
		}

		VideoSinkPtr = Sink;
	}
}


/* IMediaTickable interface
 *****************************************************************************/

void FNdiMediaPlayer::TickInput(FTimespan Timecode, FTimespan /*DeltaTime*/, bool /*Locked*/)
{
	if (ReceiverInstance == nullptr)
	{
		return;
	}

	// update player state
	EMediaState NewState = Paused ? EMediaState::Paused : (NDIlib_recv_is_connected(ReceiverInstance) ? EMediaState::Playing : EMediaState::Preparing);

	if (NewState != CurrentState)
	{
		CurrentState = NewState;
		MediaEvent.Broadcast(NewState == EMediaState::Playing ? EMediaEvent::PlaybackResumed : EMediaEvent::PlaybackSuspended);
	}

	// receive metadata & video frames
	ProcessMetadataAndVideo(Timecode);
}


/* IMediaTracks interface
 *****************************************************************************/

uint32 FNdiMediaPlayer::GetAudioTrackChannels(int32 TrackIndex) const
{
	return ((ReceiverInstance != nullptr) && (TrackIndex == 0)) ? LastAudioChannels : 0;
}


uint32 FNdiMediaPlayer::GetAudioTrackSampleRate(int32 TrackIndex) const
{
	return ((ReceiverInstance != nullptr) && (TrackIndex == 0)) ? LastAudioSampleRate : 0;
}


bool FNdiMediaPlayer::GetCacheState(EMediaTrackType TrackType, EMediaCacheState State, TRangeSet<FTimespan>& OutCachedTimes) const
{
	return false; // not supported
}


int32 FNdiMediaPlayer::GetNumTracks(EMediaTrackType TrackType) const
{
	if (ReceiverInstance != nullptr)
	{
		if ((TrackType == EMediaTrackType::Audio) ||
			(TrackType == EMediaTrackType::Metadata) ||
			(TrackType == EMediaTrackType::Video))
		{
			return 1;
		}
	}

	return 0;
}


int32 FNdiMediaPlayer::GetSelectedTrack(EMediaTrackType TrackType) const
{
	if (ReceiverInstance == nullptr)
	{
		return INDEX_NONE;
	}

	switch (TrackType)
	{
	case EMediaTrackType::Audio:
	case EMediaTrackType::Metadata:
	case EMediaTrackType::Video:
		return 0;

	default:
		return INDEX_NONE;
	}
}


FText FNdiMediaPlayer::GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0))
	{
		return FText::GetEmpty();
	}

	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		return LOCTEXT("DefaultAudioTrackName", "Audio Track");

	case EMediaTrackType::Metadata:
		return LOCTEXT("DefaultMetadataTrackName", "Metadata Track");

	case EMediaTrackType::Video:
		return LOCTEXT("DefaultVideoTrackName", "Video Track");

	default:
		return FText::GetEmpty();
	}
}


FString FNdiMediaPlayer::GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return ((ReceiverInstance != nullptr) && (TrackIndex == 0)) ? TEXT("und") : FString();
}


FString FNdiMediaPlayer::GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FString();
}


uint64 FNdiMediaPlayer::GetVideoTrackBitRate(int32 TrackIndex) const
{
	return ((ReceiverInstance != nullptr) && (TrackIndex == 0)) ? LastVideoBitRate : 0;
}


FIntPoint FNdiMediaPlayer::GetVideoTrackDimensions(int32 TrackIndex) const
{
	return ((ReceiverInstance != nullptr) && (TrackIndex == 0)) ? LastVideoDim : FIntPoint::ZeroValue;
}


float FNdiMediaPlayer::GetVideoTrackFrameRate(int32 TrackIndex) const
{
	return ((ReceiverInstance != nullptr) && (TrackIndex == 0)) ? LastVideoFrameRate : 0;
}


bool FNdiMediaPlayer::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	if ((TrackIndex != INDEX_NONE) && (TrackIndex != 0))
	{
		return false;
	}

	if (TrackType == EMediaTrackType::Audio)
	{
		if (SelectedAudioTrack != TrackIndex)
		{
			SelectedAudioTrack = TrackIndex;

			if (AudioSinkPtr.IsValid())
			{
				AudioSinkPtr.Pin()->FlushAudioSink(false);
			}
		}

		return true;
	}

	if (TrackType == EMediaTrackType::Metadata)
	{
		if (SelectedMetadataTrack != TrackIndex)
		{
			SelectedMetadataTrack = TrackIndex;

			if (MetadataSinkPtr.IsValid())
			{
				MetadataSinkPtr.Pin()->FlushBinarySink(false);
			}
		}

		return true;
	}
	
	if (TrackType == EMediaTrackType::Video)
	{
		if (SelectedVideoTrack != TrackIndex)
		{
			SelectedVideoTrack = TrackIndex;

			if (VideoSinkPtr.IsValid())
			{
				VideoSinkPtr.Pin()->FlushTextureSink(false);
			}
		}

		return true;
	}

	return false;
}


/* INdiMediaAudioTickable interface
 *****************************************************************************/

void FNdiMediaPlayer::TickAudio(FTimespan Timecode)
{
	FScopeLock Lock(&CriticalSection);

	if (ReceiverInstance != nullptr)
	{
		if (AudioSinkPtr.IsValid())
		{
			const bool PlayAudio = (CurrentState == EMediaState::Playing) && (SelectedAudioTrack == 0);
			AudioSinkPtr.Pin()->SetAudioSinkRate(PlayAudio ? 1.0f : 0.0f);
		}

		ProcessAudio(Timecode);
	}
}



/* FNdiMediaPlayer implementation
 *****************************************************************************/

void FNdiMediaPlayer::FlushSinks(bool Shutdown)
{
	if (AudioSinkPtr.IsValid())
	{
		AudioSinkPtr.Pin()->FlushAudioSink(Shutdown);
	}

	if (MetadataSinkPtr.IsValid())
	{
		MetadataSinkPtr.Pin()->FlushBinarySink(Shutdown);
	}

	if (VideoSinkPtr.IsValid())
	{
		VideoSinkPtr.Pin()->FlushTextureSink(Shutdown);
	}
}


void FNdiMediaPlayer::ProcessAudio(FTimespan Timecode)
{
	auto AudioSink = AudioSinkPtr.Pin();

	while (true)
	{
		NDIlib_audio_frame_t AudioFrame;
		NDIlib_frame_type_e FrameType = NDIlib_recv_capture(ReceiverInstance, nullptr, &AudioFrame, nullptr, 0);

		if (FrameType == NDIlib_frame_type_error)
		{
			UE_LOG(LogNdiMedia, Verbose, TEXT("Failed to receive audio frame"));
			return;
		}

		if (FrameType == NDIlib_frame_type_none)
		{
			return; // no more frames available
		}

		if (FrameType == NDIlib_frame_type_audio)
		{
			LastAudioChannels = AudioFrame.no_channels;
			LastAudioSampleRate = AudioFrame.sample_rate;
			LastTimecode = FTimespan(AudioFrame.timecode);

			// forward sample to audio sink, or release buffer
			if ((CurrentState == EMediaState::Playing) && (SelectedAudioTrack == 0) && AudioSink.IsValid())
			{
				TSharedRef<FNdiMediaAudioSample, ESPMode::ThreadSafe> Sample = MakeShareable(
					new FNdiMediaAudioSample(
						ReceiverInstance,
						Timecode,
						AudioFrame,
						UseTimecode
					)
				);

				AudioSink->OnAudioSample(Sample);
			}
			else
			{
				NDIlib_recv_free_audio(ReceiverInstance, &AudioFrame);
			}
		}
	}
}


void FNdiMediaPlayer::ProcessMetadataAndVideo(FTimespan Timecode)
{
	auto MetadataSink = MetadataSinkPtr.Pin();
	auto VideoSink = VideoSinkPtr.Pin();

	while (true)
	{
		NDIlib_metadata_frame_t MetadataFrame;
		NDIlib_video_frame_t VideoFrame;
		NDIlib_frame_type_e FrameType = NDIlib_recv_capture(ReceiverInstance, &VideoFrame, nullptr, &MetadataFrame, 0);

		if (FrameType == NDIlib_frame_type_error)
		{
			UE_LOG(LogNdiMedia, Verbose, TEXT("Failed to receive NDI frame"));
			return;
		}

		if (FrameType == NDIlib_frame_type_none)
		{
			return; // no more frames available
		}

		if (FrameType == NDIlib_frame_type_metadata)
		{
			LastTimecode = FTimespan(MetadataFrame.timecode);

			// forward sample to metadata sink, or release buffer
			if ((CurrentState == EMediaState::Playing) && (SelectedMetadataTrack == 0) && MetadataSink.IsValid())
			{
				TSharedRef<FNdiMediaBinarySample, ESPMode::ThreadSafe> Sample = MakeShareable(
					new FNdiMediaBinarySample(
						ReceiverInstance,
						Timecode,
						MetadataFrame,
						UseTimecode
					)
				);

				MetadataSink->OnBinarySample(Sample);
			}
			else
			{
				NDIlib_recv_free_metadata(ReceiverInstance, &MetadataFrame);
			}
		}
		else if (FrameType == NDIlib_frame_type_video)
		{
			LastTimecode = FTimespan(VideoFrame.timecode);
			LastVideoDim = FIntPoint(VideoFrame.line_stride_in_bytes / 4, VideoFrame.yres);
			LastVideoFrameRate = (float)VideoFrame.frame_rate_N / (float)VideoFrame.frame_rate_D;
			LastVideoBitRate = (uint64)(VideoFrame.line_stride_in_bytes * VideoFrame.yres * LastVideoFrameRate);

			// forward sample to video sink, or release buffer
			if ((CurrentState == EMediaState::Playing) && (SelectedVideoTrack == 0) && VideoSink.IsValid())
			{
				TSharedRef<FNdiMediaTextureSample, ESPMode::ThreadSafe> Sample = MakeShareable(
					new FNdiMediaTextureSample(
						ReceiverInstance,
						Timecode,
						VideoFrame,
						VideoSampleFormat,
						UseTimecode
					)
				);

				VideoSink->OnTextureSample(Sample);
			}
			else
			{
				NDIlib_recv_free_video(ReceiverInstance, &VideoFrame);
			}
		}
	}
}


void FNdiMediaPlayer::SendMetadata(const FString& Metadata, int64 Timecode)
{
	check(ReceiverInstance != nullptr);

	NDIlib_metadata_frame_t MetadataFrame;
	{
		MetadataFrame.length = Metadata.Len() + 1;
		MetadataFrame.timecode = Timecode;
		MetadataFrame.p_data = TCHAR_TO_ANSI(*Metadata);
	}

	NDIlib_recv_add_connection_metadata(ReceiverInstance, &MetadataFrame);
}


#undef LOCTEXT_NAMESPACE

#include "NdiMediaHidePlatformTypes.h"
