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

#import <UIKit/UIKit.h>
#import "SOVoiceRecorder.h"
#include "WordMatch.h"

@interface SOAppDelegate : NSObject <UIApplicationDelegate, AVAudioPlayerDelegate> {
    
    SOVoiceRecorder* voice_recorder_;
    
    UIButton *quantitativeResultButton;
    UILabel *quantitativeResultLabel;
    UILabel *thresholdLabel;
    UIActivityIndicatorView *activityIndicatorA;
    UIActivityIndicatorView *activityIndicatorB;
    UISlider *thresholdSlider;
    
    BOOL useClippedTimes;
    float dtwThreshold;
    UISlider *beginThresholdSlider;
    float clipBeginThresholdDB;
    float clipEndThresholdDB;    
    
    UIButton *playButtonA;
    UIButton *recordButtonA;
    UIButton *playButtonB;
    UIButton *recordButtonB;
    UIActivityIndicatorView *activityIndicatorDtw;
    UILabel *qualitativeResultLabel;
    
    WMAudioFilePreProcessInfo file_a_info;
    WMAudioFilePreProcessInfo file_b_info;    
    UILabel *beginThresholdLabel;
    UILabel *endThresholdLabel;
    UISlider *endThresholdSlider;
}

@property (nonatomic, retain) IBOutlet UILabel *beginThresholdLabel;
@property (nonatomic, retain) IBOutlet UILabel *endThresholdLabel;

@property (nonatomic, retain) IBOutlet UISlider *endThresholdSlider;
@property (nonatomic, assign) float dtwThreshold;
@property (nonatomic, retain) IBOutlet UISlider *beginThresholdSlider;

@property (nonatomic, assign) float clipBeginThresholdDB;
@property (nonatomic, assign) float clipEndThresholdDB;

@property (nonatomic, assign) BOOL useClippedTimes;

@property (nonatomic, retain) NSURL * fileA;
@property (nonatomic, retain) NSURL * fileB;

@property (nonatomic, retain) IBOutlet UILabel *qualitativeResultLabel;
@property (nonatomic, retain) AVAudioPlayer *playerA;
@property (nonatomic, retain) AVAudioPlayer *playerB;
@property (nonatomic, retain) IBOutlet UIButton *playButtonA;
@property (nonatomic, retain) IBOutlet UIButton *recordButtonA;
@property (nonatomic, retain) IBOutlet UIButton *playButtonB;
@property (nonatomic, retain) IBOutlet UIButton *recordButtonB;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView *activityIndicatorDtw;

@property (nonatomic, retain) IBOutlet UIWindow *window;

@property (nonatomic, retain) IBOutlet UILabel *quantitativeResultLabel;
@property (nonatomic, retain) IBOutlet UILabel *thresholdLabel;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView *activityIndicatorA;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView *activityIndicatorB;
@property (nonatomic, retain) IBOutlet UISlider *thresholdSlider;

- (IBAction)playFileA:(id)sender;
- (IBAction)recordFileA:(id)sender;
- (IBAction)endEditingBeginThresholdDB:(id)sender;
- (IBAction)endEditingEndThresholdDB:(id)sender;

- (IBAction)playFileB:(id)sender;
- (IBAction)recordFileB:(id)sender;

- (IBAction)switchSpeakerMode:(id)sender;
- (IBAction)switchUseClippedTimes:(id)sender;

- (IBAction)executeComparison:(id)sender;
- (IBAction)changeTreshold:(id)sender;
- (IBAction)changeEndThresholdDB:(id)sender;

- (IBAction)changeBeginThresholdDB:(id)sender;

@end
