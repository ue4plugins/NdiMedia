// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPlayer.h"
#include "NdiMediaPrivate.h"

#include "HAL/PlatformProcess.h"
#include "IMediaEventSink.h"
#include "IMediaOptions.h"
#include "MediaSamples.h"
#include "Misc/App.h"
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

FNdiMediaPlayer::FNdiMediaPlayer(IMediaEventSink& InEventSink)
	: AudioSamplePool(new FNdiMediaAudioSamplePool)
	, CurrentState(EMediaState::Closed)
	, CurrentTime(FTimespan::Zero())
	, EventSink(InEventSink)
	, LastAudioChannels(0)
	, LastAudioSampleRate(0)
	, LastVideoBitRate(0)
	, LastVideoDim(FIntPoint::ZeroValue)
	, LastVideoFrameRate(0.0f)
	, Paused(false)
	, ReceiverInstance(nullptr)
	, Samples(new FMediaSamples)
	, SelectedAudioTrack(INDEX_NONE)
	, SelectedMetadataTrack(INDEX_NONE)
	, SelectedVideoTrack(INDEX_NONE)
	, UseFrameTimecode(false)
	, VideoSampleFormat(EMediaTextureSampleFormat::CharUYVY)
{ }


FNdiMediaPlayer::~FNdiMediaPlayer()
{
	Close();

	delete AudioSamplePool;
	AudioSamplePool = nullptr;

	delete Samples;
	Samples = nullptr;
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

	AudioSamplePool->Reset();

	CurrentState = EMediaState::Closed;
	CurrentTime = FTimespan::Zero();
	CurrentUrl.Empty();

	LastVideoBitRate = 0;
	LastVideoDim = FIntPoint::ZeroValue;
	LastVideoFrameRate = 0.0f;

	SelectedMetadataTrack = INDEX_NONE;
	SelectedVideoTrack = INDEX_NONE;
	SelectedAudioTrack = INDEX_NONE;

	EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
	EventSink.ReceiveMediaEvent(EMediaEvent::MediaClosed);
}


IMediaCache& FNdiMediaPlayer::GetCache()
{
	return *this;
}


IMediaControls& FNdiMediaPlayer::GetControls()
{
	return *this;
}


FString FNdiMediaPlayer::GetInfo() const
{
	FString Info;

	if (LastAudioChannels > 0)
	{
		Info += FString::Printf(TEXT("Stream\n"));
		Info += FString::Printf(TEXT("    Type: Audio\n"));
		Info += FString::Printf(TEXT("    Channels: %i\n"), LastAudioChannels);
		Info += FString::Printf(TEXT("    Sample Rate: %i Hz\n"), LastAudioSampleRate);
		Info += FString::Printf(TEXT("    Bits Per Sample: 16\n"));
	}

	if (LastVideoDim != FIntPoint::ZeroValue)
	{
		if (!Info.IsEmpty())
		{
			Info += TEXT("\n");
		}

		Info += FString::Printf(TEXT("Stream\n"));
		Info += FString::Printf(TEXT("    Type: Audio\n"));
		Info += FString::Printf(TEXT("    Dimensions: %i x %i\n"), LastVideoDim.X, LastVideoDim.Y);
		Info += FString::Printf(TEXT("    Frame Rate: %g fps\n"), LastVideoFrameRate);
		Info += FString::Printf(TEXT("    Bit Rate: %s / second\n"), *FText::AsMemory(LastVideoBitRate).ToString());
	}

	return Info;
}


FName FNdiMediaPlayer::GetPlayerName() const
{
	static FName PlayerName(TEXT("NdiMedia"));
	return PlayerName;
}


