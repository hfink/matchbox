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

#import <UIKit/UIKit.h>

@interface IPViewController : UIViewController {
    
    UIProgressView *ibBenchmarkProgress_;
    UILabel *ibTotalDuration_;
    UILabel *ibAvgDurationPerSong_;
    UIButton *ibBenchmarkButton_;
    UILabel *ibTitleLabel_;
    UIActivityIndicatorView *ibActivityIndicator_;
    
    BOOL isExecutingBenchmark;
    
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
