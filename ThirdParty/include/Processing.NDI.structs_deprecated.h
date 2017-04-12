#pragma once

// This describes a video frame
PROCESSINGNDILIB_DEPRECATED
typedef struct NDIlib_video_frame_t
{	// The resolution of this frame
	int xres, yres;

	// What FourCC this is with. This can be two values
	NDIlib_FourCC_type_e FourCC;

	// What is the frame-rate of this frame.
	// For instance NTSC is 30000,1001 = 30000/1001 = 29.97fps
	int frame_rate_N, frame_rate_D;

	// What is the picture aspect ratio of this frame.
	// For instance 16.0/9.0 = 1.778 is 16:9 video
	float picture_aspect_ratio;

	// Is this a fielded frame, or is it progressive
	NDIlib_frame_format_type_e frame_format_type;

	// The timecode of this frame in 100ns intervals
	int64_t timecode;

	// The video data itself
	uint8_t* p_data;

	// The inter line stride of the video data, in bytes.
	int line_stride_in_bytes;

}	NDIlib_video_frame_t;

// This describes an audio frame
PROCESSINGNDILIB_DEPRECATED
typedef struct NDIlib_audio_frame_t
{	// The sample-rate of this buffer
	int sample_rate;

	// The number of audio channels
	int no_channels;

	// The number of audio samples per channel
	int no_samples;

	// The timecode of this frame in 100ns intervals
	int64_t timecode;

	// The audio data
	float* p_data;

	// The inter channel stride of the audio channels, in bytes
	int channel_stride_in_bytes;

}	NDIlib_audio_frame_t;