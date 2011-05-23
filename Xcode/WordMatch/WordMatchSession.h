//
//  WordMatchSession.h
//  WordMatch
//
//  Copyright 2011 Heinrich Fink. All rights reserved.
//

#ifndef WORD_MATCH_SESSION_H
#define WORD_MATCH_SESSION_H

#include <Availability.h>
#include "Types.h"

#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_4_0

#include <CoreMedia/CMSampleBuffer.h>

/**
 * The following functions describe an API where we want to extract MFCC
 * features for a specified amount of time within one session. These features 
 * are calculated from subsequent (sychronous) chunks which are embedded within 
 * a CMSampleBuffer (such as being accessible via the AVFoundation framework).
 *
 * A session can be created, resetted and destroyed. The client of this API
 * is responsible for doing so.
 *
 * The name of this API is simply WMSession
 */

//make note that hopsize will always be window_size/2, and that window size
//must be 2*N
//as experiment, for complete duration will be stored
WMSessionResult WMSessionCreate(float session_duration,
                                WMMfccConfiguration mfcc_configuration,
                                WMSessionRef session_out);

WMSessionResult WMSessionDestroy(WMSessionRef session);

WMSessionResult WMSessionReset(WMSessionRef session);

WMSessionResult WMSessionFeedFromSampleBuffer(CMSampleBufferRef sample_buffer, 
                                              WMSessionRef session);

bool WMSessionIsCompleted(WMSessionRef session);

//Note that this requires to have a float[13] properly allocated by the caller, in future
//this will be queryable by this api via MFCCConfiguration num mfcc's
WMSessionResult WMSessionGetAverage(WMFeatureType* average_out, 
                                    WMSessionRef session);

#endif

#endif //WORD_MATCH_SESSION_H