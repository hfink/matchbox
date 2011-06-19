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

#include "MFCCUtils.h"

#include <cassert>
#include "MFCCProcessor.hpp"
#include "dtw.hpp"
#include "DebugUtils.h"

/**
 * Processes a complete file and returns a vector of MFCC features for DTW.
 */
FeatureTypeDTW::Features get_mfcc_features(const boost::shared_ptr<WM::AudioFileReader>& reader, 
                                           WMAudioFilePreProcessInfo* reader_info) 
{

    FeatureTypeDTW::Features mfcc_features;
    
    //This seems to be a pretty robust configuration, and is the same
    //as we used in our Matlab prototype.
    
    static const size_t window_frame_size = 400;
    static const Float64 sample_rate = 16000.0f;
    static const float interval_time_duration = 0.01f;
    static const float preemphasis_coefficient = 0.97f;
    static const float min_frequency = 133.33f;
    static const float max_frequency = 6855.6f;

    static const float normalized_amplitude = 0.9f;
    
    WMAudioSampleType data[window_frame_size];
    
    WM::MFCCProcessor mp(window_frame_size, 
                         preemphasis_coefficient, 
                         (float)sample_rate, 
                         min_frequency, 
                         max_frequency);
    
    std::fill(&data[0], &data[window_frame_size], 0);    
    
    static const size_t interval_frame_size = interval_time_duration * (size_t)sample_rate;
    static const float window_time_duration = window_frame_size / (float)sample_rate;
    static const int overlap_frame_size = window_frame_size - interval_frame_size;
    
    //sanity check
    assert(window_frame_size > overlap_frame_size);
    
    WMAudioFilePreProcessInfo info;
    
    //Get trimming and normalization info
    if (reader_info == NULL) {
        
        std::cout << "Warning: no pre process info was given, calculating it "
                  << "using default values (-27db, -40db)." << std::endl;
        
        info = reader->preprocess(-27,
                                  -40,
                                  normalized_amplitude);
        
    } else {
        info = *reader_info;
    }
    
    float duration = info.threshold_end_time - info.threshold_start_time;
    if (duration <= 0) {
        std::cout << "Error: thresholding yields negative duration." << std::endl;
        return mfcc_features;
    }
    
    //Ensure that we are always reading full packages
    if ((reader->duration() - info.threshold_end_time) < window_time_duration)
        duration -= window_time_duration;
    
    //Notice that we have to make sure that all packets have useful information, 
    //therefore we clip the last incomplete package to read.
    
    size_t num_packets = (size_t)(duration / interval_time_duration);    
    
    //Tmp buffer, as we want only the first few MFCC components...
    WM::MFCCProcessor::CepstraBuffer cepstra;
    cepstra.assign(0);
    
    //Memorize the element which is going to be just left of the next
    //packet to read. This is used by the preemphasis filter which otherwise
    //would generate repeated spikes in the time-domain (as sample[0-1] would
    //be zero for each packet).
    float left_of_packet = .0f;
    
    //Let's find the maximum amplitude in order to normalize the following
    //processes
    
    for (int iPacket = 0; iPacket < num_packets; ++iPacket) {
        
        size_t num_samples = window_frame_size;

        std::fill(&data[0], &data[window_frame_size], 0);

        float iteration_time = info.threshold_start_time + iPacket * interval_time_duration;

        bool success = reader->read_floats(num_samples, data, iteration_time);
        
        if (num_samples != window_frame_size) {
            std::cout << "Warning: could not retrieve a full package of samples.";
            std::cout << std::endl;
            continue;
        }
        
        //scale to normalize
        vDSP_vsmul(data, 1, 
                   &info.normalization_factor, 
                   data, 1, num_samples);

        assert(success);
        
        mp.process(data,
                   left_of_packet,
                   &cepstra);


        //copy MFCC's 2th to 8th as our features (as in Matlab prototype)
        FeatureTypeDTW::FeatureVector mfcc_vector;
        const size_t offset = 1;
        std::copy(&cepstra[offset], 
                  &cepstra[offset + FeatureTypeDTW::feature_number_size], 
                  mfcc_vector.begin());

        mfcc_features.push_back(mfcc_vector);

        left_of_packet = data[num_samples - overlap_frame_size - 1];
    }
    
    return mfcc_features;
}


