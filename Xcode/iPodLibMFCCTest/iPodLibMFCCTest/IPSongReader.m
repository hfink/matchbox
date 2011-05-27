//
//  IPSongReader.m
//  iPodLibMFCCTest
//
//  Created by Heinrich Fink on 5/20/11.
//  Copyright 2011 Heinrich Fink. All rights reserved.
//

#import "IPSongReader.h"

@implementation IPSongReader

@synthesize assetReader = assetReader_;
@synthesize assetOutput = assetOutput_;

- (id)initWithURL:(NSURL*)url 
      forDuration:(float)seconds 
     samplingRate:(Float64)samplingRate
        withBlock:(BOOL (^)(CMSampleBufferRef)) consumer_block {
    
    if ((self = [super init])) { // equivalent to "self does not equal nil"
        
        consumer_block_ = consumer_block;
        
        NSDictionary* options = [[NSDictionary alloc] initWithObjectsAndKeys:NO, AVURLAssetPreferPreciseDurationAndTimingKey, nil];
        
        AVURLAsset* url_asset = [[AVURLAsset alloc] initWithURL:url options:options];
        
        [options release];
        
        //Note: it's ok if we block here, this class shall be used on a 
        //worker thread anyways
        
        //TODO: setting AVSampleRateConverterAudioQualityKey to low might help
        //performance in here...
        
        //This currently is the mixdown option with the same
        //Probably you can ask this from a preference panel setup stuff...
//		NSDictionary *audio_settings = [NSDictionary dictionaryWithObjectsAndKeys:
//									  [NSNumber numberWithFloat:44100.0],AVSampleRateKey,
//                                      //This should force mixdown to mono
//									  [NSNumber numberWithInt:1],AVNumberOfChannelsKey,
//									  [NSNumber numberWithInt:32],AVLinearPCMBitDepthKey,
//									  [NSNumber numberWithInt:kAudioFormatLinearPCM], AVFormatIDKey,
//									  [NSNumber numberWithBool:YES], AVLinearPCMIsFloatKey,
//                                      //For Max architextures, this has to be changed!
//									  [NSNumber numberWithBool:0], AVLinearPCMIsBigEndianKey,
//									  [NSNumber numberWithBool:YES], AVLinearPCMIsNonInterleaved,
//									  [NSData data], AVChannelLayoutKey, nil];  
        
        
        NSError * error = nil;
        
        AVAssetReader* asset_reader = [[AVAssetReader alloc] initWithAsset:url_asset 
                                                                     error:&error];
        
        [url_asset release];
        
        self.assetReader = asset_reader;
        
        [asset_reader release];
        
        if (error != nil) {
            NSLog(@"There was an error create the the AssetReader: '%@'.", 
                  [error localizedDescription]);
            [self release];
            return nil;
        }
        
		NSDictionary *audio_settings = [[NSDictionary alloc] initWithObjectsAndKeys:
                                        [NSNumber numberWithFloat:samplingRate],AVSampleRateKey,
                                        [NSNumber numberWithInt:2],AVNumberOfChannelsKey,	//how many channels has original? 
                                        [NSNumber numberWithInt:32],AVLinearPCMBitDepthKey, //was 16
                                        [NSNumber numberWithInt:kAudioFormatLinearPCM], AVFormatIDKey,
                                        [NSNumber numberWithBool:YES], AVLinearPCMIsFloatKey,  //was NO
                                        [NSNumber numberWithBool:0], AVLinearPCMIsBigEndianKey,
                                        [NSNumber numberWithBool:NO], AVLinearPCMIsNonInterleaved,
                                        [NSData data], AVChannelLayoutKey, nil];              
        
		AVAssetReaderAudioMixOutput * asset_output = 
            [[AVAssetReaderAudioMixOutput alloc] initWithAudioTracks:[url_asset tracksWithMediaType:AVMediaTypeAudio]
                                                       audioSettings:audio_settings];    
        
        [audio_settings release];
        
        self.assetOutput = asset_output;
        
        [asset_output release];
        
        // Set the proper time range
        CMTime read_duration = CMTimeMakeWithSeconds(seconds, 600);
        
        Float64 total_duration_secs = CMTimeGetSeconds([url_asset duration]);
        
//        NSLog(@"Total duration is %f", total_duration_secs);
        
        CMTime start_read_time = CMTimeMakeWithSeconds(total_duration_secs*0.5 - seconds*0.5, 600);
        
        CMTimeRange read_range;
        read_range.start = start_read_time;
        read_range.duration = read_duration;
        
        //setting the proper range to read
        asset_reader.timeRange = read_range;
        
        if (![asset_reader canAddOutput:asset_output]) {
            NSLog(@"Cannot add asset output to asset reader.");
            [self release];
            return nil;
        }
        
        [asset_reader addOutput:asset_output];
        
    }
    
    return self;        
    
}

- (BOOL)consumeRange {
    
    if ([assetReader_ startReading] != YES) {
        NSLog(@"Error: Can't start reading, because:");
        if ([assetReader_ status] == AVAssetReaderStatusCompleted) {
            NSLog(@"    Reader has no samples to read.");
        } else if ([assetReader_ status] == AVAssetReaderStatusCancelled) {
            NSLog(@"    Reader has been canceled.");
        } else if ([assetReader_ status] == AVAssetReaderStatusFailed) {
            NSLog(@"    Reader encountered an error: '%@'", 
                  [[assetReader_ error] localizedDescription]);
        }
        return NO;
    }
    
    CMSampleBufferRef sample_buffer = [assetOutput_ copyNextSampleBuffer];
    
    while (sample_buffer != NULL) {
        
        BOOL consumption_success = consumer_block_(sample_buffer);
        
        if (!consumption_success) {
            NSLog(@"Consumption failed, aborting reading process.");
            [assetReader_ cancelReading];
            break;
        }
        
        //Release the previously owned sample buffer
        CFRelease(sample_buffer);
        sample_buffer = [assetOutput_ copyNextSampleBuffer];
    }
    
    //Check state, should be finished, or canceled on error
    if ([assetReader_ status] == AVAssetReaderStatusCancelled) {
        NSLog(@"Reader has been canceled, probably due to a consumption error.");
        return NO;
    } else if ([assetReader_ status] == AVAssetReaderStatusFailed) {
        NSLog(@"Reader encountered an error: '%@'", 
              [[assetReader_ error] localizedDescription]);
        return NO;
    } else if ([assetReader_ status] == AVAssetReaderStatusUnknown) {
        NSLog(@"Reader has encountered an unknown error.");
        return NO;
    } else if ([assetReader_ status] == AVAssetReaderStatusReading) {
        NSLog(@"copyNextBuffer returned NULL, but reader is still reading.");
        return NO;
    }
    
    //sanity check
    if ([assetReader_ status] != AVAssetReaderStatusCompleted) {
        NSLog(@"AssetReader finished reading without an error, but process \
                was also not completed.");
    }
    
    return YES;
}

- (void)cancel {
    [assetReader_ cancelReading];
}

- (void)dealloc
{
    [assetReader_ release];
    [assetOutput_ release];
    [super dealloc];
}



@end