IMediaSamples& FNdiMediaPlayer::GetSamples()
{
	return *Samples;
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
		StatsString += FString::Printf(TEXT("    Audio: %i\n"), PerfTotal.audio_frames);
		StatsString += FString::Printf(TEXT("    Video: %i\n"), PerfTotal.video_frames);
		StatsString += FString::Printf(TEXT("    Metadata: %i\n"), PerfTotal.metadata_frames);
		StatsString += TEXT("\n");

		StatsString += TEXT("Dropped Frames\n");
		StatsString += FString::Printf(TEXT("    Audio: %i\n"), PerfDropped.audio_frames);
		StatsString += FString::Printf(TEXT("    Video: %i\n"), PerfDropped.video_frames);
		StatsString += FString::Printf(TEXT("    Metadata: %i\n"), PerfDropped.metadata_frames);
		StatsString += TEXT("\n");

		StatsString += TEXT("Queue Depth\n");
		StatsString += FString::Printf(TEXT("    Audio: %i\n"), Queue.audio_frames);
		StatsString += FString::Printf(TEXT("    Video: %i\n"), Queue.video_frames);
		StatsString += FString::Printf(TEXT("    Metadata: %i\n"), Queue.metadata_frames);
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


IMediaView& FNdiMediaPlayer::GetView()
{
	return *this;
}


bool FNdiMediaPlayer::Open(const FString& Url, const IMediaOptions* Options)
{
	Close();

	if (Url.IsEmpty() || !Url.StartsWith(TEXT("ndi://")))
	{
		return false;
	}

	FString SourceStr = Url.RightChop(6);

	// determine playback options
	int64 Bandwidth;
	NDIlib_recv_color_format_e ColorFormat;
	FString ReceiverName;

	if (Options != nullptr)
	{
		Bandwidth = Options->GetMediaOption(NdiMedia::BandwidthOption, (int64)NDIlib_recv_bandwidth_highest);
		ColorFormat = (NDIlib_recv_color_format_e)Options->GetMediaOption(NdiMedia::ColorFormatOption, 0LL);
		ReceiveAudioReferenceLevel = (int32)Options->GetMediaOption(NdiMedia::AudioReferenceLevelOption, 5LL);
		ReceiverName = Options->GetMediaOption(NdiMedia::ReceiverName, FString());
		UseFrameTimecode = Options->GetMediaOption(NdiMedia::UseTimecodeOption, false);
	}
	else
	{
		Bandwidth = (int64)NDIlib_recv_bandwidth_highest;
		ColorFormat = NDIlib_recv_color_format_e_UYVY_BGRA;
		ReceiveAudioReferenceLevel = 5;
		UseFrameTimecode = false;
	}

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

	if (ReceiverName.IsEmpty())
	{
		ReceiverName = FString::Printf(TEXT("%p"), this);
	}

	const FString UniqueReceiverName = FString::Printf(TEXT("%s %s %s"), *FApp::GetName(), *FApp::GetInstanceName(), *ReceiverName);

	// create receiver
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

	NDIlib_recv_create_v3_t RcvCreateDesc;
	{
		RcvCreateDesc.source_to_connect_to = Source;
		RcvCreateDesc.color_format = ColorFormat;
		RcvCreateDesc.bandwidth = (NDIlib_recv_bandwidth_e)Bandwidth;
		RcvCreateDesc.allow_video_fields = true;
		RcvCreateDesc.p_ndi_name = TCHAR_TO_ANSI(*UniqueReceiverName);
	};
	
	{
		FScopeLock Lock(&CriticalSection);

		ReceiverInstance = NDIlib_recv_create_v3(&RcvCreateDesc);
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

	if (Options != nullptr)
	{
		const int64 AudioChannels = Options->GetMediaOption(NdiMedia::AudioChannelsOption, (int64)0);

		if (AudioChannels > 0)
		{
			AudioFormatString += FString::Printf(TEXT(" no_channels=\"%i\""), AudioChannels);
		}

		const int64 AudioSampleRate = Options->GetMediaOption(NdiMedia::AudioSampleRateOption, (int64)0);

		if (AudioSampleRate > 0)
		{
			AudioFormatString += FString::Printf(TEXT(" sample_rate=\"%i\""), AudioSampleRate);
		}

		const int64 FrameRateD = Options->GetMediaOption(NdiMedia::FrameRateDOption, (int64)0);

		if (FrameRateD > 0)
		{
			VideoFormatString += FString::Printf(TEXT(" frame_rate_d=\"%i\""), FrameRateD);
		}

		const int64 FrameRateN = Options->GetMediaOption(NdiMedia::FrameRateDOption, (int64)0);

		if (FrameRateN > 0)
		{
			VideoFormatString += FString::Printf(TEXT(" frame_rate_n=\"%i\""), FrameRateN);
		}

		const FString Progressive = Options->GetMediaOption(NdiMedia::ProgressiveOption, FString());

		if (!Progressive.IsEmpty())
		{
			VideoFormatString += FString::Printf(TEXT(" progressive=\"%s\""), *Progressive);
		}

		const int64 VideoHeight = Options->GetMediaOption(NdiMedia::VideoHeightOption, (int64)0);

		if (VideoHeight > 0)
		{
			VideoFormatString += FString::Printf(TEXT(" yres=\"%i\""), VideoHeight);
		}

		const int64 VideoWidth = Options->GetMediaOption(NdiMedia::VideoWidthOption, (int64)0);

		if (VideoWidth > 0)
		{
			VideoFormatString += FString::Printf(TEXT(" xres=\"%i\""), VideoWidth);
		}
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
		CustomMetadata.TrimStartInline();
		CustomMetadata.TrimEndInline();
	}

	if (!CustomMetadata.IsEmpty())
	{
		SendMetadata(CustomMetadata);
	}

	// finalize
	CurrentUrl = Url;

	EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
	EventSink.ReceiveMediaEvent(EMediaEvent::MediaOpened);

	return true;
}


bool FNdiMediaPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& /*Archive*/, const FString& /*OriginalUrl*/, const IMediaOptions* /*Options*/)
{
	return false; // not supported
}


void FNdiMediaPlayer::TickAudio()
{
	FScopeLock Lock(&CriticalSection);

	if (ReceiverInstance != nullptr)
	{
		ProcessAudio();
	}
}


void FNdiMediaPlayer::TickFetch(FTimespan DeltaTime, FTimespan /*Timecode*/)
{
	if (ReceiverInstance != nullptr)
	{
		ProcessMetadataAndVideo();
	}
}


void FNdiMediaPlayer::TickInput(FTimespan DeltaTime, FTimespan Timecode)
{
	if (ReceiverInstance == nullptr)
	{
		return;
	}

	// update player state
	const bool IsConnected = (NDIlib_recv_get_no_connections(ReceiverInstance) > 0);
	const EMediaState State = Paused ? EMediaState::Paused : (IsConnected ? EMediaState::Playing : EMediaState::Preparing);

	if (State != CurrentState)
	{
		CurrentState = State;
		EventSink.ReceiveMediaEvent(State == EMediaState::Playing ? EMediaEvent::PlaybackResumed : EMediaEvent::PlaybackSuspended);
	}

	if (!UseFrameTimecode)
	{
		CurrentTime = Timecode;
	}
}


/* IMediaControls interface
 *****************************************************************************/

bool FNdiMediaPlayer::CanControl(EMediaControl Control) const
{
	if (Control == EMediaControl::Pause)
	{
		return (CurrentState == EMediaState::Playing);
	}

	if (Control == EMediaControl::Resume)
	{
		return (CurrentState == EMediaState::Paused);
	}

	return false;
}


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


EMediaStatus FNdiMediaPlayer::GetStatus() const
{
	return (CurrentState == EMediaState::Preparing) ? EMediaStatus::Connecting : EMediaStatus::None;
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
	return CurrentTime;
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


/* IMediaTracks interface
 *****************************************************************************/

bool FNdiMediaPlayer::GetAudioTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaAudioTrackFormat& OutFormat) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0) || (FormatIndex != 0))
	{
		return false;
	}

	OutFormat.BitsPerSample = 16;
	OutFormat.NumChannels = LastAudioChannels;
	OutFormat.SampleRate = LastAudioSampleRate;
	OutFormat.TypeName = TEXT("PCM");

	return true;
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


