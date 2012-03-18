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

#import "SOAppDelegate.h"

#include "WordMatch.h"

@implementation SOAppDelegate

@synthesize beginThresholdLabel;
@synthesize endThresholdLabel;
@synthesize endThresholdSlider;
@synthesize dtwThreshold;
@synthesize beginThresholdSlider;

@synthesize window=_window;
@synthesize fileA;
@synthesize fileB;
@synthesize qualitativeResultLabel;
@synthesize playerA;
@synthesize playerB;
@synthesize playButtonA;
@synthesize recordButtonA;
@synthesize playButtonB;
@synthesize recordButtonB;
@synthesize activityIndicatorDtw;
@synthesize quantitativeResultLabel;
@synthesize thresholdLabel;
@synthesize activityIndicatorA;
@synthesize activityIndicatorB;
@synthesize thresholdSlider;
@synthesize useClippedTimes;
@synthesize clipBeginThresholdDB;
@synthesize clipEndThresholdDB;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
    [self.window makeKeyAndVisible];
    
    [self setPlayerA:nil];
    [self setPlayerB:nil];
    
    //TODO: handle the interruption delegates properly, i.e. what happens if
    //your are about to record a file, and then someone calls...
    
    // configuring AVSession
    NSError *audioSessionError = nil;
    AVAudioSession *mySession = [AVAudioSession sharedInstance];    
    [mySession setPreferredHardwareSampleRate: 16000.0      
                                        error: &audioSessionError];
    [mySession setCategory: AVAudioSessionCategoryPlayAndRecord     
                     error: &audioSessionError];
    [mySession setActive: YES                                       
                   error: &audioSessionError];
    
    if (audioSessionError != nil) {
        NSLog(@"Error occured during AVSession configuration: %@", 
              audioSessionError);
    }
        
    voice_recorder_ = [[SOVoiceRecorder alloc] init];
    
    //TODO: check what happens if you have connected with headphones...
    
    UInt32 route = kAudioSessionOverrideAudioRoute_Speaker; 
    OSStatus result = AudioSessionSetProperty (kAudioSessionProperty_OverrideAudioRoute,
                                               sizeof (route),
                                               &route);
    
    if (result != noErr) {
        NSLog(@"Error setting audio route to speaker during initialization.");
    }       
    
    
    // we set the current file in our app's documents directory
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, 
                                                         NSUserDomainMask, 
                                                         YES);
    
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *fileAString = [NSString stringWithFormat:@"%@/file_a.caf", documentsDirectory];
    NSURL* url_a = [NSURL fileURLWithPath:fileAString];
    [self setFileA:url_a];
    
    NSString *fileBString = [NSString stringWithFormat:@"%@/file_b.caf", documentsDirectory];
    NSURL* url_b = [NSURL fileURLWithPath:fileBString];
    [self setFileB:url_b];    
    
    
    dtwThreshold = [thresholdSlider value];
    [self setClipBeginThresholdDB:[[self beginThresholdSlider] value]];
    [self setClipEndThresholdDB:[[self endThresholdSlider] value]];
    
    //If we have no previous sound files in our user directory, copy
    //the reference ones.
    NSBundle * main_bundle = [NSBundle mainBundle];
    NSFileManager * file_manager = [[NSFileManager alloc] init];
    
