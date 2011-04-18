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

#include "benchmark.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

namespace {
    std::string sample_name(size_t speaker_number, size_t sample_number)
    {
        return "samples/0" +
            boost::lexical_cast<std::string>(sample_number) + "-" +
            boost::lexical_cast<std::string>(speaker_number) + ".wav";
    }
}


FeatureTypeDTW::Features get_mfcc_features(const std::string& filename)
{
    CFStringRef filename_cfstring = CFStringCreateWithCString(kCFAllocatorDefault,
                                                              filename.c_str(),
                                                              kCFStringEncodingUTF8);
    CFURLRef url = NULL;
    
#ifdef TEST_USE_MAIN_BUNDLE_FOR_FILES
    
    CFBundleRef main_bundle;
    
    // Get the main bundle for the app
    main_bundle = CFBundleGetMainBundle();  
    
    if (main_bundle == NULL)
        throw std::runtime_error("Cannot load main bundle");
    
    url = CFBundleCopyResourceURL(main_bundle, filename_cfstring, NULL, NULL);
    if (url == NULL)
        throw std::runtime_error("Cannot load file from bundle.");
    
#else
    
    url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                        filename_cfstring,
                                        kCFURLPOSIXPathStyle,
                                        false);
    
#endif    
    
    CFRelease(filename_cfstring);
    
    AudioFileReaderRef audio_file_reader = AudioFileReaderRef(new WM::AudioFileReader(url));
    CFRelease(url);
    return get_mfcc_features(audio_file_reader);
}

WMFeatureType mfcc_dtw_distance(const std::string& filename_a,
                                const std::string& filename_b)
{
    typedef std::map<std::string, FeatureTypeDTW::Features> FeatureCache;
    static FeatureCache cache_map;
    
    //Try to look up in static cache first
    FeatureCache::const_iterator it_cache_a = cache_map.find(filename_a);
    FeatureCache::const_iterator it_cache_b = cache_map.find(filename_b);    
    
    FeatureTypeDTW::Features features_a;
    FeatureTypeDTW::Features features_b;
    
    if (it_cache_a != cache_map.end()) {
        features_a = it_cache_a->second;
    } else {
        features_a = get_mfcc_features(filename_a);
        FeatureCache::value_type v(filename_a, features_a);
        if (!cache_map.insert(v).second)
            throw std::runtime_error("Cannot insert into cache.");
    }
    
    if (it_cache_b != cache_map.end()) {
        features_b = it_cache_b->second;
    } else {
        features_b = get_mfcc_features(filename_b);
        FeatureCache::value_type v(filename_b, features_b);
        if (!cache_map.insert(v).second)
            throw std::runtime_error("Cannot insert into cache.");
    }    
    
    FeatureTypeDTW dtw_data(features_a, features_b, 20);
    return dtw_data.minimum_distance();
}

SpeakerDistances calculate_benchmark_table(unsigned number_of_samples,
                                           unsigned number_of_speakers)
{
    const WMFeatureType NaN = std::numeric_limits<WMFeatureType>::quiet_NaN();

    SpeakerDistances benchmark_table;

    for (size_t i=0; i<number_of_speakers; ++i)
        for (size_t j=0; j<number_of_speakers; ++j)
        {
            const SpeakerPair speakers = std::make_pair(i, j);
            DistanceTable distances(boost::extents[number_of_samples][number_of_samples]);

            for (size_t x=0; x<number_of_samples; ++x)
                for (size_t y=0; y<number_of_samples; ++y)
                {
                    WMFeatureType distance;
                    if (i==j && x==y)
                        // same speaker, same utterance
                        distance = 0;
                    else if (i<=j && x<=y)
                    {
                        const std::string sample1 = sample_name(i+1, x+1);
                        const std::string sample2 = sample_name(j+1, y+1);
                        distance = mfcc_dtw_distance(sample1,
                                                     sample2);
                    }
                    else
                        // samples have already been compared
                        distance = NaN;

                    distances[x][y] = distance;
                }
            benchmark_table.insert(std::make_pair(speakers, distances));
        }
    return benchmark_table;
}

void analyze_benchmark_table(const SpeakerDistances& benchmark_table)
{
    // TODO: move threshold to a useful location, it's pretty much useless here
    const WMFeatureType threshold = 3.09;

    unsigned true_positives = 0;
    unsigned true_negatives = 0;
    unsigned false_negatives = 0;
    unsigned false_positives = 0;
    WMFeatureType suggested_threshold = 0.0;

    for (SpeakerDistances::const_iterator i=benchmark_table.begin();
         i!=benchmark_table.end();
         ++i)
    {
        const DistanceTable distances = (*i).second;
        for (size_t x=0; x<distances.size(); ++x)
            for (size_t y=0; y<distances[x].size(); ++y)
            {
                assert(distances.size() == distances[x].size());
                WMFeatureType distance = distances[x][y];
                if (std::isnan(distance))
                    continue;
                bool match = distance<threshold;
                if (x==y)
                {
                    // same utterance!
                    suggested_threshold = std::max(suggested_threshold,
                                                   distance);
                    if (match)
                        ++true_positives;
                    else
                        ++false_negatives;
                }
                else
                {
                    if (match)
                        ++false_positives;
                    else
                        ++true_negatives;
                }
            }
    }
    const double recall = static_cast<double>(true_positives)/(true_positives+false_negatives);
    const double precision = static_cast<double>(true_positives)/(true_positives+false_positives);
    const double specifity = static_cast<double>(true_negatives)/(true_negatives+false_positives);
    const double f_measure = 2*precision*recall / (precision + recall);
    std::cout << "\nrecall: " << recall
              << "\nprecision: " << precision
              << "\nspecifity: " << specifity
              << "\nf measure: " << f_measure
              << "\nsuggested threshold: " << suggested_threshold << std::endl;
}

void show_benchmark_data(unsigned number_of_samples,
                         unsigned number_of_speakers)
{
    analyze_benchmark_table(calculate_benchmark_table(number_of_samples,
                                                      number_of_speakers));
}
