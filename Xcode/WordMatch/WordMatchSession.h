//Copyright (c) 2011 Heinrich Fink hf@hfink.eu
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.

#ifndef WORD_MATCH_SESSION_H
#define WORD_MATCH_SESSION_H

#include <Availability.h>
#include "Types.h"

#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_4_0

#include <CoreMedia/CMSampleBuffer.h>

/**
 * The following functions describe an API where we want to extract MFCC
 * features for a specified amount of time within one session. These features 
 * are calculated from subsequent chunks which are embedded within 
 * a CMSampleBuffer (such as being accessible via the AVFoundation framework).
 *
 * A session can be created, resetted and destroyed. The client of this API
 * is responsible for doing so.
 */

/**
 * Creates a new WMSession object. Note that the hopsize for the mfcc extraction
 * always is always window_size/2. Therefore, window_size must be a multiple of 2.
 * @param session_duration Duration of the session in seconds. This parameter is
 * necessary as underyling allocations of buffers can be optimized then.
 * @param mfcc_configuration The configuration for the MFCC processor. An MFCC
 * processor configured as desired is being cached over the lifetime of each
 * session. 
 * @param session_out Out parameter of the new session created.
 */
WMSessionResult WMSessionCreate(float session_duration,
                                WMMfccConfiguration mfcc_configuration,
                                WMSessionRef* session_out);

/**
 * Destroys a previously created session.
 */
WMSessionResult WMSessionDestroy(WMSessionRef session);

/**
 * Resets the internal counters where MFCC feature vectors should be stored
 * into. Note that this call is very lightweight, i.e. it can be called within
 * a performance-sensitive loop, for example.
 */
WMSessionResult WMSessionReset(WMSessionRef session);

/**
 * Feeds the session with a fresh sample buffer. 
 */
WMSessionResult WMSessionFeedFromSampleBuffer(CMSampleBufferRef sample_buffer, 
                                              WMSessionRef session);

/**
 * Returns true if enough samples have been consumed.
 */
bool WMSessionIsCompleted(WMSessionRef session);

/**
 * Calculates the average of MFCC features over the given duration. Note that
 * the session must have been completed before.
 * @param average_out A pointer to an array floats with at least the size of 13.
 * It's the callers responsibility to pre-allocated this array.
 */
WMSessionResult WMSessionGetAverage(WMFeatureType* average_out, 
                                    WMSessionRef session);

#endif

#endif //WORD_MATCH_SESSION_H