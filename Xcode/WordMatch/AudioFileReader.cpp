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

#include "AudioFileReader.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "CAXException.h"

//#include <CoreFoundation/CFURL.h>

using namespace WM;

AudioFileReader::AudioFileReader(CFURLRef url) :
    url_(NULL), duration_(0), ext_af_ref_(NULL)
{
    
    CFStringRef url_str = CFURLGetString(url);
    char buf[256];
    bool b = CFStringGetCString(url_str, buf, 256, kCFStringEncodingUTF8);
    
    url_string_ = b ? std::string(buf) : "unknown";
    
    //Check if the file exist
    
    //Note: out of mysterious reason, we don't have CFURLResourceIsReachable
    //on iOS available. We use a different approach to check for file existance
    //therefore.
    
    SInt32 errorCode = 0;
    
    CFTypeRef property = CFURLCreatePropertyFromResource(kCFAllocatorDefault, 
                                                         url, 
                                                         kCFURLFileExists, 
                                                         &errorCode);
    
    
    if (property == NULL) {
        //error code holds useful information, probably
        std::ostringstream oss;
        oss << "There has been error checking file availability. ErrorCode: '"
            << errorCode << "'." << std::endl;
        throw std::runtime_error(oss.str());
    }
    
    Boolean file_exists = CFBooleanGetValue((CFBooleanRef)property);
    
    if (!file_exists) {
        throw std::invalid_argument("File '" +url_string_+ "' does not exist.");
    }

    CFRelease(property);    
    
    UInt32 prop_size = 0;
	
	XThrowIfError( ExtAudioFileOpenURL(url, &ext_af_ref_),
                   "Opening audio file failed.");
    
	prop_size = sizeof(file_format_);
	memset(&file_format_, 0, sizeof(AudioStreamBasicDescription));
	
	XThrowIfError( ExtAudioFileGetProperty(ext_af_ref_, 
                                           kExtAudioFileProperty_FileDataFormat, 
                                           &prop_size, 
                                           &file_format_),
                  "Getting original file format ASBD");
    
	prop_size = sizeof(client_format_);
	memset(&client_format_, 0, sizeof(AudioStreamBasicDescription));
    
	client_format_.mFormatID = kAudioFormatLinearPCM;
	client_format_.mSampleRate = 16000;
	client_format_.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	client_format_.mChannelsPerFrame = 1;
	client_format_.mBitsPerChannel = 32; //single precision
	client_format_.mFramesPerPacket = 1;
	client_format_.mBytesPerPacket = client_format_.mBytesPerFrame  = 4;
	client_format_.mReserved = 0;
	
	XThrowIfError( ExtAudioFileSetProperty(ext_af_ref_,
                                           kExtAudioFileProperty_ClientDataFormat, 
                                           prop_size, 
                                           &client_format_), 
                   "Setting the client ASBD." );
    
    // getting the total numbers of frames
    // Note that the number of frames obviously refers to the original file
    // ASBD
    SInt64 num_frames = 0;
	prop_size = sizeof(SInt64);
	XThrowIfError( ExtAudioFileGetProperty(ext_af_ref_,
                                           kExtAudioFileProperty_FileLengthFrames, 
                                           &prop_size, 
                                           &num_frames),
                  "Retrieving total number of frame in file." );
    
    //store the duration in seconds
    duration_ = (float)num_frames / (float)file_format_.mSampleRate;
    
    CFRetain(url);
    url_ = url;
    
    file_client_sampling_ratio_ = file_format_.mSampleRate /
                                  client_format_.mSampleRate;    
}

AudioFileReader::~AudioFileReader() {
    
	OSStatus err = noErr;
	if (ext_af_ref_ != NULL) {
		err = ExtAudioFileDispose(ext_af_ref_);
        if (err != noErr) {
            std::cerr << "Fatal : Cannot properly close audio file "
                      << url_string_ << std::endl;
        }
        ext_af_ref_ = NULL;
	}
    
	if (url_ != NULL) {
        CFRelease(url_);    
        url_ = NULL;
	}    
    
}

bool AudioFileReader::read_floats(size_t& num_samples, 
                                  WMAudioSampleType * data,
                                  float time_offset) const {
    
    if (num_samples == 0)
        return true;
    
    if (data == NULL)
        return false;
    
    if (time_offset != -1.0f) {
        if (time_offset < 0) {
            return false;
        } else if ( time_offset > duration_) {
            return false;
        } else {
            
            //seek to the right position
            
            //convert time offset to original file format sample offset
            
            //Look for the corresponding frame in file_format
            SInt64 frame_nr_file = (SInt64)round(time_offset * file_format_.mSampleRate);
            
            OSStatus err = ExtAudioFileSeek(ext_af_ref_, frame_nr_file);
            if (err != noErr)
                return false;
            
        }
    } // else we continue with the next package
    
    AudioBufferList buf_list;
    buf_list.mNumberBuffers = 1;
    buf_list.mBuffers[0].mNumberChannels = 1;
    buf_list.mBuffers[0].mData = data;
    buf_list.mBuffers[0].mDataByteSize = (UInt32)(num_samples * sizeof(WMAudioSampleType));
    
    OSStatus err = noErr;
    UInt32 num_frames_arg = (UInt32)num_samples;
    
    err = ExtAudioFileRead(ext_af_ref_, &num_frames_arg, &buf_list);

    num_samples = num_frames_arg;
    
    if (err != noErr)
        return false;
        
    return true;
}

