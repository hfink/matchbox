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
#include <boost/shared_ptr.hpp>
#include <vector>

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "MFCCProcessor.hpp"
#include "AudioFileReader.hpp"
#include "dtw.hpp"
#include "DebugUtils.h"
#include "MFCCUtils.h"
#include "benchmark.h"

#include <iostream>

typedef boost::shared_ptr<WM::AudioFileReader> AudioFileReaderRef;

typedef boost::scoped_array<float> FloatScopedArray;
typedef boost::scoped_array<WMFeatureType> FeatureTypeArray;

BOOST_AUTO_TEST_SUITE( DTWMFCCTest )

using namespace WM;

struct PulloversReaderFixture {
    
    PulloversReaderFixture() {
        
        CFStringRef pullover_one_str = CFSTR("02-pullover-2.wav");
        CFStringRef pullover_two_str = CFSTR("02-pullover-3.wav");
        CFStringRef blumentopf_str = CFSTR("04-blumentopf-3.wav");        
        CFURLRef pullover_one_url = NULL;
        CFURLRef pullover_two_url = NULL;
        CFURLRef blumentopf_url = NULL;        
        
#ifdef TEST_USE_MAIN_BUNDLE_FOR_FILES
        
        CFBundleRef main_bundle;
        
        // Get the main bundle for the app
        main_bundle = CFBundleGetMainBundle();  
        
        if (main_bundle == NULL)
            throw std::runtime_error("Cannot load main bundle");
        
        pullover_one_url = CFBundleCopyResourceURL(main_bundle, 
                                                   pullover_one_str, 
                                                   NULL, 
                                                   NULL);
        if (pullover_one_url == NULL)
            throw std::runtime_error("Cannot load file from bundle.");
        
        pullover_two_url = CFBundleCopyResourceURL(main_bundle, 
                                                   pullover_two_str, 
                                                   NULL, 
                                                   NULL);
        if (pullover_two_url == NULL)
            throw std::runtime_error("Cannot load file from bundle.");     
        
        blumentopf_url = CFBundleCopyResourceURL(main_bundle, 
                                                 blumentopf_str, 
                                                 NULL, 
                                                 NULL);
        if (blumentopf_url == NULL)
            throw std::runtime_error("Cannot load file from bundle.");         
        
#else
        
        pullover_one_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
                                            pullover_one_str,
                                            kCFURLPOSIXPathStyle, 
                                            false);
        
        pullover_two_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
                                                         pullover_two_str,
                                                         kCFURLPOSIXPathStyle, 
                                                         false);     
        
        blumentopf_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
                                                       blumentopf_str,
                                                       kCFURLPOSIXPathStyle, 
                                                       false);             
        
#endif
        
        BOOST_REQUIRE_NO_THROW (
                                pullover_rdr_variant_one = AudioFileReaderRef(new AudioFileReader(pullover_one_url))
                                );
        
        BOOST_REQUIRE_NO_THROW (
                                pullover_rdr_variant_two = AudioFileReaderRef(new AudioFileReader(pullover_two_url))
                                );   
        
        BOOST_REQUIRE_NO_THROW (
                                blumentopf_rdr = AudioFileReaderRef(new AudioFileReader(blumentopf_url))
                                );         

        CFRelease(pullover_one_url);                
        CFRelease(pullover_two_url);        
        CFRelease(blumentopf_url);        
        
        
	}
    
    AudioFileReaderRef pullover_rdr_variant_one;
    AudioFileReaderRef pullover_rdr_variant_two;
    AudioFileReaderRef blumentopf_rdr;
    
};

BOOST_FIXTURE_TEST_CASE( DifferentPulloverTest, PulloversReaderFixture ) {
    
    //Create two packages of features
    FeatureTypeDTW::Features features_pullover_one;
    FeatureTypeDTW::Features features_pullover_two; 
    FeatureTypeDTW::Features features_blumentopf;     
    
    features_pullover_one = get_mfcc_features(pullover_rdr_variant_one);
    features_pullover_two = get_mfcc_features(pullover_rdr_variant_two);
    features_blumentopf = get_mfcc_features(blumentopf_rdr);
    
    FeatureTypeDTW dtw_close(features_pullover_one, features_pullover_two, 20);
    
    FeatureTypeDTW dtw_far(features_pullover_one, features_blumentopf, 20);    
    
    WMFeatureType min_distance_close = dtw_close.minimum_distance();
    WMFeatureType min_distance_far = dtw_far.minimum_distance();    
    
    std::cout << "DTW: 02-pullover-2.wav vs. 02-pullover-3.wav; Min-Distance: "
              << min_distance_close << std::endl;
    
    std::cout << "DTW: 02-pullover-2.wav vs. 04-blumentopf-3.wav; Min-Distance: "
              << min_distance_far << std::endl;
    
    // dump_dtw_to_matlab_file(dtw_far, "/tmp/dtw-dump.m");

    //This was for double-checkig the MFCC components with the 
    //Matlab implementation
//    std::cout << "cpp_mfcc = [";
//    for (int i = 0; i<7; ++i) {
//        
//        if (i != 0)
//            std::cout << "; ";            
//        
//        for (int j = 0; j<features_blumentopf.size(); ++j) {
//            
//            if (j != 0)
//                std::cout << ", ";
//            
//            std::cout << features_blumentopf[j].at(i);
//        }
//    }    
//    
//    std::cout << "];" << std::endl;    
    
}

BOOST_AUTO_TEST_CASE( BenchmarkTest ) {
    
    show_benchmark_data(6,
                        4);    
    
}
    
BOOST_AUTO_TEST_SUITE_END()
