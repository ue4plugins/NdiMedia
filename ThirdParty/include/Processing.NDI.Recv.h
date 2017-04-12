#pragma once

// NOTE : The following license applies to this file ONLY and not to the SDK as a whole. Please review the SDK documentation for
// the description of the full license terms.
//
//***********************************************************************************************************************************************
// 
// Copyright(c) 2016 NewTek, inc
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
// files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, 
// merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//***********************************************************************************************************************************************

//**************************************************************************************************************************
// Structures and type definitions required by NDI finding
// The reference to an instance of the receiver
typedef void* NDIlib_recv_instance_t;

typedef enum NDIlib_recv_bandwidth_e
{
	NDIlib_recv_bandwidth_metadata_only = -10, // Receive metadata.
	NDIlib_recv_bandwidth_audio_only	=  10, // Receive metadata, audio.
	NDIlib_recv_bandwidth_lowest		=  0,  // Receive metadata, audio, video at a lower bandwidth and resolution.
	NDIlib_recv_bandwidth_highest		=  100 // Receive metadata, audio, video at full resolution.

}	NDIlib_recv_bandwidth_e;

typedef enum NDIlib_recv_color_format_e
{	
	NDIlib_recv_color_format_e_BGRX_BGRA = 0,	// No alpha channel: BGRX, Alpha channel: BGRA
	NDIlib_recv_color_format_e_UYVY_BGRA = 1,	// No alpha channel: UYVY, Alpha channel: BGRA
	NDIlib_recv_color_format_e_RGBX_RGBA = 2,	// No alpha channel: RGBX, Alpha channel: RGBA
	NDIlib_recv_color_format_e_UYVY_RGBA = 3	// No alpha channel: UYVY, Alpha channel: RGBA

}	NDIlib_recv_color_format_e;

// The creation structure that is used when you are creating a receiver
typedef struct NDIlib_recv_create_t
{	// The source that you wish to connect to.
	NDIlib_source_t source_to_connect_to;

	// Your preference of color space. See above.
	NDIlib_recv_color_format_e color_format;

	// The bandwidth setting that you wish to use for this video source. Bandwidth
	// controlled by changing both the compression level and the resolution of the source.
	// A good use for low bandwidth is working on WIFI connections. 
	NDIlib_recv_bandwidth_e bandwidth;

	// When this flag is FALSE, all video that you receive will be progressive. For sources
	// that provide fields, this is de-interlaced on the receiving side (because we cannot change
	// what the up-stream source was actually rendering. This is provided as a convenience to
	// down-stream sources that do not wish to understand fielded video. There is almost no 
	// performance impact of using this function.
	bool allow_video_fields;

}	NDIlib_recv_create_t;

// This allows you determine the current performance levels of the receiving to be able to detect whether frames have been dropped
typedef struct NDIlib_recv_performance_t
{	// The number of video frames
	int64_t m_video_frames;

	// The number of audio frames
	int64_t m_audio_frames;

	// The number of metadata frames
	int64_t m_metadata_frames;

}	NDIlib_recv_performance_t;

// Get the current queue depths
typedef struct NDIlib_recv_queue_t
{	// The number of video frames
	int m_video_frames;

	// The number of audio frames
	int m_audio_frames;

	// The number of metadata frames
	int m_metadata_frames;

}	NDIlib_recv_queue_t;

//**************************************************************************************************************************
// Create a new receiver instance. This will return NULL if it fails.
PROCESSINGNDILIB_API
NDIlib_recv_instance_t NDIlib_recv_create_v2(const NDIlib_recv_create_t* p_create_settings);

// For legacy reasons I called this the wrong thing. For backwards compatability.
#define NDIlib_recv_create2 NDIlib_recv_create_v2

// This function is deprecated, please use NDIlib_recv_create_v2 if you can. Using this function will continue to work, and be
// supported for backwards compatibility. This version sets bandwidth to highest and allow fields to true.
PROCESSINGNDILIB_API PROCESSINGNDILIB_DEPRECATED
NDIlib_recv_instance_t NDIlib_recv_create(const NDIlib_recv_create_t* p_create_settings);

// This will destroy an existing receiver instance.
PROCESSINGNDILIB_API
void NDIlib_recv_destroy(NDIlib_recv_instance_t p_instance);

