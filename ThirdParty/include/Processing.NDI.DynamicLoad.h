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

//****************************************************************************************************************
typedef struct NDIlib_v2
{
	bool(*NDIlib_initialize)(void);
	void(*NDIlib_destroy)(void);
	const char* (*NDIlib_version)(void);
	bool(*NDIlib_is_supported_CPU)(void);
	PROCESSINGNDILIB_DEPRECATED NDIlib_find_instance_t(*NDIlib_find_create)(const NDIlib_find_create_t* p_create_settings);
	NDIlib_find_instance_t(*NDIlib_find_create_v2)(const NDIlib_find_create_t* p_create_settings);
	void(*NDIlib_find_destroy)(NDIlib_find_instance_t p_instance);
	const NDIlib_source_t* (*NDIlib_find_get_sources)(NDIlib_find_instance_t p_instance, uint32_t* p_no_sources, uint32_t timeout_in_ms);
	NDIlib_send_instance_t(*NDIlib_send_create)(const NDIlib_send_create_t* p_create_settings);
	void(*NDIlib_send_destroy)(NDIlib_send_instance_t p_instance);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_send_send_video)(NDIlib_send_instance_t p_instance, const NDIlib_video_frame_t* p_video_data);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_send_send_video_async)(NDIlib_send_instance_t p_instance, const NDIlib_video_frame_t* p_video_data);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_send_send_audio)(NDIlib_send_instance_t p_instance, const NDIlib_audio_frame_t* p_audio_data);
	void(*NDIlib_send_send_metadata)(NDIlib_send_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	NDIlib_frame_type_e(*NDIlib_send_capture)(NDIlib_send_instance_t p_instance, NDIlib_metadata_frame_t* p_metadata, uint32_t timeout_in_ms);
	void(*NDIlib_send_free_metadata)(NDIlib_send_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	bool(*NDIlib_send_get_tally)(NDIlib_send_instance_t p_instance, NDIlib_tally_t* p_tally, uint32_t timeout_in_ms);
	int(*NDIlib_send_get_no_connections)(NDIlib_send_instance_t p_instance, uint32_t timeout_in_ms);
	void(*NDIlib_send_clear_connection_metadata)(NDIlib_send_instance_t p_instance);
	void(*NDIlib_send_add_connection_metadata)(NDIlib_send_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	void(*NDIlib_send_set_failover)(NDIlib_send_instance_t p_instance, const NDIlib_source_t* p_failover_source);
	NDIlib_recv_instance_t(*NDIlib_recv_create_v2)(const NDIlib_recv_create_t* p_create_settings);
	NDIlib_recv_instance_t(*NDIlib_recv_create)(const NDIlib_recv_create_t* p_create_settings);
	void(*NDIlib_recv_destroy)(NDIlib_recv_instance_t p_instance);
	PROCESSINGNDILIB_DEPRECATED NDIlib_frame_type_e(*NDIlib_recv_capture)(NDIlib_recv_instance_t p_instance, NDIlib_video_frame_t* p_video_data, NDIlib_audio_frame_t* p_audio_data, NDIlib_metadata_frame_t* p_metadata, uint32_t timeout_in_ms);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_recv_free_video)(NDIlib_recv_instance_t p_instance, const NDIlib_video_frame_t* p_video_data);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_recv_free_audio)(NDIlib_recv_instance_t p_instance, const NDIlib_audio_frame_t* p_audio_data);
	void(*NDIlib_recv_free_metadata)(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	bool(*NDIlib_recv_send_metadata)(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	bool(*NDIlib_recv_set_tally)(NDIlib_recv_instance_t p_instance, const NDIlib_tally_t* p_tally);
	void(*NDIlib_recv_get_performance)(NDIlib_recv_instance_t p_instance, NDIlib_recv_performance_t* p_total, NDIlib_recv_performance_t* p_dropped);
	void(*NDIlib_recv_get_queue)(NDIlib_recv_instance_t p_instance, NDIlib_recv_queue_t* p_total);
	void(*NDIlib_recv_clear_connection_metadata)(NDIlib_recv_instance_t p_instance);
	void(*NDIlib_recv_add_connection_metadata)(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	int(*NDIlib_recv_get_no_connections)(NDIlib_recv_instance_t p_instance);
	NDIlib_routing_instance_t(*NDIlib_routing_create)(const NDIlib_routing_create_t* p_create_settings);
	void(*NDIlib_routing_destroy)(NDIlib_routing_instance_t p_instance);
	bool(*NDIlib_routing_change)(NDIlib_routing_instance_t p_instance, const NDIlib_source_t* p_source);
	bool(*NDIlib_routing_clear)(NDIlib_routing_instance_t p_instance);
	void(*NDIlib_util_send_send_audio_interleaved_16s)(NDIlib_send_instance_t p_instance, const NDIlib_audio_frame_interleaved_16s_t* p_audio_data);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_util_audio_to_interleaved_16s)(const NDIlib_audio_frame_t* p_src, NDIlib_audio_frame_interleaved_16s_t* p_dst);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_util_audio_from_interleaved_16s)(const NDIlib_audio_frame_interleaved_16s_t* p_src, NDIlib_audio_frame_t* p_dst);

	// Find for v2 functions
	bool(*NDIlib_find_wait_for_sources)(NDIlib_find_instance_t p_instance, uint32_t timeout_in_ms);
	const NDIlib_source_t* (*NDIlib_find_get_current_sources)(NDIlib_find_instance_t p_instance, uint32_t* p_no_sources);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_util_audio_to_interleaved_32f)(const NDIlib_audio_frame_t* p_src, NDIlib_audio_frame_interleaved_32f_t* p_dst);
	PROCESSINGNDILIB_DEPRECATED void(*NDIlib_util_audio_from_interleaved_32f)(const NDIlib_audio_frame_interleaved_32f_t* p_src, NDIlib_audio_frame_t* p_dst);
	void(*NDIlib_util_send_send_audio_interleaved_32f)(NDIlib_send_instance_t p_instance, const NDIlib_audio_frame_interleaved_32f_t* p_audio_data);

	// V2.5 functions
	void(*NDIlib_recv_free_video_v2)(NDIlib_recv_instance_t p_instance, const NDIlib_video_frame_v2_t* p_video_data);
	void(*NDIlib_recv_free_audio_v2)(NDIlib_recv_instance_t p_instance, const NDIlib_audio_frame_v2_t* p_audio_data);
	NDIlib_frame_type_e(*NDIlib_recv_capture_v2)(NDIlib_recv_instance_t p_instance, NDIlib_video_frame_v2_t* p_video_data, NDIlib_audio_frame_v2_t* p_audio_data, NDIlib_metadata_frame_t* p_metadata, uint32_t timeout_in_ms);             // The amount of time in milliseconds to wait for data.
	void(*NDIlib_send_send_video_v2)(NDIlib_send_instance_t p_instance, const NDIlib_video_frame_v2_t* p_video_data);
	void(*NDIlib_send_send_video_async_v2)(NDIlib_send_instance_t p_instance, const NDIlib_video_frame_v2_t* p_video_data);
	void(*NDIlib_send_send_audio_v2)(NDIlib_send_instance_t p_instance, const NDIlib_audio_frame_v2_t* p_audio_data);

	// The new interleaved audio flags
	void(*NDIlib_util_audio_to_interleaved_16s_v2)(const NDIlib_audio_frame_v2_t* p_src, NDIlib_audio_frame_interleaved_16s_t* p_dst);
	void(*NDIlib_util_audio_from_interleaved_16s_v2)(const NDIlib_audio_frame_interleaved_16s_t* p_src, NDIlib_audio_frame_v2_t* p_dst);
	void(*NDIlib_util_audio_to_interleaved_32f_v2)(const NDIlib_audio_frame_v2_t* p_src, NDIlib_audio_frame_interleaved_32f_t* p_dst);
	void(*NDIlib_util_audio_from_interleaved_32f_v2)(const NDIlib_audio_frame_interleaved_32f_t* p_src, NDIlib_audio_frame_v2_t* p_dst);

	// This leaves us a load of space that is useful to allow us to expand the API without breaking compatability. It does
	// not matter that this struct is actually pretty big because there is only ever one instance of it around.
	void(*NDIlib_reserved[500])(void);

} NDIlib_v2;

// Load the library
PROCESSINGNDILIB_API
const NDIlib_v2* NDIlib_v2_load(void);