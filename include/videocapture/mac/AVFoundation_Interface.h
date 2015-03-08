/*

  C-Interface for the AVFoundation webcam grabber.

 */
#ifndef VIDEO_CAPTURE_AV_FOUNDATION_INTERFACE_H
#define VIDEO_CAPTURE_AV_FOUNDATION_INTERFACE_H

#include <videocapture/Types.h>
#include <vector>

void* ca_av_alloc();                                                                         /* Allocate the AVFoundation implementation. */
void ca_av_dealloc(void* cap);                                                               /* Deallocate the AVFoundation implementation */
int ca_av_get_devices(void* cap, std::vector<ca::Device>& result);                           /* Get available devices. */
int ca_av_get_capabilities(void* cap, int device, std::vector<ca::Capability>& result);      /* Get capabilities. */
int ca_av_get_output_formats(void* cap, std::vector<ca::Format>& result);                    /* The AVFoundation frameworks allows conversion of the raw input samples it gets from a webcam into a couple of other output formats. This is not supported by all implementations though. This function returns the formats which are supported. */
//int ca_av_get_output_format(void* cap);                                                      /* Get the output format of the pixel buffers that are passed into the callback. */
int ca_av_open(void* cap, ca::Settings settings);                                            /* Opens the given capture device with the given settings object. */
int ca_av_close(void* cap);                                                                  /* Closes and shuts down the capture device and session. */
int ca_av_start(void* cap);                                                                  /* Start the capture process. */
int ca_av_stop(void* cap);                                                                   /* Stop the capture process. */
void ca_av_set_callback(void* cap, ca::frame_callback fc, void* user);                       /* Set the callback function which will receive the frames. */

#endif
