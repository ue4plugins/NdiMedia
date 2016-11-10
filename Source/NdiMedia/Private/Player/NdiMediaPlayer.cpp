// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPCH.h"
#include "NdiMediaAudioSampler.h"
#include "NdiMediaPlayer.h"
#include "NdiMediaAllowPlatformTypes.h"


#define LOCTEXT_NAMESPACE "FNdiMediaTracks"


/* FNdiVideoPlayer structors
 *****************************************************************************/

FNdiMediaPlayer::FNdiMediaPlayer()
	: AudioSink(nullptr)
	, VideoSink(nullptr)
	, SelectedAudioTrack(INDEX_NONE)
	, SelectedVideoTrack(INDEX_NONE)
	, AudioSampler(new FNdiMediaAudioSampler)
	, CurrentState(EMediaState::Closed)
	, LastAudioChannels(0)
	, LastAudioSampleRate(0)
	, LastVideoDimensions(FIntPoint::ZeroValue)
	, LastVideoFrameRate(0.0f)
	, Paused(false)
	, ReceiverInstance(nullptr)
{
	AudioSampler->OnSamples().BindRaw(this, &FNdiMediaPlayer::HandleAudioSamplerSample);
}


FNdiMediaPlayer::~FNdiMediaPlayer()
{
	Close();
	FlushRenderingCommands();

	AudioSampler->OnSamples().Unbind();
	delete AudioSampler;
	AudioSampler = nullptr;
}


/* FTickerObjectBase interface
 *****************************************************************************/

bool FNdiMediaPlayer::Tick(float DeltaTime)
{
	if (ReceiverInstance == nullptr)
	{
		return true;
	}

	// retrieve frame samples
	if (!Paused)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(NdiMediaCaptureVideoFrame, FNdiMediaPlayer*, This, this,
		{
			This->CaptureVideoFrame();
		});
	}

	// update player state
	EMediaState State = NDIlib_recv_is_connected(ReceiverInstance)
		? (Paused ? EMediaState::Paused : EMediaState::Playing)
		: EMediaState::Preparing;

	if ((State != CurrentState) && (AudioSink != nullptr))
	{
		CurrentState = State;
		UpdateAudioSampler();

		if (State == EMediaState::Playing)
		{
			MediaEvent.Broadcast(EMediaEvent::PlaybackResumed);

			if (AudioSink != nullptr)
			{
				AudioSink->ResumeAudioSink();
			}
		}
		else
		{
			MediaEvent.Broadcast(EMediaEvent::PlaybackSuspended);

			if (AudioSink != nullptr)
			{
				AudioSink->PauseAudioSink();
				AudioSink->FlushAudioSink();
			}
		}
	}

	return true;
}


/* IMediaControls interface
 *****************************************************************************/

FTimespan FNdiMediaPlayer::GetDuration() const
{
	return FTimespan::Zero(); //MaxValue?
}


float FNdiMediaPlayer::GetRate() const
{
	return (CurrentState == EMediaState::Playing) ? 1.0f : 0.0f;
}


EMediaState FNdiMediaPlayer::GetState() const
{
	return CurrentState;
}


TRange<float> FNdiMediaPlayer::GetSupportedRates(EMediaPlaybackDirections Direction, bool Unthinned) const
{
	return TRange<float>(1.0f);
}