// This will allow you to receive video, audio and metadata frames.
// Any of the buffers can be NULL, in which case data of that type
// will not be captured in this call. This call can be called simultaneously
// on separate threads, so it is entirely possible to receive audio, video, metadata
// all on separate threads. This function will return NDIlib_frame_type_none if no
// data is received within the specified timeout and NDIlib_frame_type_error if the connection is lost.
// Buffers captured with this must be freed with the appropriate free function below.
PROCESSINGNDILIB_API PROCESSINGNDILIB_DEPRECATED
NDIlib_frame_type_e NDIlib_recv_capture(
	NDIlib_recv_instance_t p_instance,   // The library instance
	NDIlib_video_frame_t* p_video_data,  // The video data received (can be NULL)
	NDIlib_audio_frame_t* p_audio_data,  // The audio data received (can be NULL)
	NDIlib_metadata_frame_t* p_metadata, // The metadata received (can be NULL)
	uint32_t timeout_in_ms);             // The amount of time in milliseconds to wait for data.

PROCESSINGNDILIB_API
NDIlib_frame_type_e NDIlib_recv_capture_v2(
	NDIlib_recv_instance_t p_instance,   // The library instance
	NDIlib_video_frame_v2_t* p_video_data,  // The video data received (can be NULL)
	NDIlib_audio_frame_v2_t* p_audio_data,  // The audio data received (can be NULL)
	NDIlib_metadata_frame_t* p_metadata, // The metadata received (can be NULL)
	uint32_t timeout_in_ms);             // The amount of time in milliseconds to wait for data.

// Free the buffers returned by capture for video
PROCESSINGNDILIB_API PROCESSINGNDILIB_DEPRECATED
void NDIlib_recv_free_video(NDIlib_recv_instance_t p_instance, const NDIlib_video_frame_t* p_video_data);

PROCESSINGNDILIB_API
void NDIlib_recv_free_video_v2(NDIlib_recv_instance_t p_instance, const NDIlib_video_frame_v2_t* p_video_data);

// Free the buffers returned by capture for audio
PROCESSINGNDILIB_API PROCESSINGNDILIB_DEPRECATED
void NDIlib_recv_free_audio(NDIlib_recv_instance_t p_instance, const NDIlib_audio_frame_t* p_audio_data);

PROCESSINGNDILIB_API 
void NDIlib_recv_free_audio_v2(NDIlib_recv_instance_t p_instance, const NDIlib_audio_frame_v2_t* p_audio_data);

// Free the buffers returned by capture for metadata
PROCESSINGNDILIB_API
void NDIlib_recv_free_metadata(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);

// This function will send a meta message to the source that we are connected too. This returns FALSE if we are
// not currently connected to anything.
PROCESSINGNDILIB_API
bool NDIlib_recv_send_metadata(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);

// Set the up-stream tally notifications. This returns FALSE if we are not currently connected to anything. That
// said, the moment that we do connect to something it will automatically be sent the tally state.
PROCESSINGNDILIB_API
bool NDIlib_recv_set_tally(NDIlib_recv_instance_t p_instance, const NDIlib_tally_t* p_tally);

// Get the current performance structures. This can be used to determine if you have been calling NDIlib_recv_capture fast
// enough, or if your processing of data is not keeping up with real-time. The total structure will give you the total frame
// counts received, the dropped structure will tell you how many frames have been dropped. Either of these could be NULL.
PROCESSINGNDILIB_API
void NDIlib_recv_get_performance(NDIlib_recv_instance_t p_instance, NDIlib_recv_performance_t* p_total, NDIlib_recv_performance_t* p_dropped);

// This will allow you to determine the current queue depth for all of the frame sources at any time. 
PROCESSINGNDILIB_API
void NDIlib_recv_get_queue(NDIlib_recv_instance_t p_instance, NDIlib_recv_queue_t* p_total);

// Connection based metadata is data that is sent automatically each time a new connection is received. You queue all of these
// up and they are sent on each connection. To reset them you need to clear them all and set them up again. 
PROCESSINGNDILIB_API
void NDIlib_recv_clear_connection_metadata(NDIlib_recv_instance_t p_instance);

// Add a connection metadata string to the list of what is sent on each new connection. If someone is already connected then
// this string will be sent to them immediately.
PROCESSINGNDILIB_API
void NDIlib_recv_add_connection_metadata(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);

// Is this receiver currently connected to a source on the other end, or has the source not yet been found or is no longe ronline.
// This will normally return 0 or 1
PROCESSINGNDILIB_API
int NDIlib_recv_get_no_connections(NDIlib_recv_instance_t p_instance);