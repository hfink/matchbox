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

#include "Types.h"
#include "AudioFileReader.hpp"
#include "MFCCProcessor.hpp"
#include "MFCCUtils.h"
#include "CAHostTimeBase.h"
#include "CAStreamBasicDescription.h"

#include <CoreAudio/CoreAudioTypes.h>

#include <stdexcept>
#include <iostream>

extern "C" bool WMGetMinDistanceForFile(CFURLRef file_a,
                                        CFURLRef file_b,
                                        WMFeatureType* min_distance,
                                        WMAudioFilePreProcessInfo* file_a_info,
                                        WMAudioFilePreProcessInfo* file_b_info)
{
    
    if (min_distance == NULL)
        return false;
    
    try {
      
        AudioFileReaderRef reader_a(new WM::AudioFileReader(file_a));
        AudioFileReaderRef reader_b(new WM::AudioFileReader(file_b));        
        
        FeatureTypeDTW::Features mfcc_features_a = get_mfcc_features(reader_a, 
                                                                     file_a_info);
        
        FeatureTypeDTW::Features mfcc_features_b = get_mfcc_features(reader_b, 
                                                                     file_b_info);        
        
        FeatureTypeDTW dtw(mfcc_features_a, mfcc_features_b, 20);    
        
        *min_distance = dtw.minimum_distance();
  
    } catch (const std::exception& e) {
        
        std::cerr << "Exception during MFCC extraction: " << e.what() 
                  << std::endl;
        
        return false;
        
    } catch (...) {
        //Note: a C++ exception must not leave the C/C++ realm. Objective-C
        //exception model is completely different.
        std::cerr << "Unknown exception caught." << std::endl;
        
        return false;
    }
    
    return true;
    
}

extern "C" bool WMGetPreProcessInfoForFile(CFURLRef file, 
                                float begin_threshold_db,
                                float end_threshold_db,
                                WMAudioFilePreProcessInfo* info_out)
{
    
    try {
        
        AudioFileReaderRef reader(new WM::AudioFileReader(file)); 
        
        WMAudioFilePreProcessInfo info = reader->preprocess(begin_threshold_db, 
                                                            end_threshold_db, 
                                                            1.0f);
        
        if (info_out != NULL) {
            *info_out = info;
        } else {
            throw std::invalid_argument("info_out is null.");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during Pre-Processing: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exceptino during Pre-Processing." << std::endl;
        return false;
    }
    
    return true;
    
}

extern "C" uint64_t WMMilliSecondsToMachTime(double ms) {
	//convert to nanoseconds
	double dNs = ms * 1e6;
	uint64_t uintNs = static_cast<uint64_t >( dNs );
	
	uint64_t machTime = CAHostTimeBase::ConvertFromNanos(uintNs);
	return machTime;
}

extern "C" double WMMachTimeToMilliSeconds(uint64_t mach) {
	uint64_t nsecUint = CAHostTimeBase::ConvertToNanos(mach);
	double result = static_cast<double >( nsecUint ) * 1e-6;
	return result;
}

extern "C" void WMPrintAudioStreamBasicDescription(const AudioStreamBasicDescription* desc)
{
    CAStreamBasicDescription desc_obj(*desc);
    desc_obj.Print();
}