FTimespan FNdiMediaPlayer::GetTime() const
{
	return FTimespan::Zero(); //MaxValue?
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


bool FNdiMediaPlayer::SupportsRate(float Rate, bool Unthinned) const
{
	return (Rate == 1.0f);
}


bool FNdiMediaPlayer::SupportsScrubbing() const
{
	return false; // not supported
}


bool FNdiMediaPlayer::SupportsSeeking() const
{
	return false; // not supported
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

		CurrentState = EMediaState::Closed;
		CurrentUrl.Empty();

		LastAudioChannels = 0;
		LastAudioSampleRate = 0;
		LastVideoDimensions = FIntPoint::ZeroValue;
		LastVideoFrameRate = 0.0f;

		SelectedAudioTrack = INDEX_NONE;
		SelectedVideoTrack = INDEX_NONE;
	}

	UpdateAudioSampler();

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

	const TCHAR* SourceUrl = &Url[6];

	// create receiver
	int64 Bandwidth = Options.GetMediaOption("Bandwidth", (int64)NDIlib_recv_bandwidth_highest);

	NDIlib_source_t Source;
	{
		Source.p_ip_address = TCHAR_TO_ANSI(SourceUrl);
		Source.p_ndi_name = nullptr;
	}

	NDIlib_recv_create_t RcvCreateDesc;
	{
		RcvCreateDesc.source_to_connect_to = Source;
		RcvCreateDesc.color_format = NDIlib_recv_color_format_e_BGRX_BGRA;
		RcvCreateDesc.bandwidth = (NDIlib_recv_bandwidth_e)Bandwidth;
		RcvCreateDesc.allow_video_fields = true;
	};

	FScopeLock Lock(&CriticalSection);

	ReceiverInstance = NDIlib_recv_create2(&RcvCreateDesc);

	if (ReceiverInstance == nullptr)
	{
		UE_LOG(LogNdiMedia, Error, TEXT("Failed to open NDI media source %s: couldn't create receiver"), SourceUrl);

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

	const int64 AudioChannels = Options.GetMediaOption("AudioChannels", (int64)0);
	
	if (AudioChannels > 0)
	{
		AudioFormatString += FString::Printf(TEXT(" no_channels=\"%i\""), AudioChannels);
	}

	const int64 AudioSampleRate = Options.GetMediaOption("AudioSampleRate", (int64)0);

	if (AudioSampleRate > 0)
	{
		AudioFormatString += FString::Printf(TEXT(" sample_rate=\"%i\""), AudioSampleRate);
	}

	const int64 FrameRateD = Options.GetMediaOption("FrameRateD", (int64)0);

	if (FrameRateD > 0)
	{
		VideoFormatString += FString::Printf(TEXT(" frame_rate_d=\"%i\""), FrameRateD);
	}

	const int64 FrameRateN = Options.GetMediaOption("FrameRateD", (int64)0);

	if (FrameRateN > 0)
	{
		VideoFormatString += FString::Printf(TEXT(" frame_rate_n=\"%i\""), FrameRateN);
	}

	const FString Progressive = Options.GetMediaOption("Progressive", FString());

	if (!Progressive.IsEmpty())
	{
		VideoFormatString += FString::Printf(TEXT(" progressive=\"%s\""), *Progressive);
	}

	const int64 VideoHeight = Options.GetMediaOption("VideoHeight", (int64)0);

	if (VideoHeight > 0)
	{
		VideoFormatString += FString::Printf(TEXT(" yres=\"%i\""), VideoHeight);
	}

	const int64 VideoWidth = Options.GetMediaOption("VideoWidth", (int64)0);

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

void FNdiMediaPlayer::SetAudioSink(IMediaAudioSink* Sink)
{
	if (Sink == AudioSink)
	{
		return;
	}

	FScopeLock Lock(&CriticalSection);

	if (AudioSink != nullptr)
	{
		AudioSink->ShutdownAudioSink();
	}

	AudioSink = Sink;
		
	if (Sink != nullptr)
	{
		Sink->InitializeAudioSink(LastAudioChannels, LastAudioSampleRate);
	}

	UpdateAudioSampler();
}


void FNdiMediaPlayer::SetOverlaySink(IMediaOverlaySink* Sink)
{
	// not supported
}


void FNdiMediaPlayer::SetVideoSink(IMediaTextureSink* Sink)
{
	if (Sink == VideoSink)
	{
		return;
	}

	FScopeLock Lock(&CriticalSection);

	if (VideoSink != nullptr)
	{
		VideoSink->ShutdownTextureSink();
	}

	VideoSink = Sink;

	if (Sink != nullptr)
	{
		Sink->InitializeTextureSink(LastVideoDimensions, EMediaTextureSinkFormat::CharBGRA, EMediaTextureSinkMode::Unbuffered);
	}
}


/* IMediaTracks interface
 *****************************************************************************/

uint32 FNdiMediaPlayer::GetAudioTrackChannels(int32 TrackIndex) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0))
	{
		return 0;
	}

	return LastAudioChannels;
}


uint32 FNdiMediaPlayer::GetAudioTrackSampleRate(int32 TrackIndex) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0))
	{
		return 0;
	}

	return LastAudioSampleRate;
}


int32 FNdiMediaPlayer::GetNumTracks(EMediaTrackType TrackType) const
{
	if (ReceiverInstance != nullptr)
	{
		if ((TrackType == EMediaTrackType::Audio) ||
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

	case EMediaTrackType::Video:
		return LOCTEXT("DefaultVideoTrackName", "Video Track");

	default:
		return FText::GetEmpty();
	}
}


FString FNdiMediaPlayer::GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0))
	{
		return FString();
	}

	return TEXT("und");
}


FString FNdiMediaPlayer::GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FString();
}


uint32 FNdiMediaPlayer::GetVideoTrackBitRate(int32 TrackIndex) const
{
	return 0;
}


FIntPoint FNdiMediaPlayer::GetVideoTrackDimensions(int32 TrackIndex) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0))
	{
		return FIntPoint::ZeroValue;
	}

	return LastVideoDimensions;
}


