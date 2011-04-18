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

#ifndef WORD_MATCH_MEL_FILTER_BANK_HPP
#define WORD_MATCH_MEL_FILTER_BANK_HPP

#include "TriangleFilter.hpp"

#include <vector>

namespace WM {
    
    /**
     * This class represents the filters necessary to warp an audio spectrum
     * into Mel-Frequency scaling. It is basically a collection of properly
     * placed TriangleFilters.
     */
    class MelFilterBank {
        
    public:
        
        /**
         * Creates a new MelFilterBank.
         * @param min_freq The minimum frequency in hz to be considered, i.e.
         * the left edge of the first TriangleFilter.
         * @param max_freq The maximum frequency in hz to be considered, i.e.
         * the right edge of the last TriangleFilter.
         * @param num_mel_bands The number of Mel bands to be calculated, i.e.
         * the number of TriangleFilters to be applied.
         * @param num_bins The number of bins that are present in the fft_buffer
         * that will be passed to the MelFilterBank::apply method. This is also
         * required to properly configure the TriangleFilter instances which
         * operate on array indices only.
         * @param sample_rate The original sample rate the FFT buffer which will 
         * be passed to MelFilterBank::apply is based on.
         * @param normalize_filter_area If set to "true", the area of the
         * created TriangleFilter will be normalized, e.g. the height of the 
         * filter's triangle shape will be configured in a way, that the area
         * of the triangle shape equals one.
         */
        MelFilterBank(float min_freq, 
                      float max_freq,
                      int num_mel_bands,
                      int num_bins,
                      int sample_rate,
                      bool normalize_filter_area = true);
        
        /**
         * Apply all filters on the incoming FFT data, and write out the results
         * into an array.
         * @param fft_data The incoming FFT data on which the triangle filters
         * will be applied on.
         * @param mel_bands The caller is responsible that the passed array 
         * accomodates at least num_mel_bands elements. On output this array
         * will be filled with the resulting Mel-Frqeuency warped spectrum.
         */
        void apply(const float* fft_data, float* mel_bands) const;
        
        /**
         * Utility function to convert HZ to Mel.
         */
        static float hz_to_mel(float hz);
        
        /**
         * Utility function to convert Mel to HZ.
         */
        static float mel_to_hz(float mel);
        
        /**
         * Used for debugging. Prints out a detailed descriptions of the 
         * configured filters.
         */
        void print() const;
        
    private:
        
        float min_freq_;
        float max_freq_;
        int num_mel_bands_;
        int num_bins_;
        int sample_rate_;
        const bool normalize_filter_area_;
        
        typedef std::vector<TriangleFilterRef> TriangleFilterArray;
        TriangleFilterArray filters_;
        
    };
    
}

#endif //WORD_MATCH_MEL_FILTER_BANK_HPP