void AudioFileReader::reset() {
    OSStatus err = ExtAudioFileSeek(ext_af_ref_, 0);
    if (err != noErr)
        std::cout << "Error resetting audio file to first frame." << std::endl;
}

WMAudioFilePreProcessInfo AudioFileReader::preprocess(float begin_threshold_db,
                                                      float end_threshold_db,
                                                      float normalized_amplitude) 
{

    reset();    
    
    WMAudioFilePreProcessInfo info;
    memset(&info, 0, sizeof(WMAudioFilePreProcessInfo));    
    

    static const int packet_size = 1024;

    // we convert the threshold given in db to an amplitude power level
    float normalized_begin_threshold = decibel_to_amplitude(begin_threshold_db);
    float normalized_end_threshold = decibel_to_amplitude(end_threshold_db);    
    
    AudioProcessBuffer audio_data(new WMAudioSampleType[packet_size]);
    std::fill(&audio_data[0], &audio_data[packet_size], 0);
    
    AudioProcessBuffer process_data(new WMAudioSampleType[packet_size]);
    std::fill(&process_data[0], &process_data[packet_size], 0);    
    
    size_t samples_read = packet_size;
        
    // Calculate the maximum peak magnitude of the whole audio file
    while (samples_read > 0) {
        
        bool success = read_floats(samples_read, audio_data.get());
        if (!success) {
            std::cout << "Could not read packages during max-peak determination." << std::endl;
            return info;
        }
        
        if (samples_read > 0) {
            float max_in_packet = 0;
            vDSP_maxmgv(audio_data.get(), 1, &max_in_packet, samples_read);
            
            if (info.max_peak < max_in_packet)
                info.max_peak = max_in_packet;
        }
        
    }
    
    //scale factor in order to normalize
    info.normalization_factor = normalized_amplitude / info.max_peak;
    float normalization_factor_inv = 1.0f / info.normalization_factor;

    // Since we haven't processed the actual audio file, we convert the 
    // threshold of normalized audio data back to the euqivalent threshold
    // in the original unnormalized data.
    float unnormalized_begin_threshold = normalized_begin_threshold * normalization_factor_inv;
    float unnormalized_end_threshold = normalized_end_threshold * normalization_factor_inv;    
    
    //reset the reader position back to the beginning
    reset();
    
    size_t total_samples_read = 0;
    
    bool looking_for_beginning = true;
    
    //We are now performing the thresholding operation. We look for a begin and
    //end location in time corresponding to the first and second crossing of an 
    //average threshold per packet.
    
    for (samples_read = packet_size;
         samples_read > 0;
         total_samples_read += samples_read) 
    {
        
        bool success = read_floats(samples_read, audio_data.get());
        if (!success) {
            std::cout << "Could not read packages during thresholding operation." << std::endl;
            return info;
        }        
        
        float max_abs_packet = 0;
        vDSP_Length max_abs_packet_idx = 0;
        vDSP_maxmgvi(audio_data.get(), 
                     1, 
                     &max_abs_packet, 
                     &max_abs_packet_idx, 
                     samples_read);
        
        if ( looking_for_beginning && 
            (max_abs_packet >= unnormalized_begin_threshold))
        {
        
            //We are looking for the begin index
            
            // At first calculate absolute values of amplitudes
            vDSP_vabs(audio_data.get(), 1, process_data.get(), 1, samples_read);            
            
            float C = 1.0f;
            
            //Then perform a thresholding operation. Note that we are not 
            //looking for the max abs idx, but rather for the first sample that
            //passed the threshold
            
            vDSP_Length num_samples_to_check = max_abs_packet_idx+1;
            
            vDSP_vthrsc(process_data.get(), 
                        1, 
                        &unnormalized_begin_threshold, 
                        &C, 
                        process_data.get(), 
                        1,
                        num_samples_to_check);
            
            vDSP_Length threshold_pass_idx = 0;            
            
            //again look for the first max value, e.g. the first value that
            //is equal to 1
            float max_abs_thresh = 0;
            vDSP_maxvi(process_data.get(), 
                       1, 
                       &max_abs_thresh, 
                       &threshold_pass_idx, 
                       num_samples_to_check);
            
            //Look for the first zero crossing LEFT of the first thresholded 
            //sample
            
            //The first just counts the zero crossing left of our sample of 
            //interest
            vDSP_Length idx_of_last_crossing = 0;
            vDSP_Length total_number_crossings = 0;
            
            vDSP_Length num_samples_to_consider = threshold_pass_idx+1;
            
            vDSP_nzcros(audio_data.get(), 
                        1, 
                        (vDSP_Length)packet_size, 
                        &idx_of_last_crossing, 
                        &total_number_crossings, 
                        num_samples_to_consider);
                        
            // If there were no zero crossing left of the threshold indes, 
            // we use the threshold idx position as the left clipping position
            if (total_number_crossings == 0) {
                idx_of_last_crossing = threshold_pass_idx;
            } else {
                // We perform a  second pass, we look up the location of the 
                // _last_ occurring zero crossing left of our sample of interest
                vDSP_Length total_number_crossings__ = 0; 
                
                idx_of_last_crossing = 0;
                
                vDSP_nzcros(audio_data.get(), 
                            1, 
                            total_number_crossings, 
                            &idx_of_last_crossing, 
                            &total_number_crossings__, 
                            num_samples_to_consider);                

            }
            
            float time_location = 
                static_cast<float>((total_samples_read + idx_of_last_crossing)) / static_cast<float>(client_format_.mSampleRate);
            
            info.threshold_start_time = time_location;
            
            //switch mode
            looking_for_beginning = false;
            
        } else if ( !looking_for_beginning && 
                    (max_abs_packet <= unnormalized_end_threshold) )
        {

            // Looking for the end point. More specifically, this means that
            // we are looking for the first zero crossing right next to the
            // last maximum value that went over threshold. Therefore we must
            // look within the last read packet (which is temporarily stored
            // in the process_data buffer.
            
            
            //As opposed to the search for the begin index, we use audio_data
            //as the temporary buffer, and process_data as the buffer with
            //the original audio data.            
            
            //if this is the first packet, make sure we exit
            if (total_samples_read < packet_size) {
                break;
            }
            
            //copy into the tmp buffer
            std::copy(&process_data[0], 
                      &process_data[packet_size], 
                      audio_data.get());
            
            // Reverse the last tmp buffer since we 
            // are interested in the LAST occurence of the maximum, 
            // not the first one.
            vDSP_vrvrs(audio_data.get(), 1, packet_size);
            
            //Again, lookup up the maximum value, and retrieve the index
            float max_abs_previous_packet = 0;
            vDSP_Length max_abs_previous_packet_last_idx = 0;
            vDSP_maxmgvi(audio_data.get(), 
                         1, 
                         &max_abs_previous_packet, 
                         &max_abs_previous_packet_last_idx, 
                         packet_size);            
            
            // Calculate absolute values of amplitudes from the original packet
            // Note that previous slice always has to be a full packet, 
            // therefore we can safely access packet_size num of samples.
            
            vDSP_vabs(process_data.get(), 1, audio_data.get(), 1, packet_size);
            
            float C = -1.0f;
            
            vDSP_Length num_samples_to_check = packet_size - max_abs_previous_packet_last_idx;
            size_t offset = max_abs_previous_packet_last_idx;
            
            //CONTINUE IN HERE: something is not right with it...
            
            vDSP_vthrsc(&audio_data.get()[offset], 
                        1, 
                        &unnormalized_end_threshold, 
                        &C, 
                        &audio_data.get()[offset], 
                        1,
                        num_samples_to_check);            
            
            vDSP_Length threshold_pass_idx = 0;            
            
            //again look for the first max value, e.g. the first value that
            //is equal to 1
            float max_abs = 0;
            vDSP_maxvi(&audio_data.get()[offset], 
                       1, 
                       &max_abs, 
                       &threshold_pass_idx, 
                       num_samples_to_check);            
            
            // threshold_pass_idx now holds the index of the first sample
            // right next the last maximum value that is lower than the
            // specified end threshold
            
            // We are now looking for the FIRST zero crossing right next to
            // this location
            
            vDSP_Length idx_of_first_crossing = 0;            
            vDSP_Length total_number_crossings = 0;                        
            
            vDSP_nzcros(&process_data[offset], 
                        1, 
                        1, 
                        &idx_of_first_crossing, 
                        &total_number_crossings, 
                        num_samples_to_check);
            
            if (total_number_crossings == 0)
                idx_of_first_crossing = 0;
            
            size_t loc_samples = 
                    (total_samples_read - packet_size + idx_of_first_crossing + offset);
            
            float time_location = static_cast<float>(loc_samples) / static_cast<float>(client_format_.mSampleRate);
            
            info.threshold_end_time = time_location;
            break; //terminate, we've got all we want.
        }
        
        // copy the current amount of sample to our process buffer, 
        // the end point search might need it
        std::copy(&audio_data[0], 
                  &audio_data[samples_read], 
                  process_data.get());
        
    }
    
    //Reset the reading position of the player.
    reset();
    
    //We bias the values a bit to catch onset more accurately
//    info.threshold_start_time = std::max(0.0f, 
//                                         info.threshold_start_time-0.05f);
//    
//    info.threshold_end_time = std::max(duration(), 
//                                       info.threshold_start_time+0.05f);    
    
    return info;    
}

float AudioFileReader::duration() const {
    return duration_;
}