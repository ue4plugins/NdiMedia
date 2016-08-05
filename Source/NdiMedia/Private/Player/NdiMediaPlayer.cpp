// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NdiMediaPCH.h"

#if PLATFORM_WINDOWS
	#include "AllowWindowsPlatformTypes.h"
#endif


#define LOCTEXT_NAMESPACE "FNdiMediaTracks"


/* FNdiVideoPlayer structors
 *****************************************************************************/

FNdiMediaPlayer::FNdiMediaPlayer()
	: AudioSink(nullptr)
	, VideoSink(nullptr)
	, Connected(false)
	, LastAudioChannels(0)
	, LastAudioSampleRate(0)
	, LastVideoDimensions(FIntPoint::ZeroValue)
	, LastVideoFrameRate(0.0f)
	, Receiver(nullptr)
{ }


FNdiMediaPlayer::~FNdiMediaPlayer()
{
	Close();
	FlushRenderingCommands();
}


/* FTickerObjectBase interface
 *****************************************************************************/

bool FNdiMediaPlayer::Tick(float DeltaTime)
{
	if (Receiver != nullptr)
	{
		CaptureAudioFrame();

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(NdiMediaCaptureVideoFrame, FNdiMediaPlayer*, This, this,
		{
			This->CaptureVideoFrame();
		});
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
	return 1.0f;
}


EMediaState FNdiMediaPlayer::GetState() const
{
	if (Receiver == nullptr)
	{
		return EMediaState::Closed;
	}

	if (!Connected)
	{
		return EMediaState::Preparing;
	}

	return EMediaState::Playing;
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
	return (Rate == 1.0f);
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
	FScopeLock Lock(&CriticalSection);

	Connected = false;
	CurrentUrl.Empty();
	LastAudioChannels = 0;
	LastAudioSampleRate = 0;
	LastVideoDimensions = FIntPoint::ZeroValue;
	LastVideoFrameRate = 0.0f;

	if (Receiver != nullptr)
	{
		NDIlib_recv_destroy(Receiver);
		Receiver = nullptr;
	}
}


IMediaControls& FNdiMediaPlayer::GetControls()
{
	return *this;
}


FString FNdiMediaPlayer::GetInfo() const
{
	return FString(); // @todo gmp: implement NDI info
}


IMediaOutput& FNdiMediaPlayer::GetOutput()
{
	return *this;
}


FString FNdiMediaPlayer::GetStats() const
{
	return FString(); // @todo gmp: implement NDI stats
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

	NDIlib_recv_create_t RcvCreateDesc = {
		Source,
		NDIlib_recv_color_format_BGRA_BGRA,
		(NDIlib_recv_bandwidth_e)Bandwidth,
		TRUE // allow fielded video
	};

	FScopeLock Lock(&CriticalSection);

	Receiver = NDIlib_recv_create2(&RcvCreateDesc);

	if (Receiver == nullptr)
	{
		UE_LOG(LogNdiMedia, Error, TEXT("Failed to open NDI media source %s: couldn't create receiver"), SourceUrl);

		return false;
	}

	CurrentUrl = Url;

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
}


void FNdiMediaPlayer::SetCaptionSink(IMediaStringSink* Sink)
{
	// not supported
}


void FNdiMediaPlayer::SetImageSink(IMediaTextureSink* Sink)
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
	if ((Receiver == nullptr) || (TrackIndex != 0))
	{
		return 0;
	}

	return LastAudioChannels;
}


uint32 FNdiMediaPlayer::GetAudioTrackSampleRate(int32 TrackIndex) const
{
	if ((Receiver == nullptr) || (TrackIndex != 0))
	{
		return 0;
	}

	return LastAudioSampleRate;
}


int32 FNdiMediaPlayer::GetNumTracks(EMediaTrackType TrackType) const
{
	if (Receiver != nullptr)
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
	if (Receiver == nullptr)
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
	if ((Receiver == nullptr) || (TrackIndex != 0))
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
	if ((Receiver == nullptr) || (TrackIndex != 0))
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
	if ((Receiver == nullptr) || (TrackIndex != 0))
	{
		return FIntPoint::ZeroValue;
	}

	return LastVideoDimensions;
}


float FNdiMediaPlayer::GetVideoTrackFrameRate(int32 TrackIndex) const
{
	if ((Receiver == nullptr) || (TrackIndex != 0))
	{
		return 0;
	}

	return LastVideoFrameRate;
}


bool FNdiMediaPlayer::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	if ((TrackType != EMediaTrackType::Audio) && (TrackType != EMediaTrackType::Video))
	{
		return false;
	}

	return (TrackIndex == 0);
}


/* FNdiMediaPlayer implementation
 *****************************************************************************/

void FNdiMediaPlayer::CaptureAudioFrame()
{
	NDIlib_audio_frame_t AudioFrame;
	NDIlib_frame_type_e FrameType = NDIlib_recv_capture(Receiver, NULL, &AudioFrame, NULL, 0);

	if (FrameType == NDIlib_frame_type_error)
	{
		Connected = false;
	}
	else if (FrameType == NDIlib_frame_type_audio)
	{
		FScopeLock Lock(&CriticalSection);

		if (AudioSink != nullptr)
		{
			uint32 NumSamples = AudioFrame.no_samples * AudioFrame.no_channels;

			// convert float samples to interleaved 16-bit samples
			NDIlib_audio_frame_interleaved_16s_t AudioFrameInterleaved = { 0 };
			{
				AudioFrameInterleaved.reference_level = 20;	// We are going to have 20dB of headroom
				AudioFrameInterleaved.p_data = new short[NumSamples];
			}

			NDIlib_util_audio_to_interleaved_16s(&AudioFrame, &AudioFrameInterleaved);

			// forward to sink
			AudioSink->PlayAudioSink((const uint8*)AudioFrameInterleaved.p_data, NumSamples * sizeof int16, FTimespan(AudioFrame.timecode));
			delete[] AudioFrameInterleaved.p_data;
		}

		NDIlib_recv_free_audio(Receiver, &AudioFrame);
		Connected = true;
	}
}


void FNdiMediaPlayer::CaptureVideoFrame()
{
	check(IsInRenderingThread());

	NDIlib_video_frame_t VideoFrame;
	NDIlib_frame_type_e FrameType = NDIlib_recv_capture(Receiver, &VideoFrame, NULL, NULL, 0);

	if (FrameType == NDIlib_frame_type_error)
	{
		Connected = false;
	}
	else if (FrameType == NDIlib_frame_type_video)
	{
		// re-initialize sink if format changed
		LastVideoDimensions = FIntPoint(VideoFrame.xres, VideoFrame.yres);

		FScopeLock Lock(&CriticalSection);

		if (VideoSink != nullptr)
		{
			// re-initialize sink if format changed
			if ((VideoSink->GetTextureSinkDimensions() == LastVideoDimensions) ||
				VideoSink->InitializeTextureSink(LastVideoDimensions, EMediaTextureSinkFormat::CharBGRA, EMediaTextureSinkMode::Unbuffered))
			{
				VideoSink->UpdateTextureSinkBuffer(VideoFrame.p_data, VideoFrame.line_stride_in_bytes);
				VideoSink->DisplayTextureSinkBuffer(FTimespan(VideoFrame.timecode));
			}
		}

		NDIlib_recv_free_video(Receiver, &VideoFrame);
		Connected = true;
	}
}


#undef LOCTEXT_NAMESPACE


#if PLATFORM_WINDOWS
	#include "HideWindowsPlatformTypes.h"
#endif
