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

#include <string>
#include <stdexcept>
#include <algorithm>
#include <math.h>

#include "AudioFileReader.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#include <CoreFoundation/CoreFoundation.h>

typedef boost::shared_ptr<WM::AudioFileReader> AudioFileReaderRef;

BOOST_AUTO_TEST_SUITE( AudioFileReaderTest )

using namespace WM;

struct FileAReaderFixture {
    
    FileAReaderFixture() {
        
        CFStringRef file_string = CFSTR("file_a.caf");
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

struct Sine16KHZReaderFadedFixture {
    
    Sine16KHZReaderFadedFixture() {
        
        CFStringRef file_string = CFSTR("sine_880hz_1_sec_norm_faded_16khz.caf");
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

struct Sine16KHZReaderTrimmedFixture {
    
    Sine16KHZReaderTrimmedFixture() {
        
        CFStringRef file_string = CFSTR("sine_880hz_1_sec_norm_trimmed_16khz.caf");
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


struct Sine16KHZReaderFixture {
    
    Sine16KHZReaderFixture() {
        
        CFStringRef file_string = CFSTR("sine_40hz_1_sec_norm_16khz.wav");
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

struct Sine48KHZReaderFixture {
    
    Sine48KHZReaderFixture() {
        CFStringRef file_string = CFSTR("sine_40hz_1_sec_norm_48khz.wav");
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

namespace
{
    bool almost_equal(float a, float b)
    {
//        const float epsilon = 0.002f;
        const float epsilon = 0.0105f;        
        return fabs(a-b)<=epsilon;
    }
}

void check_40hz_1sec_sine(AudioFileReaderRef reader) {
    
    //Since we have created this file manually, we know the exact number of 
    //frames this file must contain.
    //1 sec of 16khz == 16k frames
    BOOST_CHECK_CLOSE(1.0f, reader->duration(), 5);
    
    //Let's read in 400 samples in chunks and confirm our expected sine wave
    //data
    
    WMAudioSampleType data[400];
    std::fill(&data[0], &data[400], 0);
    
    size_t num_samples = 0;
    size_t read_num_samples = 0;
    
    num_samples = 400;    
    
    while(num_samples > 0) {
        
        num_samples = 400;    
        
        bool success = reader->read_floats(num_samples, data);
        BOOST_REQUIRE(success);
        
        for (int j = 0; j<num_samples; ++j) {
            //Time between 0..1
            float time = read_num_samples / 16000.0f;
            //Convert to angle
            float angle = 2.0f*M_PI * time;
            //40hz sine
            float check_value = sin(angle*40);
            
            //Check if they are equal
            bool b = almost_equal(check_value, data[j]);
            
            BOOST_CHECK(b);
            
            read_num_samples++;
        }
        
    }
    
}

void check_40hz_1sec_sine_overlap(AudioFileReaderRef reader) {
    
    //Since we have created this file manually, we know the exact number of 
    //frames this file must contain.
    //1 sec of 16khz == 16k frames
    BOOST_CHECK_CLOSE(1.0f, reader->duration(), 5);
    
    //Let's read in 400 samples in chunks and confirm our expected sine wave
    //data
    
    //However, this time, on each read, we advance only by 10ms on each read, 
    //but require a packet of 400 frames. This is equivalent to a hop size 
    //of 160 frames (with 16khz sampling)
    
    WMAudioSampleType data[400];
    std::fill(&data[0], &data[400], 0);
    
    size_t num_samples = 400;
    size_t num_iterations = 0;
    
    const float interval = 0.01f;
//    const float interval = 0.0125f;
    const float sample_duration = 1.0f/16000.0f;
    
    while(num_samples > 0) {
        
        num_samples = 400;    
        
        float iteration_time = num_iterations * interval;
        
        bool success = reader->read_floats(num_samples, data, iteration_time);
        BOOST_REQUIRE(success);

        //Note: we ignore the first few elements of a packet since on-the-fly
        //resampling of the ExtAudioFile API might cause some artefacts here
        //(probably due to windowing, etc...)
        
        for (int j = 5; j<num_samples; ++j) {
            //Time between 0..1
            float time = iteration_time + sample_duration*(j+0.5f);
            //Convert to angle
            float angle = 2.0f * M_PI * time;
            //40hz sine
            float check_value = sin(angle*40);
            
            //Check if they are equal
            bool b = almost_equal(check_value, data[j]);
            
            if (!b) {
                float diff = (check_value - data[j]);
                std::cout << diff << std::endl;
            }
            
            BOOST_CHECK(b);

        }
        
        num_iterations++;
    }
    
}

BOOST_AUTO_TEST_CASE(JunkInputTest) {
    
    //test input with a file that does not exist
    CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
                                                 CFSTR("/somejunk/bla.wav"),
                                                 kCFURLPOSIXPathStyle, 
                                                 false);
    
    AudioFileReaderRef fail_reader;
    
    BOOST_CHECK_THROW(fail_reader = AudioFileReaderRef(new AudioFileReader(url)), 
                      std::invalid_argument);
    
    CFRelease(url);
}

BOOST_FIXTURE_TEST_CASE(ReadSineWave16khzTest, Sine16KHZReaderFixture) {
    
    //reads a wave file with 40 hz, 1 sec duration, already sampled in 16kHZ
    check_40hz_1sec_sine(reader);
    
}

BOOST_FIXTURE_TEST_CASE(ReadSineWave48khzTest, Sine48KHZReaderFixture) {
    
    //The same as above, but this test performs resampling as well
    check_40hz_1sec_sine(reader);
    
}

BOOST_FIXTURE_TEST_CASE(ReadSineWave16khzOverlappingTest, 
                        Sine16KHZReaderFixture) {
    
    // This test checks if the overlapping with a specified time interval 
    // (or in terms of sampels "hopsize" works as expected.
    check_40hz_1sec_sine_overlap(reader);
    
}

BOOST_FIXTURE_TEST_CASE(ReadSineWave48khzOverlappingTest, 
                        Sine48KHZReaderFixture) {
    
    // This test checks if the overlapping with a specified time interval 
    // (or in terms of sampels "hopsize" works as expected.
    check_40hz_1sec_sine_overlap(reader);
    
}

BOOST_FIXTURE_TEST_CASE(TestPreProcessTrimmed, 
                        Sine16KHZReaderTrimmedFixture) {
    
    WMAudioFilePreProcessInfo info = reader->preprocess(-20.0f, -40.0f, 1.0f);

    BOOST_REQUIRE_EQUAL(info.max_peak, 1.0f);
    BOOST_REQUIRE_CLOSE(info.normalization_factor, 1.0f, 0.01f);
    
    //This can't be hugely accurate
    BOOST_REQUIRE_CLOSE(info.threshold_start_time, 0.25f, 4);
    BOOST_REQUIRE_CLOSE(info.threshold_end_time, 0.75f, 4);
    
}

BOOST_FIXTURE_TEST_CASE(TestPreProcessFaded, 
                        Sine16KHZReaderFadedFixture) {
    
    WMAudioFilePreProcessInfo info = reader->preprocess(-6, -6, 1.0f);
    
    BOOST_REQUIRE_CLOSE(info.max_peak, 1.0f, 1);
    
    //This can't be hugely accurate
    BOOST_REQUIRE_CLOSE(info.threshold_start_time, 0.25f, 4);
    BOOST_REQUIRE_CLOSE(info.threshold_end_time, 0.75f, 4);
    
}

BOOST_FIXTURE_TEST_CASE(TestPreProcessFileA, 
                        FileAReaderFixture) {
    
    WMAudioFilePreProcessInfo info = reader->preprocess(-27, -17, 0.4368f);
    
    BOOST_REQUIRE_CLOSE(info.max_peak, 0.4368f, 1);
    
    //This can't be hugely accurate
    BOOST_REQUIRE_CLOSE(info.threshold_start_time, 1.0, 1);
    BOOST_REQUIRE_CLOSE(info.threshold_end_time, 1.34, 1);    
    
}

BOOST_AUTO_TEST_SUITE_END()