int32 FNdiMediaPlayer::GetNumTrackFormats(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return ((TrackIndex == 0) && (GetNumTracks(TrackType) > 0)) ? 1 : 0;
}


int32 FNdiMediaPlayer::GetSelectedTrack(EMediaTrackType TrackType) const
{
	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		return SelectedAudioTrack;

	case EMediaTrackType::Metadata:
		return SelectedMetadataTrack;

	case EMediaTrackType::Video:
		return SelectedVideoTrack;

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


int32 FNdiMediaPlayer::GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return (GetSelectedTrack(TrackType) != INDEX_NONE) ? 0 : INDEX_NONE;
}


FString FNdiMediaPlayer::GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return ((ReceiverInstance != nullptr) && (TrackIndex == 0)) ? TEXT("und") : FString();
}


FString FNdiMediaPlayer::GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FString();
}


bool FNdiMediaPlayer::GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaVideoTrackFormat& OutFormat) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0) || (FormatIndex != 0))
	{
		return false;
	}

	OutFormat.Dim = LastVideoDim;
	OutFormat.FrameRate = LastVideoFrameRate;
	OutFormat.FrameRates = TRange<float>(LastVideoFrameRate);
	OutFormat.TypeName = (VideoSampleFormat == EMediaTextureSampleFormat::CharBGRA) ? TEXT("UYVY") : TEXT("RGBA");

	return true;
}


bool FNdiMediaPlayer::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	if ((ReceiverInstance == nullptr) || (TrackIndex < INDEX_NONE) || (TrackIndex > 0))
	{
		return false;
	}

	if (TrackType == EMediaTrackType::Audio)
	{
		if (SelectedAudioTrack != TrackIndex)
		{
			FScopeLock Lock(&CriticalSection);
			SelectedAudioTrack = TrackIndex;
		}

		return true;
	}

	if (TrackType == EMediaTrackType::Metadata)
	{
		if (SelectedMetadataTrack != TrackIndex)
		{
			SelectedMetadataTrack = TrackIndex;
		}

		return true;
	}
	
	if (TrackType == EMediaTrackType::Video)
	{
		if (SelectedVideoTrack != TrackIndex)
		{
			SelectedVideoTrack = TrackIndex;
		}

		return true;
	}

	return false;
}


