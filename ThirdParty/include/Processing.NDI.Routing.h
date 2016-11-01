#pragma once

//**************************************************************************************************************************
// Structures and type definitions required by NDI sending
// The reference to an instance of the sender
typedef void* NDIlib_routing_instance_t;

// The creation structure that is used when you are creating a sender
typedef struct NDIlib_routing_create_t
{	// The name of the NDI source to create. This is a NULL terminated UTF8 string.
	const char* p_ndi_name;

	// What groups should this source be part of
	const char* p_groups;

} NDIlib_routing_create_t;

// Create an NDI routing source
PROCESSINGNDILIB_API
NDIlib_routing_instance_t NDIlib_routing_create(const NDIlib_routing_create_t* p_create_settings);

// Destroy and NDI routing source
PROCESSINGNDILIB_API
void NDIlib_routing_destroy(NDIlib_routing_instance_t p_instance);

// Change the routing of this source to another destination
PROCESSINGNDILIB_API
bool NDIlib_routing_change(NDIlib_routing_instance_t p_instance, const NDIlib_source_t* p_source);

// Change the routing of this source to another destination
PROCESSINGNDILIB_API
bool NDIlib_routing_clear(NDIlib_routing_instance_t p_instance);