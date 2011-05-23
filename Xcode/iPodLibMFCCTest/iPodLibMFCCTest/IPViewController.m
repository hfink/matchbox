//
//  IPViewController.m
//  IP
//
//  Created by Heinrich Fink on 5/18/11.
//  Copyright 2011 Heinrich Fink. All rights reserved.
//

#import "IPViewController.h"

#import <MediaPlayer/MediaPlayer.h>

#include <dispatch/dispatch.h>

#include <mach/mach_time.h>

#include "WordMatch.h"

#include "IPSongReader.h"

@implementation IPViewController
@synthesize ibHeaderLabel = ibHeaderLabel_;
@synthesize ibMfccAvgLabel;
@synthesize ibTitleLabel = ibTitleLabel_;
@synthesize ibActivityIndicator = ibActivityIndicator_;
@synthesize ibAvgDurationPerSong = ibAvgDurationPerSong_;
@synthesize ibBenchmarkButton = ibBenchmarkButton_;
@synthesize ibArtistLabel;
@synthesize ibTotalDuration = ibTotalDuration_;
@synthesize ibBenchmarkProgress = ibBenchmarkProgress_;
@synthesize ibCounterLabel;

- (void)dealloc
{
    [ibBenchmarkProgress_ release];
    [ibTotalDuration_ release];
    [ibAvgDurationPerSong_ release];
    [ibActivityIndicator_ release];
    [ibTitleLabel_ release];
    [ibBenchmarkButton_ release];
    [ibTitleLabel_ release];
    [ibHeaderLabel_ release];
    [ibCounterLabel release];
    [ibMfccAvgLabel release];
    [ibArtistLabel release];
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)hideRunningLabels
{
    [self.ibArtistLabel setHidden:YES];
    [self.ibTitleLabel setHidden:YES];    
    [self.ibMfccAvgLabel setHidden:YES];        
}

- (void)showRunningLabels
{
    [self.ibArtistLabel setHidden:NO];
    [self.ibTitleLabel setHidden:NO];    
    [self.ibMfccAvgLabel setHidden:NO];        
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
    isExecutingBenchmark = NO;
    song_processing_queue = [[NSOperationQueue alloc] init];
    [song_processing_queue setName:@"SongProcessingQueue"];
    [self hideRunningLabels];
}


