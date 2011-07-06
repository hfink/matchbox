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

#include "Types.h"
#include <CoreMedia/CMSampleBuffer.h>
#include "MFCCProcessor.hpp"
#include "CAStreamBasicDescription.h"
#include <stdexcept>
#include <algorithm>
#include <Accelerate/Accelerate.h>

struct opaqueWMSession {
    
    WMMfccConfiguration mfcc_configuration;
    float session_duration;
    //Note, this is the data required by the test, where we assume that we need
    //to save the MFCC results for the whole duration in one C array
    float* mfcc_data;
    size_t num_of_features_expected;
    size_t num_of_features_set;
    size_t num_read_samples;
    WM::MFCCProcessor* mfcc_processor;
    float *overlap_buffer;
    float preemph_border;
    size_t num_of_overlap_samples;
    AudioBufferList* non_interleaved_stereo_buffer;
    // of each CMSampleBuffer in this session, has to stay the same    
    size_t max_incoming_frames_per_buffer;
    
    opaqueWMSession() : session_duration(0), 
                        mfcc_data(NULL),
                        num_of_features_expected(0),
                        num_of_features_set(0),
                        num_read_samples(0),
                        mfcc_processor(NULL),
                        overlap_buffer(NULL),
                        preemph_border(0),
                        num_of_overlap_samples(0),
                        non_interleaved_stereo_buffer(NULL), 
                        max_incoming_frames_per_buffer(0)
    {}
    
};

static const size_t kWMSessionNumberOfMFCCs = WM::MFCCProcessor::kNumMelCepstra();

void cleanup_session(opaqueWMSession* session) {
    
    if (session->mfcc_data != NULL) {
        delete[] session->mfcc_data;
        session->mfcc_data = NULL;
    }
    
    if (session->mfcc_processor != NULL) {
        delete session->mfcc_processor;
        session->mfcc_processor = NULL;
    }
    
    if (session->non_interleaved_stereo_buffer != NULL) {
        free(session->non_interleaved_stereo_buffer->mBuffers[0].mData);
        free(session->non_interleaved_stereo_buffer->mBuffers[1].mData); 
        free(session->non_interleaved_stereo_buffer);
        session->non_interleaved_stereo_buffer = NULL;
    }
    
    if (session->overlap_buffer != NULL) {
        delete[] session->overlap_buffer;
        session->overlap_buffer = NULL;
    }
    
    delete session;
}

extern "C" WMSessionResult WMSessionCreate(float session_duration,
                                           WMMfccConfiguration mfcc_configuration,
                                           WMSessionRef* session_out)
{
    if (session_duration <= 0)
        return kWMSessionResultErrorInvalidArgument;
    
    if (mfcc_configuration.window_size % 2 != 0) {
        std::cerr << "Window size '" << mfcc_configuration.window_size 
                  << "' is invalid (not dividable by 2)." << std::endl;
        return kWMSessionResultErrorInvalidWindowSize;
    }
    
    opaqueWMSession* new_session = NULL;
    
    try {
        
        new_session = new opaqueWMSession();
        new_session->session_duration = session_duration;
        new_session->mfcc_configuration = mfcc_configuration;        
        new_session->mfcc_processor = 
            new WM::MFCCProcessor(mfcc_configuration.window_size,
                                  mfcc_configuration.pre_empha_alpha,
                                  (int)mfcc_configuration.sampling_rate,
                                  mfcc_configuration.mel_min_freq,
                                  mfcc_configuration.mel_max_freq);
        
        // Let's calculate how often we could extract MFCC with a hopsize 
        // of wnd_size*0.5f
        size_t total_num_samples_read = 
            session_duration * mfcc_configuration.sampling_rate;
        
        new_session->num_of_features_expected = 
            (total_num_samples_read / mfcc_configuration.window_size) * 2 - 1;
        
        size_t hop_size = mfcc_configuration.window_size / 2;
        
        size_t num_mfcc_values_expected = 
            new_session->num_of_features_expected*kWMSessionNumberOfMFCCs;
        
        new_session->mfcc_data = new float[num_mfcc_values_expected];
        
        vDSP_vclr(new_session->mfcc_data, 
                  1, 
                  num_mfcc_values_expected);
        
        // Note that the upper-bound size of hop_size * 3 results from the worst 
        // case scenario for the overlap where would have to use the tmp buffer 
        // to cover overlap mfcc extraction for two times. That of course is
        // only guaranteed if hop_size is strictly 
        // window_size / 2 (which we enforce)
        
        new_session->overlap_buffer = new float[hop_size * 3];
        
        vDSP_vclr(new_session->overlap_buffer, 1, mfcc_configuration.window_size);
        
    } catch (const std::exception& e) {
        
        std::cerr << "Error: WMSessionCreate: " << e.what() << std::endl;
        
        if (new_session != NULL)
            cleanup_session(new_session);
            
        return kWMSessionResultErrorGeneric;
    }
    
    *session_out = new_session;
    
    return kWMSessionResultOK;
}

