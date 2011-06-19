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

#include <boost/test/unit_test.hpp>
#include <boost/scoped_array.hpp>

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "MFCCProcessor.hpp"
#include "AudioFileReader.hpp"

#include <iostream>

typedef boost::shared_ptr<WM::AudioFileReader> AudioFileReaderRef;

typedef boost::scoped_array<float> FloatScopedArray;

typedef boost::scoped_array<WM::MFCCProcessor::CepstraBuffer> CepstraBufferArrayRef;

//Note: Actually this wouldn't be necessary, as this header is included in the
//prefix header. However, obviously LLVM on-the-fly correction has some troubles
//still with precompiled headers...
//#include "MFCCProcessor_Test_Data.h"

BOOST_AUTO_TEST_SUITE( MFCCProcessorTest )

using namespace WM;

struct PulloverReaderFixture {
    
    PulloverReaderFixture() {
        CFStringRef file_string = CFSTR("02-pullover-3.wav");
        CFURLRef url = NULL;
        
#ifdef TEST_USE_MAIN_BUNDLE_FOR_FILES
        
        CFBundleRef main_bundle;
        
        // Get the main bundle for the app
        main_bundle = CFBundleGetMainBundle();  
        
        if (main_bundle == NULL)
            throw std::runtime_error("Cannot load main bundle");
        
        url = CFBundleCopyResourceURL(main_bundle, file_string, NULL, NULL);
        if (url == NULL)
            throw std::runtime_error("Cannot load file from bundle.");
        
#else
        
        url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
                                            file_string,
                                            kCFURLPOSIXPathStyle, 
                                            false);
        
#endif
        
        BOOST_REQUIRE_NO_THROW (
                                reader = AudioFileReaderRef(new AudioFileReader(url))
                                );
        
        CFRelease(url);
        
	}
    
    AudioFileReaderRef reader;
};

BOOST_AUTO_TEST_CASE(SanityCheck) {
    
    BOOST_REQUIRE_THROW(MFCCProcessor mp(0, 0, 0, 0, 0), std::invalid_argument);
    
    BOOST_REQUIRE_THROW(MFCCProcessor mp(400, 0.97, 16000, 400.0f, 100.0f), 
                        std::invalid_argument);
    
    BOOST_REQUIRE_THROW(MFCCProcessor mp(0, 0.97, 16000, 100.0f, 400.0f), 
                        std::invalid_argument);    
    
    BOOST_REQUIRE_THROW(MFCCProcessor mp(400, 0, 16000, 100.0f, 400.0f), 
                        std::invalid_argument);        
    
    BOOST_REQUIRE_THROW(MFCCProcessor mp(400, 0.97, 0, 100.0f, 400.0f), 
                        std::invalid_argument);       
    
    BOOST_REQUIRE_THROW(MFCCProcessor mp(400, 1.3, 16000, 100.0f, 400.0f), 
                        std::invalid_argument);           
    
}

/**
 * Checks if the MFCC Processor calculates the same for two equal inputs.
 */
BOOST_AUTO_TEST_CASE( DeterministicBehaviour ) {
    
    const size_t test_sample_size = 400;
    WMAudioSampleType data[test_sample_size];
    
    //let's fill the audio with some overlaid sine's
    for (int i = 0; i<test_sample_size; ++i) {
        data[i] = sinf(5.0f*i / test_sample_size);
    }
    
    MFCCProcessor mp(400, 0.97f, 16000, 133.33f, 6855.6);
    
    MFCCProcessor::CepstraBuffer cepstra_1;
    cepstra_1.assign(0);
    
    MFCCProcessor::CepstraBuffer cepstra_2;
    cepstra_2.assign(0);
        
    mp.process(data, 0, &cepstra_1);
    
    //process again, and compare
    
    mp.process(data, 0, &cepstra_2);    
    
    for (int i = 0; i<13; ++i) {
        BOOST_REQUIRE_EQUAL(cepstra_1[i], cepstra_2[i]);
    }
}

//TODO: if I was more familar with boost::serialization, I would have used
//that instead.
void print_reference_array(const std::string& array_name, 
                           const float * array_data,
                           const size_t& num_packets,
                           const size_t& num_components) {

    std::cout << "static const float "
              << array_name << "[] = {";
       
    for (int iPkt = 0; iPkt<num_packets; ++iPkt) {    

       if (iPkt != 0)
           std::cout << ", " << std::endl;                    
       
       for (int iCmp = 0; iCmp<num_components; ++iCmp) {
           
           if ( (iCmp != 0) )
               std::cout << ", ";
           
           if (iCmp % 7 == 0)
               std::cout << std::endl;            
           
           std::cout << array_data[iPkt*num_components + iCmp];
       }
    }
    std::cout << std::endl << "};" << std::endl;    
}

/**
 * This test compares the output data with previous data where the MFCC was
 * confirmed yield the exact same results as reference implementations of
 * Matlab. These confirmed data have been hard-coded into 
 * MFCCProcessor_Test_data. Modifications of the MFCC algorithm might cause
 * subtle changes in the output data. However, if these subtle changes were
 * expected and confirmed to still yield correct results, the test data shall
 * be regenerated using the commented print functions of this test routine.
 */
