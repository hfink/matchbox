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

#import "SOVoiceRecorder.h"

@implementation SOVoiceRecorder

@synthesize isRecording;

#pragma mark Callback Handlers

void ErrorCallbackHandler (void                 *inRefCon,
                           AudioUnit            inUnit,
                           AudioUnitPropertyID  inID,
                           AudioUnitScope       inScope,
                           AudioUnitElement     inElement)
{
    UInt32 size = 0;
    Boolean isWritable = false;
    OSStatus result = AudioUnitGetPropertyInfo(inUnit, 
                                               inID, 
                                               inScope, 
                                               inElement, 
                                               &size, 
                                               &isWritable);
    
    if (result != noErr) {
        [SOVoiceRecorder printErrorMessage:@"In ErrorCallback: while getting prop size." 
                                withStatus:result];        
    }
    
    if (inID == kAudioUnitProperty_LastRenderError) {
        OSStatus reportedError = noErr;
        result = AudioUnitGetProperty(inUnit, 
                                      inID, 
                                      inScope, 
                                      inElement, 
                                      &reportedError, 
                                      &size);
        if (result != noErr) {
            [SOVoiceRecorder printErrorMessage:@"In ErrorCallback: while getting the actual error." 
                                    withStatus:result];                 
        }
        
        if (reportedError != noErr) {
            [SOVoiceRecorder printErrorMessage:@"ErrorCallback: AudioUnit VP reported an error." 
                                withStatus:reportedError];
        }
    }
    
}

OSStatus RecordCallbackFunction (
                            void                        *inRefCon,
                            AudioUnitRenderActionFlags  *ioActionFlags,
                            const AudioTimeStamp        *inTimeStamp,
                            UInt32                      inBusNumber,
                            UInt32                      inNumberFrames,
                            AudioBufferList             *ioData)
{
    
    
    SOVoiceRecorder * ctxt = (SOVoiceRecorder*)inRefCon;
    
    AudioBufferList* abl = &ctxt->input_buffer_list;
    abl->mNumberBuffers = 1;
    abl->mBuffers[0].mNumberChannels = 1;
    abl->mBuffers[0].mData = NULL;
    abl->mBuffers[0].mDataByteSize = inNumberFrames * ctxt->asbd_mono_canonical_.mBytesPerFrame;
    
    OSStatus result = AudioUnitRender(ctxt->io_audio_unit_, 
                                      ioActionFlags, 
                                      inTimeStamp, 
                                      1, 
                                      inNumberFrames, 
                                      abl);
    
    if (result != noErr) {
        NSLog(@"Error rendering Audio!");
        return result;
    }
    
    // we could in fact only write into the file once we have not received ANY
    // silence
    
    //TODO: should we react to silence in here?
    
    result = ExtAudioFileWriteAsync(ctxt->file_target_,
                                    inNumberFrames,
                                    abl);
    
    if (result != noErr) {
        [SOVoiceRecorder printErrorMessage:@"While writing" 
                                withStatus:result];
    }
    
    return noErr;
}

