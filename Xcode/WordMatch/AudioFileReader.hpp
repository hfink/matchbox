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

#ifndef WORD_MATCH_AUDIO_FILE_READER_HPP
#define WORD_MATCH_AUDIO_FILE_READER_HPP

#include <string>

#include <boost/utility.hpp>
#include <boost/scoped_array.hpp>

#include "Types.h"

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>

namespace WM {
    
    /**
     * A minimalistic interface to read a mono audio channel with a sampling
     * rate of 16kHZ from a file. By using Apple's File Converter services
     * the actual audio file might be stored in many formats.
     */
    class AudioFileReader : boost::noncopyable {

    public:
        
        AudioFileReader(CFURLRef url);
        ~AudioFileReader();
        
        /**
         * @param num_samples On input defines how many samples to fetch from
         * the audio file. On output defines how many samples where actually
         * retrieved and how far the internal pointer within the audio file
         * has been advanced.
         * @param data The output data that the retrieved audio samples will be
         * written to. Note that the caller must be responsible for allocating 
         * at least num_samples size of SampleData. If less samples than
         * actually required were fetched, the data array will be null-padded.
         * @param time_offset Defines an offset in seconds from which position 
         * in the file to start reading the requested number of samples. If set
         * to -1, we just continue reading from the currently set position (each
         * read forward the internal pointer with the number of read samples).
         * @return If the read operation was successful (i.e. no error was 
         * encountered.)
         *
         * Note that this method will not throw when an error is encountered. 
         * This is a design decision since this method is possibly very often
         * and possibly throw/catch clauses might be too expensive in here.
         */
        bool read_floats(size_t& num_samples, 
                         WMAudioSampleType * data, 
                         float time_offset = -1.0f) const;
        
        /**
         * @return The duration of the file in seconds.
         */
        float duration() const;

        /**
         * Moves the reader pointer back to the beginning of the audio file.
         * The next call to read_floats without an time_offset will return 
         * exactly the first num_samples specified frames then.
         */
        void reset();
        
        /**
         * This method collects useful data about the file loaded. This includes
         * peak information, and begin/end timing for a given threshold. Note
         * that this method does NOT change the actual file, it just collects
         * information.
         * Decibel parameters are given in terms of gain, i.e. max. 0db, min. 
         * -96.0 db.
         * @param begin_threshold_db The threshold in DB at which the begin time
         * should be set.
         * @param end_threshold_db The threshold in DB at which the end time
         * should be set.
         * @param normalized_amplitude The amplitude from 0 to 1 for which the
         * the derived normalization factor should be calculated.
         */
        WMAudioFilePreProcessInfo preprocess(float begin_threshold_db,
                                             float end_threshold_db,
                                             float normalized_amplitude);
        
        /**
         * Convert decibel to amplitude level. This assume a reference level
         * at 0 db for an amplitude of 1 and -inf for an amplitude of 0.
         */
        static float decibel_to_amplitude(float db) {
            return powf(10, db*0.05f);
        }
        
        /**
         * Converts amplitude level to decibel where an amplitude of 1 
         * relates to a decibel value of 0, and amplitude 0 related to -inf.
         */
        static float amplitude_to_decibe(float a) {
            return log10f(fabsf(a))*20;
        }
        
    private:
        
        CFURLRef url_;
        std::string url_string_;
        
        AudioStreamBasicDescription client_format_;
        AudioStreamBasicDescription file_format_;
        
        Float64 file_client_sampling_ratio_;
        
        float duration_;
        ExtAudioFileRef ext_af_ref_;
        
        typedef boost::scoped_array<WMAudioSampleType> AudioProcessBuffer;
        
    };
    
}

#endif //WORD_MATCH_AUDIO_FILE_READER_HPP