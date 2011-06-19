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

#include "MFCCProcessor.hpp"

#include "CABitOperations.h"

#include <stdexcept>
#include <sstream>
#include <math.h>

#include <iostream>

// Macro to check for memory alignment

#define MEM_IS_ALIGNED(POINTER, BYTE_COUNT) \
(((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)

using namespace WM;

MFCCProcessor::MFCCProcessor(size_t user_window_size,
                             float pre_emph_alpha,
                             int sampling_rate, 
                             float mel_min_freq, 
                             float mel_max_freq) :
    user_window_size_(user_window_size), 
    fft_size_(NextPowerOfTwo((UInt32)user_window_size_*2)),
    fft_size_half_(fft_size_ >> 1),
    pre_emph_alpha_(pre_emph_alpha),
    process_buffer_(new float[fft_size_]),
    hamming_window_(new float[user_window_size_]),
    sampling_rate_(sampling_rate),
    mel_filter_bank_(mel_min_freq, 
                     mel_max_freq,
                     kNumMelBands(), 
                     (int)(fft_size_half_), // half of FFT size 
                     sampling_rate),
    fft_data_real_part_(new float[fft_size_half_]),
    fft_data_imag_part_(new float[fft_size_half_]),
    mel_bands_buffer_(new float[kNumMelBands()]),
    dct_ii_matrix_(new float[kNumMelBands() * kNumMelCepstra()])
{
    std::ostringstream oss;
    //input checks
    if (user_window_size == 0) {
        oss << "Window size is zero.";
        throw std::invalid_argument(oss.str());
    }
    
    if ( (pre_emph_alpha < 0) ||
         (pre_emph_alpha > 1) ) {
        oss << "Invalid pre-emphasis coefficient: '" << pre_emph_alpha << "'.";
        throw std::invalid_argument(oss.str());
    }
    
    if (sampling_rate <= 0) {
        oss << "Invalid sampling rate: '" << sampling_rate << "'.";
        throw std::invalid_argument(oss.str());
    }
    
    if (mel_max_freq <= mel_min_freq) {
        oss << "Invalid mel min/max frequencies: min='" << mel_min_freq
            << "', max='" << mel_max_freq << "'.";
        throw std::invalid_argument(oss.str());
    }
    
    if (mel_max_freq > sampling_rate * 0.5f) {
        oss << "Max Mel-Frequency is higher than the nyquist limit (sr/2): ";
        oss << "max='" << mel_max_freq << "'.";
        throw std::invalid_argument(oss.str());        
    }
    
    //Set FFT buffer to zero
    vDSP_vclr(process_buffer_.get(), 1, fft_size_);
    
    //initialize a symmetric hamming window. This is later going to be used
    //to be applied to our sample window.
    vDSP_hamm_window(hamming_window_.get(), user_window_size_, 0);
    
    fft_log2n_ = static_cast<int>(log2l(static_cast<long>(fft_size_)));
    
    // FFT setup, will be reused
    fft_setup_ = vDSP_create_fftsetup(fft_log2n_, FFT_RADIX2);
    if (fft_setup_ == NULL) {
        throw std::runtime_error("Could not create FFT setup.");
    }
    
    //set to zero 
    vDSP_vclr(fft_data_real_part_.get(), 1, fft_size_half_);
    vDSP_vclr(fft_data_imag_part_.get(), 1, fft_size_half_);
    
    fft_data_split_complex_.realp = fft_data_real_part_.get();
    fft_data_split_complex_.imagp = fft_data_imag_part_.get();
    
    vDSP_vclr(mel_bands_buffer_.get(), 1, kNumMelBands());
    
    //Calculate the matrix to perform DCT
    //see
    //http://www.dsprelated.com/dspbooks/mdft/Discrete_Cosine_Transform_DCT.html
    
    // we are preparing this matrix to be multiplied with a column vector, and
    // we store its data in column major order
    const float ortho_factor = sqrtf(2.0f/(float)kNumMelBands());
    for (int n = 0; n < kNumMelBands(); ++n) { // cols
        for (int k = 0; k < kNumMelCepstra(); ++k) { // rows
            float omega = (float(M_PI) / kNumMelBands()) * (float)k;
            float val = 2*cosf(omega*((float)n + 0.5f));
            
            // we further multiply by sqrt(2/N) to creat the orthogonal 
            // versio: http://en.wikipedia.org/wiki/Discrete_cosine_transform
            val *= ortho_factor;
            
            dct_ii_matrix_[n*kNumMelCepstra() + k] = val;
            
        }
    }
    
    // vDSP Documentation suggests using 16byte aligned pointers, malloc does 
    // that by default, just check that the "new" implementation took care of 
    // that as well...
    bool is_aligned = ( MEM_IS_ALIGNED(fft_data_real_part_.get(), 16) ||
                        MEM_IS_ALIGNED(fft_data_imag_part_.get(), 16) ||
                        MEM_IS_ALIGNED(process_buffer_.get(), 16) || 
                        MEM_IS_ALIGNED(hamming_window_.get(), 16) ||
                        MEM_IS_ALIGNED(mel_bands_buffer_.get(), 16) ||
                        MEM_IS_ALIGNED(dct_ii_matrix_.get(), 16) );
    
    if (!is_aligned) {
        std::cout << "Warning: Memory allocation for FFTs is not 16-byte "
        << " aligned. This might cause lesser performance." << std::endl;
    }    
    
}

MFCCProcessor::~MFCCProcessor() {

    vDSP_destroy_fftsetup(fft_setup_);
    
}

void MFCCProcessor::process(const WMAudioSampleType * samples,
                            WMAudioSampleType pre_emph_filter_border,
                            CepstraBuffer * mfcc_out,
                            WMFeatureType * spectrum_mag_out,
                            WMFeatureType * mel_spectrum_mag_out)
{

    if ( samples == NULL )
        return;
    
    //we either copy straight to the process buffer, or we perform 
    //pre-emphasis and set the process buffer as the target

    
    // Pre-emphasis, a high-pass filter
    if (pre_emph_alpha_ != 0) {
        pre_emphasize_to_buffer(pre_emph_filter_border, samples);
    } else {
        size_t num_bytes = sizeof(WMAudioSampleType)*user_window_size_;
        memcpy(process_buffer_.get(), samples, num_bytes);        
    }
    
    //make sure that the rest of process buffer is set to zero
    vDSP_vclr(&process_buffer_[user_window_size_], 1, fft_size_ - user_window_size_);    
    
    // Apply Hamming Window before performing FFT
    apply_hamming_window();
    
    // FWD FFT Real, in-place
    calculate_spectrum_magnitudes();
    
    // Copy FFT magnitudes, if requested.
    if (spectrum_mag_out != NULL) {
        std::copy(&process_buffer_[0], 
                  &process_buffer_[fft_size_half_], 
                  spectrum_mag_out);
    }
    
    //After the previous step, we know that the first half of the process_buffer
    //holds the magnitudes of the spectrum. We can now apply the mel filter
    //on it.
    
    // mel triangular bandpass filter
    mel_filter_bank_.apply(process_buffer_.get(), mel_bands_buffer_.get());
    
    //Copy mel power spectrum, if requested.
    
    if (mel_spectrum_mag_out != NULL) {
        std::copy(&mel_bands_buffer_[0], 
                  &mel_bands_buffer_[kNumMelBands()], 
                  mel_spectrum_mag_out);        
    }    
    
    // take the log of the mel bands

    if (mfcc_out != NULL) {    
    
        //We only have a vectorized version to get the log10 on OS X
#ifdef HAVE_VDSP_FORCE_LIB
        const int num_mel_bands = kNumMelBands();
        vvlog10f(mel_bands_buffer_.get(), mel_bands_buffer_.get(), &num_mel_bands);
#else
        for (int i = 0; i < kNumMelBands(); ++i)
            mel_bands_buffer_[i] = log10f(mel_bands_buffer_[i]);
#endif
    
        // Perform the discrete cosine transform. We have prepared a matrix that
        // we can simply multiply with the vector of mel_bands
        
        vDSP_vclr(mfcc_out->c_array(), 1, kNumMelCepstra());
        
        // Vectorized matrix multiplication
        
        cblas_sgemm(CblasColMajor,
                    CblasNoTrans, 
                    CblasNoTrans,
                    kNumMelCepstra(),
                    1, 
                    kNumMelBands(), 
                    1, 
                    dct_ii_matrix_.get(), 
                    kNumMelCepstra(), 
                    mel_bands_buffer_.get(), 
                    kNumMelBands(), 
                    0.0f, 
                    mfcc_out->c_array(), 
                    kNumMelCepstra());
        
        // for orthogonalized DCT version we have to multiply the first cepstral
        // value with 1/sqrt(2)
        const float sqrt_two_inv = 1.0f/sqrtf(2.0f);
        mfcc_out->c_array()[0] *= sqrt_two_inv;
        
    }
    
    // TODO: Add optional other features like differential values or energy/
    // packet
    
}

void MFCCProcessor::pre_emphasize_to_buffer(float border_value,
                                            const WMAudioSampleType* orig_audio)
{
    // Apply a high-pass filter to compensate the high-frequency part that was
    // suppressed during sound production of humans
    
    // s2(n) = s(n) - a*s(n-1), where a is between 0.96 ..0.99
    
    // multiply with -pre_emph_coeff into process buffer 
    
    // vector add + factor, mind the offset
    const float pre_emph_neg = -pre_emph_alpha_;
    vDSP_vsma(orig_audio, 
              1, 
              &pre_emph_neg, 
              &orig_audio[1], 
              1, 
              &(process_buffer_.get())[1], 
              1, 
              user_window_size_-1);
    
    // deal with border properly
    process_buffer_[0] = orig_audio[0] - pre_emph_alpha_*  border_value;
    
}

void MFCCProcessor::apply_hamming_window()
{
    // Simply apply the previously generated hamming window
    vDSP_vmul(process_buffer_.get(), 1, 
              hamming_window_.get(), 1, 
              process_buffer_.get(), 1, 
              user_window_size_);
    
}

void MFCCProcessor::calculate_spectrum_magnitudes() {
    
    // Convert to a special packed data format (see vDSP programming guide, 
    // single array packed data format)
    
    vDSP_ctoz((DSPComplex*)process_buffer_.get(), 
              2, 
              &fft_data_split_complex_, 
              1, 
              fft_size_half_);
    
    // Perform the actual FFT
    
    vDSP_fft_zrip(fft_setup_, 
                  &fft_data_split_complex_, 
                  1, 
                  fft_log2n_, 
                  FFT_FORWARD);
    
    // Get the magnitudes (our actual power spectrum we are interested in)
    // we re-use the same memory, but we need only half of its size
    vDSP_zvabs(&fft_data_split_complex_, 
               1, 
               process_buffer_.get(), 
               1, 
               fft_size_half_);
    
    // We still need to divide by 2 as the previous FWD FFT introduced a factor
    // of 2 due to optimization techniques (see vDSP programming guide)
    float scale = 0.5f;
    
    vDSP_vsmul(process_buffer_.get(), 
               1, 
               &scale, 
               process_buffer_.get(), 
               1, 
               fft_size_half_);
    
    // Done, the first half of process_buffer_ now holds the magnitude spectrum
    
}



