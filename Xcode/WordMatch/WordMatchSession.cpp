//
//  WordMatchSession.cpp
//  WordMatch
//
//  Copyright 2011 Heinrich Fink. All rights reserved.
//

#include "Types.h"
#include <CoreMedia/CMSampleBuffer.h>

extern "C" WMSessionResult WMSessionCreate(float session_duration,
                                WMMfccConfiguration mfcc_configuration,
                                WMSessionRef session_out)
{
    return kWMSessionResultOK;
}

extern "C" WMSessionResult WMSessionDestroy(WMSessionRef session)
{
    return kWMSessionResultOK;
}

extern "C" WMSessionResult WMSessionReset(WMSessionRef session)
{
    return kWMSessionResultOK;
}

extern "C" WMSessionResult WMSessionFeedFromSampleBuffer(CMSampleBufferRef sample_buffer, 
                                              WMSessionRef session)
{
    return kWMSessionResultOK;
}

extern "C" bool WMSessionIsCompleted(WMSessionRef session)
{
    return false;
}

extern "C" WMSessionResult WMSessionGetAverage(WMFeatureType* average_out)
{
    return kWMSessionResultOK;
}