bool FNdiMediaPlayer::SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex)
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0) || (FormatIndex != 0))
	{
		return false;
	}

	return ((TrackType == EMediaTrackType::Audio) ||
			(TrackType == EMediaTrackType::Metadata) ||
			(TrackType == EMediaTrackType::Video));
}


/* FNdiMediaPlayer implementation
 *****************************************************************************/

void FNdiMediaPlayer::ProcessAudio()
{
	check(ReceiverInstance != nullptr);

	while (true)
	{
		NDIlib_audio_frame_v2_t AudioFrame;
		NDIlib_frame_type_e FrameType = NDIlib_recv_capture_v2(ReceiverInstance, nullptr, &AudioFrame, nullptr, 0);

		if (FrameType == NDIlib_frame_type_error)
		{
			UE_LOG(LogNdiMedia, Verbose, TEXT("Failed to receive audio frame"));
			return;
		}

		if (FrameType == NDIlib_frame_type_none)
		{
			break; // no more frames available
		}

		if (FrameType == NDIlib_frame_type_audio)
		{
			LastAudioChannels = AudioFrame.no_channels;
			LastAudioSampleRate = AudioFrame.sample_rate;

			if (UseFrameTimecode)
			{
				CurrentTime = FTimespan(AudioFrame.timecode);
			}

			// // create & add sample to queue, or release frame
			if ((CurrentState == EMediaState::Playing) && (SelectedAudioTrack == 0))
			{
				auto AudioSample = AudioSamplePool->AcquireShared();
				
				if (AudioSample->Initialize(ReceiverInstance, AudioFrame, ReceiveAudioReferenceLevel, CurrentTime))
				{
					Samples->AddAudio(AudioSample);
				}
			}
			else
			{
				NDIlib_recv_free_audio_v2(ReceiverInstance, &AudioFrame);
			}
		}
	}
}


void FNdiMediaPlayer::ProcessMetadataAndVideo()
{
	check(ReceiverInstance != nullptr);

	while (true)
	{
		NDIlib_metadata_frame_t MetadataFrame;
		NDIlib_video_frame_v2_t VideoFrame;
		NDIlib_frame_type_e FrameType = NDIlib_recv_capture_v2(ReceiverInstance, &VideoFrame, nullptr, &MetadataFrame, 0);

		if (FrameType == NDIlib_frame_type_error)
		{
			UE_LOG(LogNdiMedia, Verbose, TEXT("Failed to receive NDI frame"));
			return;
		}

		if (FrameType == NDIlib_frame_type_none)
		{
			break; // no more frames available
		}

		if (FrameType == NDIlib_frame_type_metadata)
		{
			if (UseFrameTimecode)
			{
				CurrentTime = FTimespan(MetadataFrame.timecode);
			}

			// // create & add sample to queue, or release frame
			if ((CurrentState == EMediaState::Playing) && (SelectedMetadataTrack == 0))
			{
				auto BinarySample = MakeShared<FNdiMediaBinarySample, ESPMode::ThreadSafe>();

				if (BinarySample->Initialize(ReceiverInstance, MetadataFrame, CurrentTime))
				{
					Samples->AddMetadata(BinarySample);
				}
			}
			else
			{
				NDIlib_recv_free_metadata(ReceiverInstance, &MetadataFrame);
			}
		}
		else if (FrameType == NDIlib_frame_type_video)
		{
			LastVideoDim = FIntPoint(VideoFrame.xres, VideoFrame.yres);
			LastVideoFrameRate = (float)VideoFrame.frame_rate_N / (float)VideoFrame.frame_rate_D;
			LastVideoBitRate = (uint64)(VideoFrame.line_stride_in_bytes * VideoFrame.yres * LastVideoFrameRate);

			if (UseFrameTimecode)
			{
				CurrentTime = FTimespan(VideoFrame.timecode);
			}

			// // create & add sample to queue, or release frame
			if ((CurrentState == EMediaState::Playing) && (SelectedVideoTrack == 0))
			{
				auto TextureSample = MakeShared<FNdiMediaTextureSample, ESPMode::ThreadSafe>();

				if (TextureSample->Initialize(ReceiverInstance, VideoFrame, VideoSampleFormat, CurrentTime))
				{
					Samples->AddVideo(TextureSample);
				}
			}
			else
			{
				NDIlib_recv_free_video_v2(ReceiverInstance, &VideoFrame);
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