extern "C" WMSessionResult WMSessionDestroy(WMSessionRef session)
{
    
    if (session == NULL)
        return kWMSessionResultErrorInvalidArgument;
    
    cleanup_session(session);
    
    return kWMSessionResultOK;
}

extern "C" WMSessionResult WMSessionReset(WMSessionRef session)
{
    
    if (session == NULL)
        return kWMSessionResultErrorInvalidArgument;    
    
    //We only reset the internal counters.
    session->num_read_samples = 0;
    session->num_of_features_set = 0;
    session->num_of_overlap_samples = 0;
    session->preemph_border = 0;
    
    return kWMSessionResultOK;
}

void copy_mfcc_and_advance(const WM::MFCCProcessor::CepstraBuffer& mfcc_data,
                           WMSessionRef session) 
{
 
    if (session->num_of_features_set < session->num_of_features_expected) {
        memcpy(&session->mfcc_data[session->num_of_features_set*kWMSessionNumberOfMFCCs], 
               mfcc_data.data(), 
               sizeof(float)*kWMSessionNumberOfMFCCs);
        session->num_of_features_set++;
    } else {
        std::cerr << "Error: feature tmp buffer overflow!" << std::endl;
    }
    
}

extern "C" bool WMSessionIsCompleted(WMSessionRef session)
{
    if (session == NULL)
        return false;
    
    return (session->num_of_features_set == session->num_of_features_expected);
}

