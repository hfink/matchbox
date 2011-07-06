//Copyright (c) 2011 Heinrich Fink hf@hfink.eu
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

#import "IPSongReaderTest.h"

#import "IPSongReader.h"
#include "WordMatch.h"

#include <math.h>

#import <SenTestingKit/SenTestCase.h>

#import <CoreAudio/CoreAudioTypes.h>

@implementation IPSongReaderTest

#if USE_APPLICATION_UNIT_TEST     // all code under test is in the iPhone Application

- (void)testAppDelegate {
    
    id yourApplicationDelegate = [[UIApplication sharedApplication] delegate];
    STAssertNotNil(yourApplicationDelegate, @"UIApplication failed to find the AppDelegate");
    
}

#else                           // all code under test must be linked into the Unit Test bundle

static const float kTestDuration = 2;

//Note that this method assume 10 seconds file!
- (void)check440hzSineStereoWithData:(Float32*)audioData
                     sampleRate:(Float64)samplingRate
                     numSamples:(size_t)numSamples
                   sampleOffset:(size_t)sampleOffset
{
    for (int i = 0; i<numSamples; ++i) {
        float time = 5.0f - kTestDuration*0.5f + (float)(sampleOffset + i) / samplingRate;
        float reference = sinf(time*440.0f*2.0f*M_PI);
        //TODO: fix this for non-interleaved audio, or mono audio!
        float current_sample = audioData[i*2];
        STAssertEqualsWithAccuracy(reference, current_sample, 0.0105, @"Comparing references for sine.");
    }
}

- (void)testReader {
    
    [self raiseAfterFailure];
    
    NSBundle* test_bundle = [NSBundle bundleForClass:[self class]];
    NSURL* file_url = [test_bundle URLForResource:@"sine_440hz_44100khz_10sec.wav" withExtension:nil];
    STAssertNotNil(file_url, @"Could not load test file.");
    
    static size_t num_samples_read = 0;    
    
    NSLog(@"Starting reader test...");
    
    IPSongReader * reader = [[IPSongReader alloc] initWithURL:file_url forDuration:kTestDuration withBlock:^BOOL(CMSampleBufferRef sampler){
        
        BOOL is_data_ready = CMSampleBufferDataIsReady(sampler);
        
        STAssertTrue(is_data_ready, @"Sample data wasn't ready.");
        
        //Check the format description
        CMFormatDescriptionRef format_desc = CMSampleBufferGetFormatDescription(sampler);
        
        //TODO: in subsequent, store this and compare with upcoming...
        const AudioStreamBasicDescription* asbd =  CMAudioFormatDescriptionGetStreamBasicDescription(format_desc);
        
        WMPrintAudioStreamBasicDescription(asbd);
        
        CMItemCount count_samples = CMSampleBufferGetNumSamples(sampler);
        
        NSLog(@"Got %ld num of samples.", count_samples);
        
        AudioBufferList abl;
        CMBlockBufferRef audio_data;
        //Note that we assume that our data is multichannel and interleaved at this point...
        OSStatus result = CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(sampler, 
                                                                                  NULL, 
                                                                                  &abl, 
                                                                                  sizeof(AudioBufferList), 
                                                                                  NULL, 
                                                                                  NULL, 
                                                                                  kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, 
                                                                                  &audio_data);
        
        
        STAssertTrue(abl.mBuffers[0].mNumberChannels == 2, @"Numbers of channels != 2");
        
        [self check440hzSineStereoWithData:abl.mBuffers[0].mData 
                                sampleRate:44100.0 
                                numSamples:count_samples sampleOffset:num_samples_read];
        
        CFRelease(audio_data);
        STAssertTrue(result == noErr, @"Operation to fetch audio data failed.");
        
        num_samples_read += (int)count_samples;
        
        return TRUE;
    }];
    
    STAssertNotNil(reader, @"Could not create reader onto test file.");
    
    BOOL success = [reader consumeRange];
    STAssertTrue(success, @"Test constumption process failed.");    

    NSLog(@"Read %ld num of sample.", num_samples_read);
    
    size_t expected_num_samples = 88200;
    STAssertEquals(num_samples_read, expected_num_samples, @"Checksum for read audio material failed");    
    
    [reader release];
}

#endif

@end