- (void)viewDidUnload
{
    [self setIbBenchmarkProgress:nil];
    [self setIbTotalDuration:nil];
    [self setIbAvgDurationPerSong:nil];
    [self setIbActivityIndicator:nil];
    [self setIbTitleLabel:nil];
    [self setIbBenchmarkButton:nil];
    [self setIbTitleLabel:nil];
    [self setIbHeaderLabel:nil];
    [self setIbCounterLabel:nil];
    [self setIbMfccAvgLabel:nil];
    [self setIbArtistLabel:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
    [song_processing_queue release];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (IBAction)startOrStopBenchmark:(id)sender {
    
    if (isExecutingBenchmark) {
        
        //stop the benchmark process
        [song_processing_queue cancelAllOperations];
        
    } else {
        
        [self.ibBenchmarkButton setTitle:@"Stop Benchmark" forState:UIControlStateNormal];        
        isExecutingBenchmark = YES;
        
        //Kick off media query. We will put this into a background operation
        //running somewhere else...
        
        song_processing_operation = [NSBlockOperation blockOperationWithBlock:^{
            
            //Initialization the MFCC Session provided by our WordMatch lib
            
            static const float mfcc_duration = 30.0f;            
            
            float * mfcc_average = malloc(sizeof(float)*13);
            memset(mfcc_average, 0, sizeof(float)*13);
            
            WMSessionRef mfcc_session;            
            WMMfccConfiguration mfcc_config;
            mfcc_config.mel_min_freq = 20.0f;
            mfcc_config.mel_max_freq = 15000.0f;
            mfcc_config.sampling_rate = 44100.0f;
            mfcc_config.pre_empha_alpha = 0.97f;
            mfcc_config.window_size = 1024;            
            
            WMSessionResult result = WMSessionCreate(mfcc_duration, 
                                                     mfcc_config, 
                                                     &mfcc_session);
            
            if (result != kWMSessionResultOK) {
                NSLog(@"WMSession returned %hd, exiting calculation", result);
                return;
            }            
            
            dispatch_queue_t main_queue = dispatch_get_main_queue();            
            
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            
            MPMediaPropertyPredicate *music_type_filter = 
            [MPMediaPropertyPredicate predicateWithValue: [NSNumber numberWithInt:MPMediaTypeMusic]
                                             forProperty: MPMediaItemPropertyMediaType];       
            
            NSSet* predicate_set = [NSSet setWithObject:music_type_filter];
            
            MPMediaQuery* songs_query = [[MPMediaQuery alloc] initWithFilterPredicates:predicate_set];
            
            NSArray* all_songs = [songs_query items];
            
            dispatch_async(main_queue, ^{  
                
                NSString * str = [[NSString alloc] initWithFormat:@"Reading %u songs", [all_songs count]];        
                
                [self.ibHeaderLabel setText:str];
                
                [str release];
                
            });            
            
            double duration = 0;
            
            int songs_counter = 0;
            
            uint64_t now = mach_absolute_time();
            double song_processing_start = WMMachTimeToMilliSeconds(now);                            
            double song_processing_end = 0;
            
            BOOL was_canceled = NO;
            BOOL was_error = NO;            
            
            for (MPMediaItem* song in all_songs) {
            
                //Resetting the session
                result = WMSessionReset(mfcc_session);
                if (result != kWMSessionResultOK) {
                    NSLog(@"Could not reset WMSessin: %hd", result);
                    was_error = YES;
                    break;
                }
                
                NSString * title = [song valueForProperty:MPMediaItemPropertyTitle];
                NSString * artist = [song valueForProperty:MPMediaItemPropertyArtist];                
                NSURL* song_url = [song valueForProperty:MPMediaItemPropertyAssetURL];
                
                if (song_url == nil) {
                    
                    NSLog(@"Can not access MPMediaItemPropertyAssetURL for song '%@'.", title);
                    duration = 0;
                    
                } else {
                    
                    IPSongReader* song_reader = [[IPSongReader alloc] initWithURL:song_url 
                                                                      forDuration:mfcc_duration 
                                                                        withBlock:^BOOL(CMSampleBufferRef sample_buffer) {
                                                                            
                                                                            if (!WMSessionIsCompleted(mfcc_session)) {
                                                                                WMSessionResult mfcc_result = WMSessionFeedFromSampleBuffer(sample_buffer, mfcc_session);
                                                                                if (mfcc_result != kWMSessionResultOK) {
                                                                                    NSLog(@"MFCC calculation returned an error: %hd", mfcc_result);
                                                                                    return NO;
                                                                                }
                                                                            } else {
                                                                                NSLog(@"Session was already completed.");
                                                                            }
                                                                            
                                                                            return TRUE;
                                                                        }];
                    
                    
                    //Make the paths below easier to maintain by additn try catch finally
                    if (song_reader == nil) {
                        NSLog(@"Song reader initialization failed.");
                        was_error = YES;
                        break;
                    }
                    
                    dispatch_async(main_queue, ^{                      
                    
                        [self showRunningLabels];
                        
                        [self.ibTitleLabel setText:title];
                        [self.ibArtistLabel setText:artist];
                        [self.ibBenchmarkProgress setProgress:songs_counter/(float)[all_songs count]];
                        
                        NSString* str = [[NSString alloc] initWithFormat:@"%i / %u", songs_counter, [all_songs count]];                    
                        [self.ibCounterLabel setText:str];                    
                        [str release];                    
                        
                    });
                    
                    
                    //Start processing that stuff
                    BOOL success = [song_reader consumeRange];
                    if (!success) {
                        NSLog(@"Could not consume the song properly.");
                        was_error = YES;
                        [song_reader release];
                        break;
                    }
                    
                    //Get the average MFCC values
                    result = WMSessionGetAverage(mfcc_average, mfcc_session);
                    if (result != kWMSessionResultOK) {
                        NSLog(@"WMSessionGetAverage returned an error: %hd", result);
                        was_error = YES;
                        [song_reader release];
                        break;
                    }
                    
                    song_processing_end = WMMachTimeToMilliSeconds(mach_absolute_time());
                    
                    duration = (song_processing_end - song_processing_start)*1e-3;
                    
                    [song_reader release];                    
                }
                
                songs_counter++;                
                
                //Update GUI information
                
                dispatch_async(main_queue, ^{  
                    
                    double avg_song_time = duration / (double)songs_counter;
                    
                    NSString * str = [[NSString alloc] initWithFormat:@"%.2f sec", avg_song_time];
                    [self.ibAvgDurationPerSong setText:str];
                    [str release];
                    
                    str = [[NSString alloc] initWithFormat:@"%.2f sec", duration];
                    [self.ibTotalDuration setText:str];
                    [str release];
                    
                    str = [[NSString alloc] initWithFormat:@"%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f", 
                                                           mfcc_average[0], 
                                                           mfcc_average[1],
                                                           mfcc_average[2],
                                                           mfcc_average[3],
                                                           mfcc_average[4],
                                                           mfcc_average[5],
                                                           mfcc_average[6],
                                                           mfcc_average[7],
                                                           mfcc_average[8],
                                                           mfcc_average[9],
                                                           mfcc_average[10],
                                                           mfcc_average[11],
                                                           mfcc_average[12]];                    
                    [self.ibMfccAvgLabel setText:str];                    
                    
                    [str release];                    
                    
                });
                
                //Check early exit
                
                if ([song_processing_operation isCancelled] == YES) {
                    was_canceled = YES;
                    break;
                }
                
            }            
            
            dispatch_async(main_queue, ^{  
                
                NSString * str = [[NSString alloc] initWithFormat:@"%.2f sec", duration];
                [self.ibTotalDuration setText:str];
                [str release];
                
                if (was_canceled)
                    [self.ibHeaderLabel setText:@"User canceled benchmark"];
                else if (was_error)
                    [self.ibHeaderLabel setText:@"Error in benchmark process"];                    
                else
                    [self.ibHeaderLabel setText:@"All song were processed successfully"];                    
                
                isExecutingBenchmark = NO;
                
                [self hideRunningLabels];
                
                [self.ibBenchmarkButton setTitle:@"Run Benchmark" forState:UIControlStateNormal];
                [self.ibBenchmarkProgress setProgress:.0f];
                [self.ibActivityIndicator stopAnimating];
                
            });           
            
            [songs_query release];
            [pool release];                  
            result = WMSessionDestroy(mfcc_session);
            if (result != kWMSessionResultOK)
                NSLog(@"After destruction, WMSession returned %hd", result);
            
            free(mfcc_average);
            
        }];
        
        [song_processing_queue addOperation:song_processing_operation];
        
        [self.ibActivityIndicator startAnimating];
        [self.ibBenchmarkProgress setProgress:.0f];
                
    }
    
}
@end