extern "C" WMSessionResult WMSessionFeedFromSampleBuffer(CMSampleBufferRef sample_buffer, 
                                                         WMSessionRef session)
{
    if (session == NULL)
        return kWMSessionResultErrorInvalidArgument;   
    
    if (WMSessionIsCompleted(session)) {
        std::cerr << "WMSessionFeedFromSampleBuffer was called while " 
                  << "session was already completed." << std::endl;
        return kWMSessionResultErrorGeneric;
    }
    
    WMSessionResult return_code = kWMSessionResultOK;
    
    Boolean is_data_ready = CMSampleBufferDataIsReady(sample_buffer);
    
    if (!is_data_ready) {
        std::cerr << "Error: Data in sample_buffer wasn't ready to read." 
                  << std::endl;
        return kWMSessionResultErrorGeneric;
    }
    
    //Check the format description
    CMFormatDescriptionRef format_desc = 
        CMSampleBufferGetFormatDescription(sample_buffer);
    
    const AudioStreamBasicDescription* asbd = 
        CMAudioFormatDescriptionGetStreamBasicDescription(format_desc);
    
    CAStreamBasicDescription ca_asbd(*asbd);
    
    if (asbd->mSampleRate != session->mfcc_configuration.sampling_rate) {
        std::cerr << "Error: Configured sampling rate '" 
                  << session->mfcc_configuration.sampling_rate
                  << "' did not match incoming sample buffer's sampling rate '"
                  << asbd->mSampleRate << "'>" << std::endl;
        return kWMSessionResultErrorGeneric;
    }
    
    //Note that this actually is the numbers of frames, i.e. one frame could 
    //consist of several samples for each channel
    
    CMItemCount count_samples = CMSampleBufferGetNumSamples(sample_buffer);
    
    size_t frame_count = (size_t)count_samples;    
    
    //sanity check. Our algorithm requires that the frame_count is at least
    //size of the window
    if (frame_count < session->mfcc_configuration.window_size) {
        std::cout << "Warning: Incoming number of frames '" << frame_count 
                  << "' is smaller than the window size Skipping feature "
                  << "calculation for this packet." << std::endl;
        //TODO: check is we should tread this as an error
        return kWMSessionResultOK;
    }
    
    AudioBufferList abl;
    CMBlockBufferRef audio_data;
    
    OSStatus result = CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(sample_buffer, 
                                                                              NULL, 
                                                                              &abl, 
                                                                              sizeof(AudioBufferList), 
                                                                              NULL, 
                                                                              NULL, 
                                                                              kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, 
                                                                              &audio_data);
    
    if (result != noErr) {
        std::cerr << "Error: Operation to fetch audio" 
                  << " from CMSampleBuffer failed: " << result << std::endl;
        return kWMSessionResultErrorGeneric;
    }
    
    size_t window_size = session->mfcc_configuration.window_size;    
    size_t hop_size = window_size / 2;
    
    
    //Note: in here we could perform manual mixing of left/right channels, 
    //if Apple's APIs don't support it (mixing it by ourselves
    //might be the best case anyway...)
    const float* noninterleaved_data = NULL;
    
    if (ca_asbd.IsInterleaved() && (asbd->mChannelsPerFrame == 2)) {
        
        //Delayed allocation of the buffer for de-interleaving the incoming 
        //interleaved streams. Note that we assume the buffer size to be 
        //constant for each feed. This will be checked. If this assumption is 
        //not met, we'll return an error
        if (session->non_interleaved_stereo_buffer == NULL) {
            
            session->non_interleaved_stereo_buffer = 
                (AudioBufferList*)malloc(sizeof(AudioBufferList) + sizeof(AudioBuffer));
            
            session->non_interleaved_stereo_buffer->mNumberBuffers = 2;
            size_t byte_size = sizeof(float)*frame_count;
            
            session->non_interleaved_stereo_buffer->mBuffers[0].mData = malloc(byte_size);
            session->non_interleaved_stereo_buffer->mBuffers[0].mDataByteSize = byte_size;
            session->non_interleaved_stereo_buffer->mBuffers[0].mNumberChannels = 1;
            
            session->non_interleaved_stereo_buffer->mBuffers[1].mData = malloc(byte_size);
            session->non_interleaved_stereo_buffer->mBuffers[1].mDataByteSize = byte_size;
            session->non_interleaved_stereo_buffer->mBuffers[1].mNumberChannels = 1;
            
            session->max_incoming_frames_per_buffer = frame_count;
        }
        
        if (session->max_incoming_frames_per_buffer < frame_count) {
            
            std::cerr << "Error: Frame count too large during buffer feed. "
                      << "Original: "
                      << session->max_incoming_frames_per_buffer << " vs now: "
                      << frame_count << "." << std::endl;
            
            return_code = kWMSessionResultErrorGeneric;
            
        } else {
        
            //At first we'll have to de-interleave the incoming samples. We'll
            //do this with a little help from the vDSP framework.
            DSPSplitComplex noninterleaved_proxy;
            
            noninterleaved_proxy.realp = 
                (float*)session->non_interleaved_stereo_buffer->mBuffers[0].mData;
            
            noninterleaved_proxy.imagp = 
                (float*)session->non_interleaved_stereo_buffer->mBuffers[1].mData;            
            
            //De-interleaving, note that realp contains one channel, and imagp
            //the other after this operation
            vDSP_ctoz((DSPComplex*)abl.mBuffers[0].mData, 
                      2,
                      &noninterleaved_proxy, 
                      1, 
                      frame_count);            
            
            noninterleaved_data = noninterleaved_proxy.realp;
        }
        
    } else if (asbd->mChannelsPerFrame == 1) {
        noninterleaved_data = (const float*)abl.mBuffers[0].mData;
    } else {
        std::cout << "Unsupported stream format." << std::endl;
        return_code = kWMSessionResultErrorGeneric;        
    }
            
    if (noninterleaved_data != NULL) {
            
        //This is an offset into the direct buffer access, depending on the
        //previous buffer overlap configuration
        size_t direct_buffer_access_offset = 0;
        
        WM::MFCCProcessor::CepstraBuffer mfcc_results;
        
        //If we have an overlap left from the previous buffer read opertion
        //we'll have to take care of that. To "knit" together the ends
        //we use the overlap_buffer provided by the session.
        if (session->num_of_overlap_samples != 0) {
            
            //copy samples from the new buffer into the rest of the tmp
            //overlap buffer. The upperbound size of hop_size*3 is derived
            //from the worst scenario where we might have to executed 
            //MFCC extraction twice on this tmp buffer.
            memcpy(&session->overlap_buffer[session->num_of_overlap_samples], 
                   noninterleaved_data, 
                   sizeof(float)*(hop_size*3 - session->num_of_overlap_samples));
            
            //perform MFCC extraction
            session->mfcc_processor->process(&session->overlap_buffer[0], 
                                             session->preemph_border, 
                                             &mfcc_results);
            
            copy_mfcc_and_advance(mfcc_results, session);
            
            session->preemph_border = session->overlap_buffer[hop_size - 1];
            
            //Since we can assume that hopsize is exactly window_size/2, 
            //there could be only one execution left were we would have
            //to use overlap data
            if ( (session->num_of_overlap_samples > hop_size) && 
                 !WMSessionIsCompleted(session) ) {
            
                //we have already enough data in the tmp buffer from the
                //previous copy operation
                session->mfcc_processor->process(&session->overlap_buffer[hop_size], 
                                                 session->preemph_border, 
                                                 &mfcc_results);
                
                copy_mfcc_and_advance(mfcc_results, session);
                
                session->preemph_border = session->overlap_buffer[hop_size*2 - 1];                    
                
                direct_buffer_access_offset = hop_size*2 - session->num_of_overlap_samples;                    
                
            } else {
                
                //adapt offset accordingly
                direct_buffer_access_offset = hop_size - session->num_of_overlap_samples;
                
            }
            
        }
        
        // We are done dealing with a possible overlap from the previous
        // feed operation. We will now access the de-interleaved data 
        // directly, without copying back and forth. We will stop when
        // we don't have enough samples left to execute a full MFCC
        // extraction, or when the session has gathered enough feature data
        // (when it is complete).
        
        size_t direct_frame_access = direct_buffer_access_offset;
        
        while (((direct_frame_access + window_size) <= frame_count) && 
               !WMSessionIsCompleted(session)) 
        {
            
            session->mfcc_processor->process(&noninterleaved_data[direct_frame_access], 
                                             session->preemph_border, 
                                             &mfcc_results);
            
            copy_mfcc_and_advance(mfcc_results, session);
            
            session->preemph_border = noninterleaved_data[direct_frame_access + hop_size - 1];                               
            
            direct_frame_access += hop_size;
        }
        
        //There will always be some samples left as we advance by hop_size,
        //but require that window_size must be available.
        if (direct_frame_access < (frame_count-1)) {
            
            //Copying the rest that wasn't processed into the overlap
            //buffer.
            size_t num_overlap_samples = (frame_count-1) - direct_frame_access;
            session->num_of_overlap_samples = num_overlap_samples;
            memcpy(session->overlap_buffer, 
                   &noninterleaved_data[direct_frame_access], 
                   num_overlap_samples*sizeof(float));
            
        } else {
            //That's an incorrect state. Return an error.
            std::cerr << "Reached an impossible state, where we have "
                      << " consumed all samples, even with a hop_size set." 
                      << std::endl;
            CFRelease(audio_data);
            return kWMSessionResultErrorGeneric;
        }
    }
    
    CFRelease(audio_data);
    
    session->num_read_samples += (int)count_samples;
    
    return return_code;
}

