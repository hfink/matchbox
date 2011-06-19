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

#ifndef WORD_MATCH_MFCC_PROCESSOR_HPP
#define WORD_MATCH_MFCC_PROCESSOR_HPP

#include "Types.h"

#include <boost/utility.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/array.hpp>

#include "MelFilterBank.hpp"

#include <Accelerate/Accelerate.h>

namespace WM {

    /**
     * This class calculates the Mel-Frequency Cepstral Coefficients from a
     * packet of mono-channel frames. The conversion consists of the following
     * steps: 
     *
     *      ] Pre-Emphasis (a high-pass filter)
     *      ] Applying a Hamming Window to the packet
     *      ] Calculate spectrum magnitues
     *      ] Frequency warping according to the Mel-Scale
     *      ] Applying the Discrete-Cosine-Transform (DCT II) on the warped
     *        frequency bins. This results in the cepstral coefficients.
     *
     * An excellent explanation of these transformations is found in 
     * "Speech and Language Processing", Chapter 9, pg 329 ff. by D. Jurafsky
     * and James Martin.
     *
     * This class uses mostly vectorized versions of transformations using the
     * vDSP framework and should therefore provide very good performance on 
     * Mac OS X and iOS devices (iOS 4 required).
     */
    class MFCCProcessor : boost::noncopyable {

    public:
        
        /**
         * @param user_window_size The window size, i.e. number of frames which 
         * is going to be used for the FFT in order to calculate the MFCC 
         * components. As literature suggest, you should set this window to a
         * a length of ~ 20 - 30ms. Consider a window_overlap of 1/3 - 1/2
         * of the window size. (e.g. for 16khz a window of 320 frames with a
         * sampling rate of 100hz for an overlap of 1/2 is ok. Note that you can
         * set a user_window_size which is not a power-of-two. This class takes
         * care of zero-padding the FFT buffers properly automatically.
         * @param pre_emph_alpha The first transformation in order to calculate
         * the MFCC's is to pre-emphasize the signal, i.e. to boost higher 
         * frequencies that have been suppressed during sound production of 
         * humans. The filter is given as s2(n) = s(n) - pre_emph_alpha*s(n-1), 
         * where a suggested value of pre_emph_alpha should be between 
         * 0.96 .. 0.99.
         * @param sampling_rate The sampling rate of the signal from which the
         * MFCC's will be calculated from.
         * @param mel_min_freq The minimum frequency to be considered during
         * Mel-Frequency warping (for a sampling rate of 16khz this is usually
         * 133.33 HZ)
         * @param mel_max_freq The maximum frequency to be considered during
         * Mel-Frequency warping (for a sampling rate of 16khz this is usually
         * around 6855). This value must not be greater than the nyquist limit
         * (sampling_rate / 2).
         */
        MFCCProcessor(size_t user_window_size,
                      float pre_emph_alpha,
                      int sampling_rate, 
                      float mel_min_freq,
                      float mel_max_freq);
        
        ~MFCCProcessor();
        
        enum {
            NUM_MEL_CEPSTRA = 13,
            NUM_MEL_BANDS = 40
        };        
        
        /**
         * Constant defining how many mel bands should be used during warping.
         * 40 is the number that is used in most implementations.
         */
        static const int kNumMelBands() {
            static const int n = NUM_MEL_BANDS;
            return n;
        }
        
        /**
         * The number of cepstra to be calculated. In most cases 13 should be 
         * enough.
         */
        static const int kNumMelCepstra() {
            static const int n = NUM_MEL_CEPSTRA;
            return n;
        }
        
        typedef boost::array<float, NUM_MEL_CEPSTRA> CepstraBuffer;
        
        /**
         * Takes a packet of mono-channel frames and calculates MFCC from it.
         *
         * @param The audio samples to process. The passed array must hold at
         * least user_window_size number of sample elements. The caller is 
         * responsible for taking care of this. There won't be any additional
         * error checking for that.
         * @param pre_emph_filter_border While applying the pre-emphasis filter
         * we will reach a border case for the first element in the array. Since
         * this class is intended to be used iteratively on packets of frames, 
         * the caller might set the border explicitly to avoid spikes in the 
         * resulting spectrum. The passed value will be used as following: 
         *     s2(0) = s(0) - pre_emph_alpha*pre_emph_filter_border.
         * @param mfcc_out A fixed-size array holding the MFCC's on return.
         * @param spectum_mag_out Pass an array that accomodates at least 
         * fft_size_half number of elements. On return this array will hold
         * the power spectrum that was used to derive the MFCC's from. This
         * parameter is optional.
         * @param mel_spectrum_mag_out Pass an array that accomodates at least 
         * kNumMelBands number of elements. On return this array will hold
         * the power spectrum in Mel-Frequency warped space.
         */
        void process(const WMAudioSampleType * samples,
                     WMAudioSampleType pre_emph_filter_border,
                     CepstraBuffer * mfcc_out,
                     WMFeatureType * spectrum_mag_out = NULL,
                     WMFeatureType * mel_spectrum_mag_out = NULL);
        
        /**
         *@return The number of frames of the FFT buffer. This is the next
         *power-of-two of user_window_size*2
         */
        const size_t& fft_size() const { return fft_size_; }
        
        /**
         *@return Half of fft_size.
         */
        const size_t& fft_size_half() const { return fft_size_half_; }        
        
        /**
         * @return The filter bank that is used by this class to perform the
         * Mel-Frequency warping.
         */
        const MelFilterBank& filter_bank() const { return mel_filter_bank_; }
        
        /**
         * @return The number of frames that will be passed each time to
         * MFCCProcessor::process.
         */
        const size_t& user_window_size() const { return user_window_size_; }
        
    private:
        
        //Pre-emphasizes the signal. See MFCCProcessor::process for an
        //explanation of the border value.
        void pre_emphasize_to_buffer(float border_value, 
                                     const WMAudioSampleType* orig_audio);
        
        void apply_hamming_window();
        void calculate_spectrum_magnitudes();
        
        const size_t user_window_size_;
        const size_t fft_size_;
        const size_t fft_size_half_;
        
        const float pre_emph_alpha_;
        
        typedef boost::scoped_array<float> FloatScopedArray;

        FloatScopedArray process_buffer_;        
        FloatScopedArray hamming_window_;
        
        const int sampling_rate_;        
        
        const MelFilterBank mel_filter_bank_;
        
        int fft_log2n_;
        FFTSetup fft_setup_;
        
        FloatScopedArray fft_data_real_part_;
        FloatScopedArray fft_data_imag_part_;        
        
        DSPSplitComplex fft_data_split_complex_;
        
        FloatScopedArray mel_bands_buffer_;
        
        FloatScopedArray dct_ii_matrix_;
        
    };
    
}

#endif //WORD_MATCH_MFCC_PROCESSOR_HPP