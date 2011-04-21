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

#include "MelFilterBank.hpp"

#include <sstream>
#include <stdexcept>
#include <math.h>

using namespace WM;

MelFilterBank::MelFilterBank(float min_freq, 
                             float max_freq,
                             int num_mel_bands,
                             int num_bins,
                             int sample_rate,
                             bool normalize_filter_area) :
    min_freq_(min_freq), 
    max_freq_(max_freq),
    num_mel_bands_(num_mel_bands),
    num_bins_(num_bins),
    sample_rate_(sample_rate),
    normalize_filter_area_(normalize_filter_area)
{
    
    //Let's do some argument checking
    if ( (min_freq >= max_freq) || 
         (max_freq == 0) ) {
        std::ostringstream oss;
        oss << "Invalid min/max frequencies for MelFilterBank: min = '"
            << min_freq << "' max = '" << max_freq << "'." << std::endl;
        throw std::invalid_argument(oss.str());
    }
    
    if ( num_mel_bands == 0 ) {
        std::ostringstream oss;
        oss << "Invalid number of mel bands for MelFilterBank: n = '"
        << num_mel_bands << "'." << std::endl;
        throw std::invalid_argument(oss.str());
    }
    
    if ( sample_rate == 0 ) {
        std::ostringstream oss;
        oss << "Invalid sample rate for MelFilterBank: s = '"
            << sample_rate << "'." << std::endl;
        throw std::invalid_argument(oss.str());
    }    
    
    if ( num_bins == 0 ) {
        std::ostringstream oss;
        oss << "Invalid number of bins for MelFilterBank: s = '"
        << num_bins << "'." << std::endl;
        throw std::invalid_argument(oss.str());
    }        
    
    float delta_freq = (float)sample_rate_ / (2*num_bins);
    
    float mel_min = hz_to_mel(min_freq_);
    float mel_max = hz_to_mel(max_freq_);    
    
    // We divide by #band + 1 as min / max should present the beginning / end
    // of beginng up / ending low slope, i.e. it's not the centers of each
    // band that represent min/max frequency in mel bands.
    float delta_freq_mel = (mel_max - mel_min) / (num_mel_bands_ + 1);
    
    // Fill up equidistant spacing in mel-space
    
    float mel_left = mel_min;
    for (int i = 0; i<num_mel_bands_; i++) {
        
        float mel_center = mel_left + delta_freq_mel;
        float mel_right = mel_center + delta_freq_mel;
        
        float left_hz = mel_to_hz(mel_left);
        float right_hz = mel_to_hz(mel_right);
        
        //align to closest num_bins (round)
        int left_bin = (int) ((left_hz / delta_freq) + 0.5f);
        int right_bin = (int) ((right_hz / delta_freq) + 0.5f);        
        
        //calculate normalized height
        float height = 1.0f;
        
        if (normalize_filter_area_)
            height = 2.0f / (right_bin - left_bin);
        
        // Create the actual filter
        TriangleFilterRef fltr( new TriangleFilter(left_bin, right_bin, height) );
        filters_.push_back(fltr);
        
        //next left edge is current center
        mel_left = mel_center;
    }
    
}

void MelFilterBank::apply(const float* fft_data, float* mel_bands) const {
    
    //TODO: is dereferencing a smart pointer in here really necessary??
    
    //we assume the caller passes arrays with appropriates sizes
    for (int i = 0; i<num_mel_bands_; ++i)
        mel_bands[i] = filters_[i]->apply(fft_data);
    
}

float MelFilterBank::hz_to_mel(float hz) {
    //melFrequency = 2595 * log(1 + linearFrequency/700)
    const float ln_10 = (float)log(10.0);
    const float f = 2595.0f / ln_10;
    return f * logf(1 + hz / 700.0f);
}

float MelFilterBank::mel_to_hz(float mel) {
    const float ln_10 = (float)log(10.0f);
    const float f = 2595.0f / ln_10;
    return (expf(mel / f) - 1) * 700.0f;    
} 

void MelFilterBank::print() const {

    std::cout << "cpp_mel_filters = [";
    for (int i =0; i<filters_.size(); ++i) {
        if (i != 0)
            std::cout << "; ";
        
        std::cout << *filters_.at(i);
    }
    
    std::cout << "]; " << std::endl << std::endl;
    
}
