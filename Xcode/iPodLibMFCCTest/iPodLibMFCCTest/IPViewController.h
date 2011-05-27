//
//  IPViewController.h
//  IP
//
//  Created by Heinrich Fink on 5/18/11.
//  Copyright 2011 Heinrich Fink. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface IPViewController : UIViewController {
    
    UIProgressView *ibBenchmarkProgress_;
    UILabel *ibTotalDuration_;
    UILabel *ibAvgDurationPerSong_;
    UIButton *ibBenchmarkButton_;
    UILabel *ibTitleLabel_;
    UIActivityIndicatorView *ibActivityIndicator_;
    
    BOOL isExecutingBenchmark;
    NSOperationQueue* song_processing_queue;
    NSOperation* song_processing_operation;
    UILabel *ibHeaderLabel_;
    UILabel *ibCounterLabel;
    UILabel *ibMfccAvgLabel;
    UILabel *ibArtistLabel;
    
    Float64 samplingRate;
    NSUInteger mfccWindowSize;
    Float64 mfccMelMax;
    UISegmentedControl *ibModeSelector;
    UISegmentedControl *ibChannelSwitch;
    NSUInteger numChannelsRequest;
}

- (IBAction)ibChangeChannelMode:(id)sender;
- (IBAction)ibChangeMode:(id)sender;
- (IBAction)startOrStopBenchmark:(id)sender;

@property (nonatomic, retain) IBOutlet UISegmentedControl *ibChannelSwitch;
@property (nonatomic, retain) IBOutlet UISegmentedControl *ibModeSelector;
@property (nonatomic, assign) Float64 samplingRate;
@property (nonatomic, assign) NSUInteger mfccWindowSize;
@property (nonatomic, assign) Float64 mfccMelMax;
@property (nonatomic, assign) NSUInteger numChannelsRequest;

@property (nonatomic, retain) IBOutlet UILabel *ibHeaderLabel;
@property (nonatomic, retain) IBOutlet UILabel *ibMfccAvgLabel;

@property (nonatomic, retain) IBOutlet UILabel *ibTitleLabel;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView *ibActivityIndicator;
@property (nonatomic, retain) IBOutlet UILabel *ibAvgDurationPerSong;
@property (nonatomic, retain) IBOutlet UIButton *ibBenchmarkButton;
@property (nonatomic, retain) IBOutlet UILabel *ibArtistLabel;

@property (nonatomic, retain) IBOutlet UILabel *ibTotalDuration;
@property (nonatomic, retain) IBOutlet UIProgressView *ibBenchmarkProgress;
@property (nonatomic, retain) IBOutlet UILabel *ibCounterLabel;

@end
