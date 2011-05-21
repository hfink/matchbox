//
//  IPViewController.m
//  IP
//
//  Created by Heinrich Fink on 5/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "IPViewController.h"

#import <MediaPlayer/MediaPlayer.h>

#include <dispatch/dispatch.h>

#include "WordMatch.h"

@implementation IPViewController
@synthesize ibHeaderLabel = ibHeaderLabel_;
@synthesize ibTitleLabel = ibTitleLabel_;
@synthesize ibActivityIndicator = ibActivityIndicator_;
@synthesize ibAvgDurationPerSong = ibAvgDurationPerSong_;
@synthesize ibBenchmarkButton = ibBenchmarkButton_;
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
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
    isExecutingBenchmark = NO;
    song_processing_queue = [[NSOperationQueue alloc] init];
    [song_processing_queue setName:@"SongProcessingQueue"];
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
                
            });            
            
            double duration = 0;
            
            int songs_counter = 0;
            
            double song_processing_start = WMMachTimeToMilliSeconds(mach_absolute_time());                            
            double song_processing_end = 0;
            
            BOOL was_canceled = NO;
            
            for (MPMediaItem* song in all_songs) {
                
                //TODO: create IP Song reader in here, define block
                NSString * title = [song valueForProperty:MPMediaItemPropertyTitle];
                
                //just do something for now...
                [NSThread sleepForTimeInterval:0.1];
                
                song_processing_end = WMMachTimeToMilliSeconds(mach_absolute_time());
                
                duration = (song_processing_end - song_processing_start)*1e-3;
                
                songs_counter++;                
                
                //Update GUI information
                
                dispatch_async(main_queue, ^{  
                    
                    double avg_song_time = duration / (double)songs_counter;
                    NSString * str = [[NSString alloc] initWithFormat:@"%.2f sec", avg_song_time];
                    [self.ibAvgDurationPerSong setText:str];
                    [str release];
                    
                    [self.ibTitleLabel setText:title];
                    [self.ibBenchmarkProgress setProgress:songs_counter/(float)[all_songs count]];
                    
                    str = [[NSString alloc] initWithFormat:@"%i / %u", songs_counter, [all_songs count]];                    
                    
                    [self.ibCounterLabel setText:str];
                    
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
                
                [self.ibTitleLabel setText:@"User canceled benchmark"];
                
                isExecutingBenchmark = NO;
                
                [self.ibBenchmarkButton setTitle:@"Run Benchmark" forState:UIControlStateNormal];
                [self.ibBenchmarkProgress setProgress:.0f];
                [self.ibActivityIndicator stopAnimating];
                
            });           
            
            [songs_query release];
            [pool release];                        
        }];
        
        [song_processing_queue addOperation:song_processing_operation];
        
        [self.ibActivityIndicator startAnimating];
        [self.ibBenchmarkProgress setProgress:.0f];
                
    }
    
}
@end