- (id)init
{
    self = [super init];
    if (self) {
        
        isRecording = NO;
        file_target_ = NULL;
        
        AudioComponentDescription io_unit_descr;
        io_unit_descr.componentType = kAudioUnitType_Output;
        io_unit_descr.componentSubType = kAudioUnitSubType_VoiceProcessingIO;
//        io_unit_descr.componentSubType = kAudioUnitSubType_RemoteIO;
        io_unit_descr.componentManufacturer = kAudioUnitManufacturer_Apple;
        io_unit_descr.componentFlags = 0;
        io_unit_descr.componentFlagsMask = 0;
                
        //Find the Audio Unit (either RemoteIO or VoiceProcessing unit)
        io_audio_unit_ = NULL;
        AudioComponent comp = AudioComponentFindNext(NULL, &io_unit_descr);
        
        // Create an instance of the AudioUnit
        OSStatus result = AudioComponentInstanceNew(comp, &io_audio_unit_);
        [SOVoiceRecorder checkResult:result 
                         withMessage:@"Creating an instance of the IO audio unit."];
        
        UInt32 one = 1;
        UInt32 zero = 0;        
        
        // Enabling Input
        
        result = AudioUnitSetProperty(io_audio_unit_, 
                                      kAudioOutputUnitProperty_EnableIO, 
                                      kAudioUnitScope_Input, 
                                      1, 
                                      &one, 
                                      sizeof(UInt32));
        
        [SOVoiceRecorder checkResult:result 
                         withMessage:@"Enabling input on IO AU."];
        
        // Disabling Output
       
        result = AudioUnitSetProperty(io_audio_unit_, 
                                     kAudioOutputUnitProperty_EnableIO, 
                                     kAudioUnitScope_Output, 
                                     0, 
                                     &zero, 
                                     sizeof(UInt32));

        [SOVoiceRecorder checkResult:result 
                         withMessage:@"Disabling IO's output."];           
        
        // Setting a callback to receive errors from the IO AU
        result = AudioUnitAddPropertyListener(io_audio_unit_,
                                              kAudioUnitProperty_LastRenderError,
                                              ErrorCallbackHandler, 
                                              NULL);
        [SOVoiceRecorder checkResult:result 
                         withMessage:@"Installing Error Callback Handler."];
        
        
        // Setting voice processing specific properties
        result = AudioUnitSetProperty(io_audio_unit_, 
                                      kAUVoiceIOProperty_DuckNonVoiceAudio, 
                                      kAudioUnitScope_Global, 
                                      1, 
                                      &zero, 
                                      sizeof(UInt32));
        
        [SOVoiceRecorder checkResult:result 
                         withMessage:@"Disabling audio ducking."];
        
        
//        result = AudioUnitSetProperty(io_audio_unit_, 
//                                      kAUVoiceIOProperty_VoiceProcessingEnableAGC, 
//                                      kAudioUnitScope_Global, 
//                                      1, 
//                                      &zero, 
//                                      sizeof(UInt32));
//        
//        [SOVoiceRecorder checkResult:result 
//                         withMessage:@"Disabling AGC."];        
        
        // Setting the client stream format for the IO AU
        
        size_t bytesPerSample = sizeof (AudioSampleType);
        memset(&asbd_mono_canonical_, 0, sizeof(AudioStreamBasicDescription));
        
        asbd_mono_canonical_.mFormatID          = kAudioFormatLinearPCM;
        asbd_mono_canonical_.mFormatFlags       = kAudioFormatFlagsCanonical;
        asbd_mono_canonical_.mBytesPerPacket    = bytesPerSample;
        asbd_mono_canonical_.mBytesPerFrame     = bytesPerSample;
        asbd_mono_canonical_.mFramesPerPacket   = 1;
        asbd_mono_canonical_.mBitsPerChannel    = 8 * bytesPerSample;
        asbd_mono_canonical_.mChannelsPerFrame  = 1;
        asbd_mono_canonical_.mSampleRate        = 16000.0;
      
        result = AudioUnitSetProperty(io_audio_unit_, 
                                      kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Input, 
                                      0, 
                                      &asbd_mono_canonical_, 
                                      sizeof(asbd_mono_canonical_));
        
        [SOVoiceRecorder checkResult:result  
                         withMessage:@"Setting client stream format on output."];

        result = AudioUnitSetProperty(io_audio_unit_, 
                                      kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Output, 
                                      1, 
                                      &asbd_mono_canonical_, 
                                      sizeof(asbd_mono_canonical_));
        
        [SOVoiceRecorder checkResult:result  
                         withMessage:@"Setting client stream format on input."];        

        // Specify an input callback which will handle writing into the designated
        // ExtAudioFile instance.
        
        AURenderCallbackStruct rc;
        rc.inputProc = RecordCallbackFunction;
        rc.inputProcRefCon = (void*)self;
        
        result = AudioUnitSetProperty(io_audio_unit_, 
                                      kAudioOutputUnitProperty_SetInputCallback, 
                                      kAudioUnitScope_Global, 
                                      0, &rc, sizeof(AURenderCallbackStruct));
        
        [SOVoiceRecorder checkResult:result 
                         withMessage:@"Setting AU IO's input callback struct property."];        
        
        
        // Also build our destination ASBD for the ExtAudioFile services
        memset(&asbd_mono_aac_16khz_, 0, sizeof(AudioStreamBasicDescription));

        
//        asbd_mono_aac_16khz_.mFormatID = kAudioFormatMPEG4AAC;
//        asbd_mono_aac_16khz_.mFormatFlags = kMPEG4Object_AAC_Main;
        asbd_mono_aac_16khz_.mFormatID = kAudioFormatLinearPCM;        
        asbd_mono_aac_16khz_.mFormatFlags = kAudioFormatFlagsNativeEndian |
		kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
        asbd_mono_aac_16khz_.mBytesPerFrame = asbd_mono_aac_16khz_.mBytesPerPacket = 2;
        asbd_mono_aac_16khz_.mBitsPerChannel = 16;
        asbd_mono_aac_16khz_.mSampleRate = 16000.0;
        asbd_mono_aac_16khz_.mChannelsPerFrame = 1;
        asbd_mono_aac_16khz_.mFramesPerPacket = 1;

        // Initialize the IO AU
        
        result = AudioUnitInitialize(io_audio_unit_);
        [SOVoiceRecorder checkResult:result 
                         withMessage:@"Initializing the IO AU."];
    }
    
    return self;
}

