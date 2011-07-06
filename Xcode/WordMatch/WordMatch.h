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

#ifndef WORD_MATCH_H
#define WORD_MATCH_H

// Umbrella header of WordMatch library. This header file defines a simplistic
// API for clients using a MFCC DTW implementation in C. It also defines a few
// types that are relevant to both clients our internal implementation.

#include "Types.h"

#include "WordMatchSession.h"

#include <CoreAudio/CoreAudioTypes.h>

/**
 * Returns the minimum distance for two audio files.
 * Use WMGetPreProcessInfoFor for both files before, and then pass
 * the results to file_a_info, and file_b_info respectively.
 */
bool WMGetMinDistanceForFile(CFURLRef file_a,
                             CFURLRef file_b,
                             WMFeatureType* distance,
                             WMAudioFilePreProcessInfo* file_a_info,
                             WMAudioFilePreProcessInfo* file_b_info);

/**
 * Processes a given file, and returns a structure on output that contains
 * additional data about the recorded sound.
 */
bool WMGetPreProcessInfoForFile(CFURLRef file, 
                                float begin_threshold_db,
                                float end_threshold_db,
                                WMAudioFilePreProcessInfo* info_out);

/**
 * Convenience functions to convert mach_time to milliseconds and vice-versa
 */

uint64_t WMMilliSecondsToMachTime(double ms);

double WMMachTimeToMilliSeconds(uint64_t mach);

/**
 * Prints out a description of  AudioStreamBasicDescription to stdout 
 * for debugging purposes.
 */
void WMPrintAudioStreamBasicDescription(const AudioStreamBasicDescription* desc);

#endif //WORD_MATCH_H