float FNdiMediaPlayer::GetVideoTrackFrameRate(int32 TrackIndex) const
{
	if ((ReceiverInstance == nullptr) || (TrackIndex != 0))
	{
		return 0;
	}

	return LastVideoFrameRate;
}


bool FNdiMediaPlayer::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	if ((TrackIndex != INDEX_NONE) && (TrackIndex != 0))
	{
		return false;
	}

	if (TrackType == EMediaTrackType::Audio)
	{
		SelectedAudioTrack = TrackIndex;
		UpdateAudioSampler();
	}
	else if (TrackType == EMediaTrackType::Video)
	{
		SelectedVideoTrack = TrackIndex;
	}
	else
	{
		return false;
	}

	return true;
}


/* FNdiMediaPlayer implementation
 *****************************************************************************/

void FNdiMediaPlayer::CaptureVideoFrame()
{
	check(IsInRenderingThread());

	NDIlib_video_frame_t VideoFrame;
	NDIlib_frame_type_e FrameType = NDIlib_recv_capture(ReceiverInstance, &VideoFrame, nullptr, nullptr, 0);

	if (FrameType == NDIlib_frame_type_error)
	{
		UE_LOG(LogNdiMedia, Verbose, TEXT("Failed to receive video frame"));
		return;
	}

	if (FrameType != NDIlib_frame_type_video)
	{
		return;
	}

	// re-initialize sink if format changed
	FScopeLock Lock(&CriticalSection);
	ProcessVideoFrame(VideoFrame);

	NDIlib_recv_free_video(ReceiverInstance, &VideoFrame);
}


void FNdiMediaPlayer::ProcessAudioFrame(const NDIlib_audio_frame_t& AudioFrame)
{
	LastAudioChannels = AudioFrame.no_channels;
	LastAudioSampleRate = AudioFrame.sample_rate;

	if (AudioSink == nullptr)
	{
		return;
	}

	// re-initialize sink if format changed
	if ((AudioSink->GetAudioSinkChannels() != AudioFrame.no_channels) ||
		(AudioSink->GetAudioSinkSampleRate() != AudioFrame.sample_rate))
	{
		if (!AudioSink->InitializeAudioSink(AudioFrame.no_channels, AudioFrame.sample_rate))
		{
			return;
		}
	}

	// convert float samples to interleaved 16-bit samples
	uint32 TotalSamples = AudioFrame.no_samples * AudioFrame.no_channels;

	NDIlib_audio_frame_interleaved_16s_t AudioFrameInterleaved = { 0 };
	{
		AudioFrameInterleaved.reference_level = 20;
		AudioFrameInterleaved.p_data = new short[TotalSamples];
	}

	NDIlib_util_audio_to_interleaved_16s(&AudioFrame, &AudioFrameInterleaved);

	// forward to sink
	static int64 SamplesReceived = 0;
	SamplesReceived += TotalSamples;
	AudioSink->PlayAudioSink((const uint8*)AudioFrameInterleaved.p_data, TotalSamples * sizeof(int16), FTimespan(AudioFrame.timecode));

	delete[] AudioFrameInterleaved.p_data;
}


void FNdiMediaPlayer::ProcessVideoFrame(const NDIlib_video_frame_t& VideoFrame)
{
	LastVideoDimensions = FIntPoint(VideoFrame.xres, VideoFrame.yres);

	if (VideoSink == nullptr)
	{
		return;
	}

	// re-initialize sink if format changed
	if (VideoSink->GetTextureSinkDimensions() != LastVideoDimensions)
	{
		if (!VideoSink->InitializeTextureSink(LastVideoDimensions, EMediaTextureSinkFormat::CharBGRA, EMediaTextureSinkMode::Unbuffered))
		{
			return;
		}
	}

	// forward to sink
	VideoSink->UpdateTextureSinkBuffer(VideoFrame.p_data, VideoFrame.line_stride_in_bytes);
	VideoSink->DisplayTextureSinkBuffer(FTimespan(VideoFrame.timecode));
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


void FNdiMediaPlayer::UpdateAudioSampler()
{
	const bool SampleAudio = !Paused && (AudioSink != nullptr) && (SelectedAudioTrack == 0);
	AudioSampler->SetReceiverInstance(SampleAudio ? ReceiverInstance : nullptr);
}


/* FNdiMediaPlayer implementation
 *****************************************************************************/

void FNdiMediaPlayer::HandleAudioSamplerSample(const NDIlib_audio_frame_t& AudioFrame)
{
	FScopeLock Lock(&CriticalSection);
	ProcessAudioFrame(AudioFrame);
}


#undef LOCTEXT_NAMESPACE


#include "NdiMediaHidePlatformTypes.h"