- (void)dealloc
{
    
    [self stopRecording];    
    
    OSStatus result = AudioUnitRemovePropertyListenerWithUserData(io_audio_unit_, 
                                                                  kAudioUnitProperty_LastRenderError, 
                                                                  ErrorCallbackHandler, 
                                                                  NULL);
    
    [SOVoiceRecorder checkResult:result 
                     withMessage:@"Removing Error Callback Handler."];    
    
    result = AudioUnitUninitialize(io_audio_unit_);
    if (result != noErr) {
        [SOVoiceRecorder printErrorMessage:@"Uninitializing IO AU"
                                 withStatus:result];
    }
    
    result = AudioComponentInstanceDispose(io_audio_unit_);
    if (result != noErr) {
        [SOVoiceRecorder printErrorMessage:@"Uninitializing IO AU"
                                withStatus:result];
    }
    
    io_audio_unit_ = NULL;
    
    if (file_target_ != NULL) {
        OSStatus result = ExtAudioFileDispose(file_target_);
        [SOVoiceRecorder checkResult:result withMessage:@"ExtAudioFileDispose"];
    }
    
    [super dealloc];
}

- (void)startRecordingIntoFile:(NSURL*) url
{
    
    if ([self isRecording])
        [self stopRecording];
    
    // check file validity
    if (![url isFileURL]) {
        [NSException raise:@"Error" format:@"URL %@ is not a file-URL", url];
    }
    
    OSStatus result = ExtAudioFileCreateWithURL((CFURLRef)url, 
                                                //kAudioFileMPEG4Type, 
                                                kAudioFileCAFType,
                                                &asbd_mono_aac_16khz_,
                                                NULL, 
                                                kAudioFileFlags_EraseFile, 
                                                &file_target_);
    
    [SOVoiceRecorder checkResult:result 
                     withMessage:@"Create ExtAudioFileCreateWithURL"];
    
    result =  ExtAudioFileSetProperty(file_target_, 
                                      kExtAudioFileProperty_ClientDataFormat, 
                                      sizeof(asbd_mono_canonical_), 
                                      &asbd_mono_canonical_);
    
    [SOVoiceRecorder checkResult:result 
                     withMessage:@"Setting Ext Audio File client format."];        
    
    // initializes async writes
    result = ExtAudioFileWriteAsync(file_target_, 0, NULL);
    [SOVoiceRecorder checkResult:result withMessage:@"Initializ async writes."];
    
    // Actually start the IO unit to render
    result = AudioOutputUnitStart(io_audio_unit_);
    [SOVoiceRecorder checkResult:result withMessage:@"Starting the IO AU"];      
    
    isRecording = YES;
    
    NSLog(@"Started Recording.");
    NSLog(@"Will write into: %@", url);
    
}

- (void)stopRecording
{
    if (![self isRecording])
        return;
    
    OSStatus result = AudioOutputUnitStop(io_audio_unit_);
    [SOVoiceRecorder checkResult:result withMessage:@"Stopping the IO AU"];
    
    [SOVoiceRecorder checkResult:result 
                     withMessage:@"Removing render callback."];
    
    result = ExtAudioFileDispose(file_target_);
    [SOVoiceRecorder checkResult:result withMessage:@"Disposing ExtAudioFile."];
    
    file_target_ = NULL;
    
    isRecording = NO;
    
    NSLog(@"Stopped Recording");
    
}

+ (void) checkResult:(OSStatus) result withMessage:(NSString*) msg
{
    
    if (result != noErr) {
        [SOVoiceRecorder printErrorMessage:msg withStatus:result];
        [NSException raise:@"OSStatusCheck" 
                    format:@"Checking the return code of an operation failed: '%@'", msg];
    }
}


+ (void) printErrorMessage: (NSString *) errorString 
                withStatus: (OSStatus) result 
{
    
    char resultString[5];
    UInt32 swappedResult = CFSwapInt32HostToBig (result);
    bcopy (&swappedResult, resultString, 4);
    resultString[4] = '\0';
    
    NSLog (
           @"*** %@ error: %d %08X %4.4s\n",
           errorString,
           (int)result,
           (unsigned int)result,
           (char*) &resultString
           );    
}

@end
