//
//  IPViewController.h
//  IP
//
//  Created by Heinrich Fink on 5/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
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
}
@property (nonatomic, retain) IBOutlet UILabel *ibHeaderLabel;

@property (nonatomic, retain) IBOutlet UILabel *ibTitleLabel;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView *ibActivityIndicator;
@property (nonatomic, retain) IBOutlet UILabel *ibAvgDurationPerSong;
@property (nonatomic, retain) IBOutlet UIButton *ibBenchmarkButton;

@property (nonatomic, retain) IBOutlet UILabel *ibTotalDuration;
@property (nonatomic, retain) IBOutlet UIProgressView *ibBenchmarkProgress;
@property (nonatomic, retain) IBOutlet UILabel *ibCounterLabel;

- (IBAction)startOrStopBenchmark:(id)sender;

@end
