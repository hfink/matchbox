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

#import <Foundation/Foundation.h>

/**
 * This is a class dedicated to recording voice-only into a 16khz AAC audio
 * file. This class is utilizing the VoiceProcessing AudioUnit which provides
 * a clearer and already noise-filtered sound.
 */
@interface SOVoiceRecorder : NSObject {
@private

    AudioUnit io_audio_unit_;
    ExtAudioFileRef file_target_;
    
    AudioStreamBasicDescription asbd_mono_canonical_;
    AudioStreamBasicDescription asbd_mono_aac_16khz_;    
    
    BOOL isRecording;
    
    AudioBufferList input_buffer_list;
}

/**
 * Is set to YES if the recorder is currently recording. NO otherwise.
 */
@property (assign, readonly) BOOL isRecording;

/**
 * Start recording into a designated URL. Existing files will be overwritten 
 * without further confirmation. If the recorder is already recording, the
 * record process will be stopped before and re-started.
 */
- (void)startRecordingIntoFile:(NSURL*) url;

/**
 * Stops the current recording process and flushes the file that was previously
 * passed to startRecordingIntoFile. If the recorder is not currently
 * recording, this call has no effect.
 */
- (void)stopRecording;

/**
 * Static helper methods for debugging and error handling.
 */

+ (void) checkResult:(OSStatus) result withMessage:(NSString*) msg;
+ (void) printErrorMessage: (NSString *) errorString withStatus: (OSStatus) result;

@end
