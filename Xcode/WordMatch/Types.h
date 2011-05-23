//Copyright (c) 2011 Sebastian Böhm sebastian@sometimesfood.org
//                   Heinrich Fink hf@hfink.eu
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

#ifndef WORD_MATCH_TYPES_H
#define WORD_MATCH_TYPES_H

typedef float WMAudioSampleType;
typedef float WMFeatureType;

/**
 * This struct holds useful information about a speech sample audio file. It 
 * is used by the internal calculation for the MFCC dynamic-time-warping
 * techniques where it is important to have matching begin and end locations.
 * For example, even if a word is uttered at different speed and pitch. The
 * begin and end points should mark the exact beginning and end of the spoken 
 * word.
 *
 * max_peak : The maximum peak in amplitude values from 0 .. 1 of the sample.
 * normalization_factor : Use this factor to normalize any given value of the
 * file (mostly used internally).
 * threshold_start_time : The identified begin-time of the spoken word within
 * audio file. 
 * threshold_end_time : The idenfitied end-tim of the spoken word within the
 * audio file.
 */
typedef struct WMAudioFilePreProcessInfo {
    
    float max_peak;
    float normalization_factor;
    float threshold_start_time;
    float threshold_end_time;    
    
} WMAudioFilePreProcessInfo;


/**
 * Describe basic configurtion parameters for a MFCC extraction. In future we
 * could add num mel bands, and num mfccs here as well.
 */
struct WMMfccConfiguration {
    Float64 sampling_rate;
    size_t window_size;
    float pre_empha_alpha;
    float mel_min_freq;
    float mel_max_freq;
};

typedef struct WMMfccConfiguration WMMfccConfiguration;


enum {
    kWMSessionResultOK = 0,
    kWMSessionResultErrorGeneric = 1,
    kWMSessionResultErrorInvalidWindowSize = 2,
    kWMSessionResultErrorInvalidArgument = 3
};

typedef SInt16 WMSessionResult;

typedef struct opaqueWMSession* WMSessionRef;

#endif //WORD_MATCH_TYPES_H