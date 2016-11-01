#pragma once

//****************************************************************************************************************
struct NDIlib_v2
{	// NDI Lib functions
	bool (*NDIlib_initialize)(void);
	void (*NDIlib_destroy)(void);
	const char* (*NDIlib_version)(void);
	bool (*NDIlib_is_supported_CPU)(void);

	// NDI Find functions
	NDIlib_find_instance_t (*NDIlib_find_create)(const NDIlib_find_create_t* p_create_settings);
	NDIlib_find_instance_t (*NDIlib_find_create2)(const NDIlib_find_create_t* p_create_settings);
	void (*NDIlib_find_destroy)(NDIlib_find_instance_t p_instance);
	const NDIlib_source_t* (*NDIlib_find_get_sources)(NDIlib_find_instance_t p_instance, int* p_no_sources, uint32_t timeout_in_ms);

	// NDI Send functions
	NDIlib_send_instance_t (*NDIlib_send_create)(const NDIlib_send_create_t* p_create_settings);
	void (*NDIlib_send_destroy)(NDIlib_send_instance_t p_instance);
	void (*NDIlib_send_send_video)(NDIlib_send_instance_t p_instance, const NDIlib_video_frame_t* p_video_data);
	void (*NDIlib_send_send_video_async)(NDIlib_send_instance_t p_instance, const NDIlib_video_frame_t* p_video_data);
	void (*NDIlib_send_send_audio)(NDIlib_send_instance_t p_instance, const NDIlib_audio_frame_t* p_audio_data);
	void (*NDIlib_send_send_metadata)(NDIlib_send_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	NDIlib_frame_type_e (*NDIlib_send_capture)( NDIlib_send_instance_t p_instance, NDIlib_metadata_frame_t* p_metadata, uint32_t timeout_in_ms);
	void (*NDIlib_send_free_metadata)(NDIlib_send_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	bool (*NDIlib_send_get_tally)(NDIlib_send_instance_t p_instance, NDIlib_tally_t* p_tally, uint32_t timeout_in_ms);
	int (*NDIlib_send_get_no_connections)(NDIlib_send_instance_t p_instance, uint32_t timeout_in_ms);
	void (*NDIlib_send_clear_connection_metadata)(NDIlib_send_instance_t p_instance);
	void (*NDIlib_send_add_connection_metadata)(NDIlib_send_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);

	// NDI Recv functions
	NDIlib_recv_instance_t (*NDIlib_recv_create2)(const NDIlib_recv_create_t* p_create_settings);
	NDIlib_recv_instance_t (*NDIlib_recv_create)(const NDIlib_recv_create_t* p_create_settings);
	void (*NDIlib_recv_destroy)(NDIlib_recv_instance_t p_instance);
	NDIlib_frame_type_e (*NDIlib_recv_capture)( NDIlib_recv_instance_t p_instance, NDIlib_video_frame_t* p_video_data, NDIlib_audio_frame_t* p_audio_data, NDIlib_metadata_frame_t* p_metadata, uint32_t timeout_in_ms);
	void (*NDIlib_recv_free_video)(NDIlib_recv_instance_t p_instance, const NDIlib_video_frame_t* p_video_data);
	void (*NDIlib_recv_free_audio)(NDIlib_recv_instance_t p_instance, const NDIlib_audio_frame_t* p_audio_data);
	void (*NDIlib_recv_free_metadata)(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	bool (*NDIlib_recv_send_metadata)(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	bool (*NDIlib_recv_set_tally)(NDIlib_recv_instance_t p_instance, const NDIlib_tally_t* p_tally);
	void (*NDIlib_recv_get_performance)(NDIlib_recv_instance_t p_instance, NDIlib_recv_performance_t* p_total, NDIlib_recv_performance_t* p_dropped);
	void (*NDIlib_recv_get_queue)(NDIlib_recv_instance_t p_instance, NDIlib_recv_queue_t* p_total);
	void (*NDIlib_recv_clear_connection_metadata)(NDIlib_recv_instance_t p_instance);
	void (*NDIlib_recv_add_connection_metadata)(NDIlib_recv_instance_t p_instance, const NDIlib_metadata_frame_t* p_metadata);
	bool (*NDIlib_recv_is_connected)(NDIlib_recv_instance_t p_instance);

	// NDI Routing functions
	NDIlib_routing_instance_t (*NDIlib_routing_create)(const NDIlib_routing_create_t* p_create_settings);
	void (*NDIlib_routing_destroy)(NDIlib_routing_instance_t p_instance);
	bool (*NDIlib_routing_change)(NDIlib_routing_instance_t p_instance, const NDIlib_source_t* p_source);
	bool (*NDIlib_routing_clear)(NDIlib_routing_instance_t p_instance);

	// NDI utility functions
	void (*NDIlib_util_send_send_audio_interleaved_16s)(NDIlib_send_instance_t p_instance, const NDIlib_audio_frame_interleaved_16s_t* p_audio_data);
	void (*NDIlib_util_audio_to_interleaved_16s)(const NDIlib_audio_frame_t* p_src, NDIlib_audio_frame_interleaved_16s_t* p_dst);
	void (*NDIlib_util_audio_from_interleaved_16s)(const NDIlib_audio_frame_interleaved_16s_t* p_src, NDIlib_audio_frame_t* p_dst);
};

// Load the library
PROCESSINGNDILIB_API
const NDIlib_v2* NDIlib_v2_load(void);