//    BOOL file_a_exists = [url_a checkResourceIsReachableAndReturnError:nil];
    BOOL file_a_exists = [file_manager fileExistsAtPath:[url_a path]];
    if (!file_a_exists) {
        NSLog(@"Copying stock-version of fileA.");
        NSURL * file_a_stock = [main_bundle URLForResource:@"file_a.caf" 
                                             withExtension:nil];
        if (file_a_stock == nil) {
            NSLog(@"Could not locate stock reference file A.");
        } else {
            BOOL success = [file_manager copyItemAtURL:file_a_stock 
                                                 toURL:url_a 
                                                 error:NULL];
            
            if (!success)
                NSLog(@"Could not perform copy operation for stock-file A.");
            
        }
        
    } else {
        
        bool success = WMGetPreProcessInfoForFile((CFURLRef)[self fileA], 
                                                  [self clipBeginThresholdDB], 
                                                  [self clipEndThresholdDB], 
                                                  &file_a_info);
        
        NSLog(@"For file %@", [self fileA]);
        NSLog(@"Peak: %f", file_a_info.max_peak);
        NSLog(@"Begin: %f", file_a_info.threshold_start_time);        
        NSLog(@"End: %f", file_a_info.threshold_end_time);                
        NSLog(@"Norm Factor: %f", file_a_info.normalization_factor);                        
        
        if (!success) {
            NSLog(@"Could not calculate pre process info on startup for file A");
        }
        
    }
    
    //same thing for file b
    
    BOOL file_b_exists = [file_manager fileExistsAtPath:[url_b path]];
    if (!file_b_exists) {
        NSLog(@"Copying stock-version of fileB.");
        NSURL * file_b_stock = [main_bundle URLForResource:@"file_b.caf" 
                                             withExtension:nil];
        if (file_b_stock == nil) {
            NSLog(@"Could not locate stock reference file B.");
        } else {
            BOOL success = [file_manager copyItemAtURL:file_b_stock 
                                                 toURL:url_b 
                                                 error:NULL];
            
            if (!success)
                NSLog(@"Could not perform copy operation for stock-file B.");
            
        }
        
    } else {
        bool success = WMGetPreProcessInfoForFile((CFURLRef)[self fileB], 
                                                  [self clipBeginThresholdDB], 
                                                  [self clipEndThresholdDB], 
                                                  &file_b_info);
        
        if (!success) {
            NSLog(@"Could not calculate pre process info on startup for file B");
        }        
    }
    
    [file_manager release];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    /*
     Called when the application is about to terminate.
     Save data if appropriate.
     See also applicationDidEnterBackground:.
     */
}

- (void)dealloc
{
    [voice_recorder_ release];    
    [_window release];
    [qualitativeResultLabel release];
    [thresholdLabel release];
    [activityIndicatorA release];
    [activityIndicatorB release];
    [thresholdSlider release];
    [playButtonA release];
    [recordButtonA release];
    [playButtonB release];
    [recordButtonB release];
    [activityIndicatorDtw release];
    [beginThresholdLabel release];
    [endThresholdLabel release];
    [beginThresholdSlider release];
    [endThresholdSlider release];
    [super dealloc];
}