BOOST_FIXTURE_TEST_CASE( OutputRegressionTest, PulloverReaderFixture ) {
    
    //The following variable are fixed parameters for the regression test
    
    static const size_t window_frame_size = 400;
    static const size_t num_cepstrum_components = 13;
    static const size_t num_mel_bands = 40;
    static const Float64 sample_rate = 16000.0f;
    static const float interval_time_duration = 0.01;
    static const float preemphasis_coefficient = 0.97f;
    static const float min_frequency = 133.33f;
    static const float max_frequency = 6855.6f;
    
    WMAudioSampleType data[window_frame_size];
    
    MFCCProcessor mp(window_frame_size, 
                     preemphasis_coefficient, 
                     sample_rate, 
                     min_frequency, 
                     max_frequency);
    
    std::fill(&data[0], &data[window_frame_size], 0);    
    
    static const size_t interval_frame_size = interval_time_duration * sample_rate;
    static const float window_time_duration = window_frame_size / sample_rate;
    static const int overlap_frame_size = window_frame_size - interval_frame_size;
    
    //sanity check
    BOOST_REQUIRE_GT(window_frame_size, overlap_frame_size);

    //Notice that we have to make sure that all packets have useful information, 
    //therefore we clip the last incomplete package to read.
    
    size_t num_packets = (size_t)((reader->duration() - window_time_duration) 
                                  / interval_time_duration);    
    
    CepstraBufferArrayRef cepstra(new MFCCProcessor::CepstraBuffer[num_packets]);
    
    MFCCProcessor::CepstraBuffer null_buffer;
    null_buffer.assign(0);
    
    std::fill(&cepstra[0], &cepstra[num_packets], null_buffer);    
    
    FloatScopedArray spectrum(new float[num_packets*mp.fft_size_half()]);
    std::fill(&spectrum[0], &spectrum[num_packets*mp.fft_size_half()], 0);  
    
    FloatScopedArray mel_spectrum(new float[num_packets*num_mel_bands]);
    std::fill(&mel_spectrum[0], &mel_spectrum[num_packets*num_mel_bands], 0);      
    
    //Memorize the element which is going to be just left of the next
    //packet to read. This is used by the preemphasis filter which otherwise
    //would generate repeated spikes in the time-domain (as sample[0-1] would
    //be zero for each packet).
    float left_of_packet = .0f;
    
    for (int iPacket = 0; iPacket < num_packets; ++iPacket) {
        
        size_t num_samples = window_frame_size;    
        
        std::fill(&data[0], &data[window_frame_size], 0);           
        
        float iteration_time = iPacket * interval_time_duration;                        
        
        bool success = reader->read_floats(num_samples, data, iteration_time);
        
        BOOST_REQUIRE_EQUAL(success, true);        
        
        mp.process(data, 
                   left_of_packet,
                   &(cepstra[iPacket]), 
                   &spectrum[iPacket*mp.fft_size_half()],
                   &mel_spectrum[iPacket*num_mel_bands]);       

        left_of_packet = data[num_samples - overlap_frame_size - 1];
    }
    
    // Compare output data with reference
    for (int iPkt = 0; iPkt < num_packets; ++iPkt) {
        for (int iCmp = 0; iCmp < num_cepstrum_components; ++iCmp) {
            
            size_t idx = iPkt * num_cepstrum_components + iCmp;
            
            double tolerance = 0.02; 
            
            if (iCmp > 0.75f * num_cepstrum_components)
                tolerance = 1;
            
            BOOST_CHECK_CLOSE(cepstra[iPkt][iCmp], 
                              WmRegressionData::reference_cepstra[idx],
                              tolerance);
            
        }
    }
    
    for (int iPkt = 0; iPkt < num_packets; ++iPkt) {
        for (int iCmp = 0; iCmp < mp.fft_size_half(); ++iCmp) {
            
            size_t idx = iPkt * mp.fft_size_half() + iCmp;
        
            //In the spectrum we have to be a little more tolerant
            //in the higher frequencies due to numerical instabilities
            double tolerance = 0.1;            
            
            if (iCmp > 0.75f*mp.fft_size_half())
                tolerance = 1;
       
            BOOST_CHECK_CLOSE(spectrum[idx], 
                              WmRegressionData::reference_pspectrum[idx],
                              tolerance);
        }  
    }
    
    for (int iPkt = 0; iPkt < num_packets; ++iPkt) {
        for (int iCmp = 0; iCmp < num_mel_bands; ++iCmp) {
            
            size_t idx = iPkt * num_mel_bands + iCmp;
            
            double tolerance = 0.2;
            
            //on higher bands we have to be a little more tolerant
            if (iCmp > 0.75f*num_mel_bands)
                tolerance = 1;
            
            //Our original reference spectrum was already log10, but we return
            //the unprocessed mel power spectrum... therefore we need to
            //compensate for that            
            BOOST_CHECK_CLOSE(mel_spectrum[idx], 
                              powf(10, WmRegressionData::reference_melspectrum[idx]),
                              tolerance);
        }
    }     
    

    // Uncomment to re-generate regression data. Just copypaste these into
    // MFCCProcessor_Test_Data.h
    
//    print_reference_array("reference_cepstra", 
//                          cepstra.get(), 
//                          num_packets, 
//                          num_cepstrum_components);
//    
//    print_reference_array("reference_pspectrum", 
//                          spectrum.get(), 
//                          num_packets, 
//                          mp.fft_size_half());
//    
//    print_reference_array("reference_melspectrum", 
//                          mel_spectrum.get(), 
//                          num_packets, 
//                          num_mel_bands);    
    

}

BOOST_AUTO_TEST_SUITE_END()