extern "C" WMSessionResult WMSessionGetAverage(WMFeatureType* average_out, 
                                               WMSessionRef session)
{
    
    if (session == NULL)
        return kWMSessionResultErrorInvalidArgument;
    
    if (!WMSessionIsCompleted(session)) {
        
        //issue this warning only, if diff is larger than one
        //a diff of one can easily happen due to numerical issue with downsampling
        if (session->num_of_features_expected - session->num_of_features_set > 1) {
            std::cout << "Warning: Calculation of features average was requested "
                      << "but session is not completed yet. " << std::endl;
        }
    }
    
    //Note that this approach is rather a performance test case than the best
    //way to calculate the average over all MFCC extracted features.
    //A better approach would be to keep book of a running sum after each
    //extraction and to divide after the last extraction. This wouldn't require
    //this extra amount of memory. However, other accumulation algorithms might
    //require the complete set of MFCC data per session to be available.
    
    //TODO: is there a CBLAS command that only sums up, without multiplying?
    std::fill(&average_out[0], &average_out[kWMSessionNumberOfMFCCs], 0);
    
    
    //Calculating mean per component
    for (size_t i = 0; i<kWMSessionNumberOfMFCCs; ++i) {
        vDSP_meanv(&session->mfcc_data[i], 
                   kWMSessionNumberOfMFCCs, 
                   &average_out[i], 
                   session->num_of_features_set);
    }
    
    return kWMSessionResultOK;
}