- (IBAction)playFileB:(id)sender 
{
    if ([self playerB] == nil)
    {
        NSError* err = nil;
        AVAudioPlayer* avPlayer = 
        [[AVAudioPlayer alloc] initWithContentsOfURL:[self fileB] 
                                               error:&err];
        
        [self setPlayerB:avPlayer];
        
        [avPlayer release];  
        
        if (err != nil) {
            NSLog(@"Error creating the AVPlayer: %@", [err localizedDescription]);
            return;
        }
        
        if (![[self playerB] prepareToPlay]) {
            [NSException raise:@"error" format:@"Cannot prepare playerB to play."];
            return;
        }
        
        [[self playerB] setDelegate:self];
        [[self playerB] setVolume:1.0];
        
        if ([self useClippedTimes] && (file_b_info.max_peak != 0)) {
            NSTimeInterval start_time = file_b_info.threshold_start_time;
            NSTimeInterval duration = file_b_info.threshold_end_time - file_b_info.threshold_start_time;
            [[self playerB] setCurrentTime:start_time];
            NSTimer* timer = [NSTimer timerWithTimeInterval:duration target:self selector:@selector(timerStopFileB:) userInfo:nil repeats:NO];
            
            [[self playerB] play];
            [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
            
        } else {
            [[self playerB] play];            
        }
        

        [activityIndicatorB startAnimating];
        
        [playButtonB setTitle:@"Stop" forState:UIControlStateNormal];
        [recordButtonB setEnabled:NO];
        
        
    } else {
        
        [[self playerB] stop];
        [activityIndicatorB stopAnimating];
        [self setPlayerB:nil];
        [playButtonB setTitle:@"Play" forState:UIControlStateNormal];        
        [recordButtonB setEnabled:YES];        
        
    }
}

- (void)timerStopFileA:(NSTimer*)theTimer
{
    [self playFileA:nil];    
}

- (void)timerStopFileB:(NSTimer*)theTimer
{
    [self playFileB:nil];
}


- (IBAction)playFileA:(id)sender 
{
    if ([self playerA] == nil)
    {
        NSError* err = nil;
        AVAudioPlayer* avPlayer = 
            [[AVAudioPlayer alloc] initWithContentsOfURL:[self fileA] 
                                                   error:&err];

        
        [self setPlayerA:avPlayer];
        
        [avPlayer release];         
        
        if (err != nil) {
            NSLog(@"Error creating the AVPlayer: %@", [err localizedDescription]);
            return;
        }
        
        if (![[self playerA] prepareToPlay]) {
            [NSException raise:@"error" format:@"Cannot prepare playerA to play."];
            return;
        }
        
        [[self playerA] setDelegate:self];
        [[self playerA] setVolume:1.0];
        
        if ([self useClippedTimes] && (file_a_info.max_peak != 0)) {
            NSTimeInterval start_time = file_a_info.threshold_start_time;
            NSTimeInterval duration = file_a_info.threshold_end_time - file_a_info.threshold_start_time;
            [[self playerA] setCurrentTime:start_time];
            NSTimer* timer = [NSTimer timerWithTimeInterval:duration target:self selector:@selector(timerStopFileA:) userInfo:nil repeats:NO];
            
            [[self playerA] play];
            [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
            
        } else {
            [[self playerA] play];            
        }
        
        [activityIndicatorA startAnimating];
        
        [playButtonA setTitle:@"Stop" forState:UIControlStateNormal];
        [recordButtonA setEnabled:NO];
        
        
    } else {
        
        [[self playerA] stop];
        [activityIndicatorA stopAnimating];
        [self setPlayerA:nil];
        [playButtonA setTitle:@"Play" forState:UIControlStateNormal];        
        [recordButtonA setEnabled:YES];        
        
    }
}

- (IBAction)recordFileA:(id)sender {
    
    memset(&file_a_info, 0, sizeof(WMAudioFilePreProcessInfo));        
    
    if ([voice_recorder_ isRecording]) {
        [voice_recorder_ stopRecording];
        
        bool success = WMGetPreProcessInfoForFile((CFURLRef)[self fileA], 
                                                  [self clipBeginThresholdDB], 
                                                  [self clipEndThresholdDB], 
                                                  &file_a_info);
        
        if (!success) {
            NSLog(@"Could not get pre process info from file a.");
        }
        
        [recordButtonA setTitle:@"Record" forState:UIControlStateNormal];
        [playButtonA setEnabled:YES];
        [activityIndicatorA stopAnimating];
        [recordButtonB setEnabled:YES];        
    } else {
        [voice_recorder_ startRecordingIntoFile:[self fileA]];        
        [recordButtonA setTitle:@"Stop" forState:UIControlStateNormal];
        [playButtonA setEnabled:NO];        
        [activityIndicatorA startAnimating];
        [recordButtonB setEnabled:NO];
    }
}

- (IBAction)endEditingBeginThresholdDB:(id)sender {
    
    NSLog(@"Recalculating");
    
    bool success = WMGetPreProcessInfoForFile((CFURLRef)[self fileA], 
                                              [self clipBeginThresholdDB], 
                                              [self clipEndThresholdDB], 
                                              &file_a_info);
    
    if (!success) {
        NSLog(@"Could not calculate pre process info on startup for file A");
    } 
    
    success = WMGetPreProcessInfoForFile((CFURLRef)[self fileB], 
                                         [self clipBeginThresholdDB], 
                                         [self clipEndThresholdDB], 
                                         &file_b_info);
    
    if (!success) {
        NSLog(@"Could not calculate pre process info on startup for file B");
    }     
}

- (IBAction)endEditingEndThresholdDB:(id)sender {
    bool success = WMGetPreProcessInfoForFile((CFURLRef)[self fileA], 
                                              [self clipBeginThresholdDB], 
                                              [self clipEndThresholdDB], 
                                              &file_a_info);
    
    if (!success) {
        NSLog(@"Could not calculate pre process info on startup for file A");
    } 
    
    success = WMGetPreProcessInfoForFile((CFURLRef)[self fileB], 
                                         [self clipBeginThresholdDB], 
                                         [self clipEndThresholdDB], 
                                         &file_b_info);
    
    if (!success) {
        NSLog(@"Could not calculate pre process info on startup for file B");
    }       
}

- (IBAction)recordFileB:(id)sender {
    
    memset(&file_b_info, 0, sizeof(WMAudioFilePreProcessInfo));         
    
    if ([voice_recorder_ isRecording]) {
        [voice_recorder_ stopRecording];
        
        bool success = WMGetPreProcessInfoForFile((CFURLRef)[self fileB], 
                                                  [self clipBeginThresholdDB], 
                                                  [self clipEndThresholdDB], 
                                                  &file_b_info);
        
        if (!success) {
            NSLog(@"Could not get pre process info from file b.");
        }        
        
        [recordButtonB setTitle:@"Record" forState:UIControlStateNormal];
        [playButtonB setEnabled:YES];
        [activityIndicatorB stopAnimating];
        [recordButtonA setEnabled:YES];        
    } else {
        [recordButtonA setEnabled:NO];
        [voice_recorder_ startRecordingIntoFile:[self fileB]];        
        [recordButtonB setTitle:@"Stop" forState:UIControlStateNormal];
        [playButtonB setEnabled:NO];        
        [activityIndicatorB startAnimating];
    }
}

- (IBAction)switchSpeakerMode:(id)sender {
    
    UISwitch* uiSwitch = (UISwitch*)sender;
    
    UInt32 route = 0;
    
    if ([uiSwitch isOn]) {
        route = kAudioSessionOverrideAudioRoute_Speaker;
    } else {
        route = kAudioSessionOverrideAudioRoute_None;
    }
    
    OSStatus result = AudioSessionSetProperty (kAudioSessionProperty_OverrideAudioRoute,
                                               sizeof (route),
                                               &route);
    
    if (result != noErr) {
        NSLog(@"Error setting audio route.");
    }         
    
    
}

- (IBAction)switchUseClippedTimes:(id)sender {
    UISwitch* uiSwitch = (UISwitch*)sender;
    [self setUseClippedTimes:[uiSwitch isOn]];
}

- (IBAction)executeComparison:(id)sender {
    
    [[self activityIndicatorDtw] startAnimating];
    
    WMFeatureType min_distance = 0;
    
    bool success = WMGetMinDistanceForFile((CFURLRef)[self fileA], 
                                           (CFURLRef)[self fileB], 
                                           &min_distance,
                                           &file_a_info,
                                           &file_b_info);
    
    if (!success)
        NSLog(@"Error retrieving distance.");
    
    const float med_tolerance = 1.3f;
    
    if (min_distance < dtwThreshold) {
        [[self qualitativeResultLabel] setBackgroundColor:[UIColor greenColor]];
        
        NSString * str = 
        [[NSString alloc] initWithFormat:@"Match (%.2f)",min_distance];        
        
        [[self qualitativeResultLabel] setText:str];
        
        [str release];
    } else if (min_distance < dtwThreshold*med_tolerance) {
        [[self qualitativeResultLabel] setBackgroundColor:[UIColor orangeColor]];
        NSString * str = 
        [[NSString alloc] initWithFormat:@"Close (%.2f)",min_distance];        
        
        [[self qualitativeResultLabel] setText:str];
        
        [str release];
    } else {
        [[self qualitativeResultLabel] setBackgroundColor:[UIColor redColor]];
        NSString * str = 
        [[NSString alloc] initWithFormat:@"Wrong (%.2f)",min_distance];        
        
        [[self qualitativeResultLabel] setText:str];
        
        [str release];
    }
    
    [[self activityIndicatorDtw] stopAnimating];   
    
}

- (IBAction)changeTreshold:(id)sender {
    
    NSString * str = 
        [[NSString alloc] initWithFormat:@"%.2f",[[self thresholdSlider] value]];
        
    [[self thresholdLabel] setText:str];
    
    [str release];
    
    [self setDtwThreshold:[[self thresholdSlider] value]];
}

- (IBAction)changeEndThresholdDB:(id)sender {
    
    UISlider* slider = (UISlider*)sender;
    
    [self setClipEndThresholdDB:[slider value]];
    
    NSString * str = 
    [[NSString alloc] initWithFormat:@"%.2f db",[self clipEndThresholdDB]];
    
    [[self endThresholdLabel] setText:str];
    
    [str release];    
    
}

- (IBAction)changeBeginThresholdDB:(id)sender {
    
    UISlider* slider = (UISlider*)sender;
    
    [self setClipBeginThresholdDB:[slider value]];
    
    NSString * str = 
    [[NSString alloc] initWithFormat:@"%.2f db",[self clipBeginThresholdDB]];
    
    [[self beginThresholdLabel] setText:str];
    
    [str release];
    
}


//TODO: implement other usefull delegates of AVAudioPlayer

- (void) audioPlayerDidFinishPlaying: (AVAudioPlayer *) player
                        successfully: (BOOL) completed {
    if (completed == YES) {
        
        if (player == [self playerA])
            [self playFileA:self];
        else if (player == [self playerB])
            [self playFileB:self];
        else
            [NSException raise:@"error" format:@"Unexpected player instance"];
        
    } else {
        NSLog(@"Something happened during decoding.");
    }
}

@end
