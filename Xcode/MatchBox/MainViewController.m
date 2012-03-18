//Copyright (c) 2011 Sebastian Bšhm sebastian@sometimesfood.org
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

#import "MainViewController.h"

#import "NetworkManager.h"

@implementation MainViewController
@synthesize uiCreatorsToolbar;
@synthesize uiPlayersToolbar;
@synthesize uiImageView;
@synthesize imagePicker = imagePicker_;
@synthesize uiActionSheetBarButtonItem;
@synthesize uiMainToolbar;
@synthesize uiNavigationBar;
@synthesize uiPlayersControlDoneButton;
@synthesize uiMyFilePlayButton;
@synthesize uiNumberOfTriesLeftLabel;
@synthesize uiCorrectnessIndicatorView;
@synthesize uiMyFileRecordButton;
@synthesize uiDescribeAndCompareButton;
@synthesize uiPlaySolutionButton;
@synthesize audioPlayer = audioPlayer_;
@synthesize startImage = startImage_;
@synthesize uiCorrectnessCircle;
@synthesize lastReceiveOperation;

static const SInt32 kMatchBoxNumberOfTriesPerGame = 8;

static const float kMatchBoxMinValidDuration = 0.3f;
static const float kMatchBoxMinValidAmplitude = 0.1f;

static const float rgb_match[3] = {143/255.0f,184/255.0f,72/255.0f};
static const float rgb_wrong[3] = {177/255.0f,34/255.0f,32/255.0f};

NSString * const kMatchBoxInfoDictKeyPeak = @"MBDictKeyPeak";
NSString * const kMatchBoxInfoDictKeyNormalizationFactor = @"MBDictKeyNormF";
NSString * const kMatchBoxInfoDictKeyThresholdBeginTime = @"MBDictKeyBTime";
NSString * const kMatchBoxInfoDictKeyThresholdEndTime = @"MBDictKeyETime";

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSLog(@"Loading view...");
    
    //This ensure that we have the default values loaded, even if now 
    //user configuration existed previously
    [SettingsViewController checkAndInitializeUserDefaults:NO];
    
    NSUserDefaults * d = [NSUserDefaults standardUserDefaults];
    
    comparisonThreshold_ = [d floatForKey:kMatchBoxDefaultComparisonThresholdKey];
    trimBeginThreshold_ = [d floatForKey:kMatchBoxDefaultTrimBeginThresholdKey];
    trimEndThreshold_ = [d floatForKey:kMatchBoxDefaultTrimEndThresholdKey];
    
    [[self uiPlayersToolbar] setHidden:YES];
    UIImagePickerController* picker = [[UIImagePickerController alloc] init];    
    [self setImagePicker:picker];
    [picker release];
    
    UIActionSheet * sheet = 
        [[UIActionSheet alloc] initWithTitle:nil 
                                    delegate:self
                           cancelButtonTitle:@"Cancel"
                      destructiveButtonTitle:nil
                           otherButtonTitles:@"Send", @"Test", nil];
    
    actionSheetPerform_ = sheet;
    
    sheet = 
        [[UIActionSheet alloc] initWithTitle:@"Do you really want to play the solution?" 
                                    delegate:self                     
                           cancelButtonTitle:@"No" 
                      destructiveButtonTitle:@"Yes"                     
                           otherButtonTitles:nil];
    
    actionSheetConfirmSolution_ = sheet;
    
    alertWon_ = 
        [[UIAlertView alloc] initWithTitle:@"You won!"        
                                   message:@"Do you want to play back the solution?" 
                                  delegate:self 
                         cancelButtonTitle:nil 
                         otherButtonTitles:@"Yes", @"No", nil];
    
    alertLost_ = 
        [[UIAlertView alloc] initWithTitle:@"Aww... you lost!" 
                                   message:@"You have no clue, right? Do you want to listen to the solution?" 
                                  delegate:self 
                         cancelButtonTitle:nil 
                         otherButtonTitles:@"Yes", @"No", nil]; 
    
    alertSayItAgain_ = 
        [[UIAlertView alloc] initWithTitle:@"Speak up!" 
                                   message:@"Sorry, I could not understand you, please say it again." 
                                  delegate:self 
                         cancelButtonTitle:nil
                         otherButtonTitles:@"Ok",nil];    
    
//    alertChallengeReceived_ = 
//        [[UIAlertView alloc] initWithTitle:@"Challenge Notification" 
//                                   message:@"Someone is challenging you, do you want to accept it?" 
//                                  delegate:self 
//                         cancelButtonTitle:@"No"
//                         otherButtonTitles:@"Yes",nil];        
    
    //We start up in creators mode
    currentMode_ = kMatchBoxCreatorsMode;
    
    //Fill the main toolbar
    [[self uiMainToolbar] setItems:[[self uiCreatorsToolbar] items] animated:NO];
    
    fileManager_ = [[NSFileManager alloc] init];
    
    // we set the current file in our app's documents directory
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, 
                                                         NSUserDomainMask, 
                                                         YES);
    
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *myFileStr = [NSString stringWithFormat:@"%@/mine.caf",
                                documentsDirectory];
    
    NSURL * url = [[NSURL alloc ] initFileURLWithPath:myFileStr];
    myAudioFileURL_ = url;
    
    // delete old audio file
    [fileManager_ removeItemAtURL:myAudioFileURL_ error:NULL];
    
    
    myFileStr = [NSString stringWithFormat:@"%@/mine.plist",
                 documentsDirectory];        
    
    url = [[NSURL alloc ] initFileURLWithPath:myFileStr];
    myAudioFilePropertiesURL_ = url;
    
    NSString *othersFileStr = [NSString stringWithFormat:@"%@/others.caf",
                           documentsDirectory];
    
    othersAudioFileURL_ = [[NSURL alloc ] initFileURLWithPath:othersFileStr];
    
    
    NSString *myImageFileString = [NSString stringWithFormat:@"%@/mine.jpg",
                                   documentsDirectory];
    
    myImageFileURL_ = [[NSURL alloc] initFileURLWithPath:myImageFileString];
    
    NSString *othersImageFileString = 
        [NSString stringWithFormat:@"%@/others.jpg",
         documentsDirectory];
    
    othersImageFileURL_ = 
        [[NSURL alloc] initFileURLWithPath:othersImageFileString];
    
    memset(&myFileProcessInfo_, 0, sizeof(WMAudioFilePreProcessInfo));        
    
    BOOL myImageFileExists = 
        [fileManager_ fileExistsAtPath:[myImageFileURL_ path]];
    
    if (myImageFileExists) {
        
        UIImage * image = 
            [UIImage imageWithContentsOfFile:[myImageFileURL_ path]];
        
        [[self uiImageView] setImage:image];
    }
    
    voiceRecorder_ = [[SOVoiceRecorder alloc] init];
    
    [[self uiCorrectnessIndicatorView] setHidden:YES];
    [[self uiNumberOfTriesLeftLabel] setHidden:YES];
    
    numberOfTriesLeft_ = kMatchBoxNumberOfTriesPerGame;

    NSString * str = [NSString stringWithFormat:@"# Tries left: %d", 
                                                numberOfTriesLeft_];
    
    [[self uiNumberOfTriesLeftLabel] setText:str];
    
    UIImage * img = [UIImage imageNamed:@"start_screen"];
    if (img == nil) {
        NSLog(@"Could not load start image.");
    }
    
    [self setStartImage:img];
    
    [self loadInitialState];
    
    self.lastReceiveOperation = [NSDate dateWithTimeIntervalSinceNow:-10];
    
    isTesting_ = false;
    
    //TODO: check what happens if you have connected with headphones...
    
    UInt32 route = kAudioSessionOverrideAudioRoute_Speaker;
    
    OSStatus result = AudioSessionSetProperty (kAudioSessionProperty_OverrideAudioRoute,
                                               sizeof (route),
                                               &route);
    
    if (result != noErr) {
        NSLog(@"Error setting audio route to speaker during initialization.");
    }           
    
}

- (void)loadInitialState
{
    
    [[self uiImageView] setImage:self.startImage];
    self.uiMyFilePlayButton.enabled = NO;
    self.uiActionSheetBarButtonItem.enabled = NO;
    
}

- (void)checkForCompletenessAndEnableActionSheet
{
    BOOL myFileExists = [fileManager_ fileExistsAtPath:[myAudioFileURL_ path]];
    if ( myFileExists && ([uiImageView image] != self.startImage) )
    {
        self.uiActionSheetBarButtonItem.enabled = YES;
    }
}

- (IBAction)describeAndCompare:(id)sender {
    
    UIBarButtonItem* btn = (UIBarButtonItem*)sender;
    
    // Start the recorder (recording into myFile, reuse that!!!)
    if (![voiceRecorder_ isRecording]) { 
        
        [voiceRecorder_ startRecordingIntoFile:myAudioFileURL_];
        [btn setTitle:@"Compare"];
        
    } else {
        
        [voiceRecorder_ stopRecording];
        [self prepareAudioPlayerForURL:myAudioFileURL_];        
        
        [btn setTitle:@"Describe"];
        
        //Extract the process info from the just-record file
        bool b = WMGetPreProcessInfoForFile((CFURLRef)myAudioFileURL_, 
                                            trimBeginThreshold_, 
                                            trimEndThreshold_, 
                                            &myFileProcessInfo_);
        
        assert(b);
        
        float dist = myFileProcessInfo_.threshold_end_time - 
                     myFileProcessInfo_.threshold_start_time;
        
        if ( (dist <= kMatchBoxMinValidDuration) || 
             (myFileProcessInfo_.max_peak < kMatchBoxMinValidAmplitude) ) {
            //In this case we are tolerant and let the player say it again
            [alertSayItAgain_ show];
            return;
        }
        
        //Finally, compare!
        WMFeatureType distance = 0;
        b = WMGetMinDistanceForFile((CFURLRef)myAudioFileURL_, 
                                    (CFURLRef)othersAudioFileURL_, 
                                    &distance, 
                                    &myFileProcessInfo_ , 
                                    &othersFileProcessInfo_);
        
        assert(b);

        //We define a linear scaling between the value the is our total match
        //down to some other value
        const float lower_value = comparisonThreshold_;
        const float upper_value = comparisonThreshold_*2;
        
        float scale_value = (distance - lower_value) / (upper_value - lower_value);
        
        scale_value = MIN( MAX(scale_value, 0), 1);
        
        [self setCorrectnessLevel:scale_value withAnimation:YES];
        
        NSLog(@"MinDist Value: %f", distance);        
        NSLog(@"Scale Value: %f", scale_value);
                
    }
    
}

- (IBAction)playSolution:(id)sender {
    
    [actionSheetConfirmSolution_ showFromBarButtonItem:[self uiPlaySolutionButton] 
                                             animated:YES];
        
}

- (void)prepareAudioPlayerForURL:(NSURL*)url {

    //Initialize the AV audio player
    NSError * error = nil;
    AVAudioPlayer* player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
    
    [player setVolume:1.0f];
    
    if (error != nil) {
        NSLog(@"Error creating the AVPlayer: %@", [error localizedDescription]);            
    }
    
    if (![player prepareToPlay]) {
        NSLog(@"Could not prepare the player for playback.");
    }
    
    [player setDelegate:self];
    [self setAudioPlayer:player];
    
    [player release];    
    
}

- (void)switchCurrentModeTo:(MatchBoxControlMode)mode
{
    
    if (mode == kMatchBoxPlayersMode) {
        
        //Note: this method assumes, that others.caf, others.jpg and 
        //others.plist has already been set correctly, no matter if these are 
        //created by ourselves our have been sent by someone else...
        
        [[self uiMainToolbar] setItems:[[self uiPlayersToolbar] items] 
                              animated:YES];
        
        [[self uiActionSheetBarButtonItem] setEnabled:NO];
        [[self uiCorrectnessIndicatorView] setHidden:NO];
        [[self uiNumberOfTriesLeftLabel] setHidden:NO];

        numberOfTriesLeft_ = kMatchBoxNumberOfTriesPerGame;
        NSString * str = [NSString stringWithFormat:@"# Tries left: %d", 
                                                      numberOfTriesLeft_];
        
        [[self uiNumberOfTriesLeftLabel] setText:str];
        
        //animate to position
        
        [self setCorrectnessLevel:0.5f withAnimation:YES];
        
        //prepare the player to play back the others' file
        [self prepareAudioPlayerForURL:othersAudioFileURL_];

        //set the image to others' image file
        UIImage * image = 
            [UIImage imageWithContentsOfFile:[othersImageFileURL_ path]];        
        self.uiImageView.image = image;
        
    } else {
        
        [[self uiMainToolbar] setItems:[[self uiCreatorsToolbar] items] 
                              animated:YES];        
        [[self uiActionSheetBarButtonItem] setEnabled:YES];        
        [[self uiCorrectnessIndicatorView] setHidden:YES];        
        [[self uiNumberOfTriesLeftLabel] setHidden:YES];
        
        if (!isTesting_) {
            
            [self loadInitialState];
            
        } else {
            
            //copy back the others to mine.caf, since we were just testing
            
            NSError* error = nil;        
            
            //Be sure to remove destination files, if they exist
            NSString * path = [myAudioFileURL_ path];
            
            [[self audioPlayer] stop];
            
            [self setAudioPlayer:nil];
            
            if ([fileManager_ fileExistsAtPath:path]) {
                BOOL s = [fileManager_ removeItemAtURL:myAudioFileURL_ 
                                                 error:&error];
                if (!s || (error != nil)) {
                    [NSException raise:@"error" 
                                format:@"Error removing file: %@", 
                                         [error localizedDescription]];
                }            
                
            }
            
            //Copy others back to mine... 
            BOOL success = [fileManager_ copyItemAtURL:othersAudioFileURL_ 
                                                 toURL:myAudioFileURL_ 
                                                 error:&error];
            
            [self prepareAudioPlayerForURL:myAudioFileURL_];
            
            
            if (!success || (error != nil)) {
                [NSException raise:@"error" 
                            format:@"Error removing file: %@", 
                 [error localizedDescription]];
            }            
            
            //also copy process info
            myFileProcessInfo_ = othersFileProcessInfo_; 
            
            
        }
        
        isTesting_ = false;
    }
    
    currentMode_ = mode;
}

- (IBAction)recordIntoMyFile:(id)sender {
    
    UIBarButtonItem* btn = (UIBarButtonItem*)sender;    
    
    //TODO: the record button has be a custom view to make it 
    //RED and to say stop!!!! 
    if (![voiceRecorder_ isRecording]) { 
        
        [voiceRecorder_ startRecordingIntoFile:myAudioFileURL_];
        [btn setTitle:@"Stop"];
        [[self uiMyFilePlayButton] setEnabled:NO];
        
    } else {
        
        [voiceRecorder_ stopRecording];
        [self prepareAudioPlayerForURL:myAudioFileURL_];        
        [btn setTitle:@"Record"];
        [[self uiMyFilePlayButton] setEnabled:YES];
        
        //Extract the process info from the just-record file
        bool b = WMGetPreProcessInfoForFile((CFURLRef)myAudioFileURL_, 
                                            trimBeginThreshold_, 
                                            trimEndThreshold_, 
                                            &myFileProcessInfo_);
        assert(b);
        
        float dist = myFileProcessInfo_.threshold_end_time - 
                     myFileProcessInfo_.threshold_start_time;    
        
        if ( (dist <= kMatchBoxMinValidDuration) || 
            (myFileProcessInfo_.max_peak < kMatchBoxMinValidAmplitude) ) {
            [alertSayItAgain_ show];
            return;
        }        
        
        [self checkForCompletenessAndEnableActionSheet];        
        
    }
}

- (void)timerStopPlayingOthersFile:(NSTimer*)theTimer
{
    [[self audioPlayer] stop];
}

- (void)timerStopPlayingMyFile:(NSTimer*)theTimer
{
    [[self audioPlayer] stop];
    [self playMyFile:nil];
}

- (IBAction)playMyFile:(id)sender {
    
    if ( (sender == nil) || 
         [[self audioPlayer] isPlaying] )
    {
        
        [[self uiMyFilePlayButton] setTitle:@"Play"];
        [[self uiMyFileRecordButton] setEnabled:YES];        
        
    } else {
        
        if (![[[self audioPlayer] url] isEqual:myAudioFileURL_])
            [self prepareAudioPlayerForURL:myAudioFileURL_];
        
        NSTimeInterval start_time = myFileProcessInfo_.threshold_start_time;
        
        NSTimeInterval duration = myFileProcessInfo_.threshold_end_time - 
                                  myFileProcessInfo_.threshold_start_time;

        if (duration <= 0)
            return;

        [[self audioPlayer] setCurrentTime:start_time];
        
        NSTimer* timer = [NSTimer timerWithTimeInterval:duration 
                                                 target:self selector:@selector(timerStopPlayingMyFile:)
                                               userInfo:nil 
                                                repeats:NO];
        
        [[self audioPlayer] play];
        
        [[NSRunLoop currentRunLoop] addTimer:timer 
                                     forMode:NSDefaultRunLoopMode];
        
        [[self uiMyFilePlayButton] setTitle:@"Pause"];
        [[self uiMyFileRecordButton] setEnabled:NO];
    }
    
}

- (void)settingsViewControllerDidFinish:(SettingsViewController *)controller
{
    
    //read out the new settings
    NSUserDefaults * d = [NSUserDefaults standardUserDefaults];
    comparisonThreshold_ = [d floatForKey:kMatchBoxDefaultComparisonThresholdKey];
    trimBeginThreshold_ = [d floatForKey:kMatchBoxDefaultTrimBeginThresholdKey];
    trimEndThreshold_ = [d floatForKey:kMatchBoxDefaultTrimEndThresholdKey];
    
    //re-calculate process info for both,  if present
    BOOL myFileExists = [fileManager_ fileExistsAtPath:[myAudioFileURL_ path]];
    
    [[self uiMyFilePlayButton] setEnabled:myFileExists];            
    
    if (myFileExists) {
        bool b = WMGetPreProcessInfoForFile((CFURLRef)myAudioFileURL_, 
                                            trimBeginThreshold_, 
                                            trimEndThreshold_, 
                                            &myFileProcessInfo_);
        assert(b);
    }
    
    //Note: it does not make any sense to recalculate the others' audio file, 
    //since they  might have been recorded in a completely different 
    //environment
    
    [self dismissModalViewControllerAnimated:YES];    
}

- (void)messagingViewControllerDidFinish:(MessagingViewController *)controller
{
    
    //when sending, everything is fine, probably show alert when it failed
    BOOL doSwitchToPlayersMode = NO;
    
    //when receiving, copy files to others
    if (  controller.messagingSucceeded && 
         (controller.mode == kMatchBoxMessagingModeReceive) ) {
        
        NSArray* filesReceived = [controller receivedFiles];
        
        //Be sure to remove destination files, if they exist
        NSError* error = nil;
        NSString * path = [othersAudioFileURL_ path];
        
        if ([fileManager_ fileExistsAtPath:path]) {
            BOOL s = [fileManager_ removeItemAtURL:othersAudioFileURL_ 
                                             error:&error];
            if (!s || (error != nil)) {
                NSLog(@"Error removing file: %@", 
                      [error localizedDescription]);
                return;
            }            
            
        }
        
        path = [othersImageFileURL_ path];
        
        if ([fileManager_ fileExistsAtPath:path]) {
            BOOL s = [fileManager_ removeItemAtURL:othersImageFileURL_ 
                                             error:&error];
            if (!s || (error != nil)) {
                NSLog(@"Error removing file: %@", [error localizedDescription]);
                return;
            }            
            
        }        
        
        NSLog(@"Num of files received: %d", [filesReceived count] );
        assert([filesReceived count] == 3);
        
        //Copy received stuff to others...
        NSURL* url_received = 
            [NSURL fileURLWithPath:(NSString*)[filesReceived objectAtIndex:0]];
        
        NSLog(@"File: %@", [filesReceived objectAtIndex:0]);
        

        if ([fileManager_ fileExistsAtPath:[filesReceived objectAtIndex:0]])
            NSLog(@"Exists!");
        
        if (error != nil)
            NSLog(@"Error: %@", [error localizedDescription]);
        
        BOOL success = [fileManager_ copyItemAtURL:url_received 
                                             toURL:othersAudioFileURL_ 
                                             error:&error];
        
        
        if (!success || (error != nil)) {
            NSLog(@"Error copying audio: %@", [error localizedDescription]);
            return;
        }
        
        url_received = 
            [NSURL fileURLWithPath:(NSString*)[filesReceived objectAtIndex:1]];        
        
        success = [fileManager_ copyItemAtURL:url_received      
                                        toURL:othersImageFileURL_ 
                                        error:&error];
        
        if (!success || (error != nil)) {
            NSLog(@"Error copying image: %@", [error localizedDescription]);
            return;
        }    
        
        //convert the received metadata file and copy it
        url_received = 
            [NSURL fileURLWithPath:(NSString*)[filesReceived objectAtIndex:2]];        
        
        NSDictionary * dict = 
            [NSDictionary dictionaryWithContentsOfURL:url_received];
        
        WMAudioFilePreProcessInfo info = 
            [MainViewController getProcessInfoFromDictionary:dict];
        
        //copy my metadata to the others' metadata
        othersFileProcessInfo_ = info;
        
        isTesting_ = NO;
        doSwitchToPlayersMode = YES;
        
        
    } 
    
    if (!controller.messagingSucceeded) {
        self.lastReceiveOperation = [NSDate dateWithTimeIntervalSinceNow:0];
    }
    
    //in each case, we'll have to reset the delegate
    NetworkManager* m = [NetworkManager sharedNetworkManager];
    
    m.delegate = self;
    
    [self dismissModalViewControllerAnimated:YES];
    
    if (doSwitchToPlayersMode)
        [self switchCurrentModeTo:kMatchBoxPlayersMode];            
}

- (IBAction)showInfo:(id)sender
{    
    
    SettingsViewController *controller = 
        [[SettingsViewController alloc] initWithNibName:@"FlipsideView" 
                                                 bundle:nil];

    controller.delegate = self;
    
    controller.modalTransitionStyle = UIModalTransitionStyleCoverVertical;    
    
    [self presentModalViewController:controller animated:YES];
    
    [controller release];
    
}

- (IBAction)pickImageWithCamera:(id)sender {
    
    if ([UIImagePickerController isSourceTypeAvailable:
          UIImagePickerControllerSourceTypeCamera] == NO) {
        NSLog(@"Image Picker does not currently support camera source.");
        return;
    }
    
    [[self imagePicker] setSourceType:UIImagePickerControllerSourceTypeCamera];
    
    //Note: we are fine with the default media type, which is just
    //a still image to take.
    
    [[self imagePicker] setAllowsEditing:YES];
    [[self imagePicker] setDelegate:self];
    
    [self presentModalViewController:[self imagePicker] animated:YES];
    
}

- (IBAction)pickImageFromLibrary:(id)sender {
    
    if ([UIImagePickerController isSourceTypeAvailable:
         UIImagePickerControllerSourceTypePhotoLibrary] == NO) {
        NSLog(@"Image Picker does not currently support photo library.");
        return;
    }
    
    [[self imagePicker] setSourceType:UIImagePickerControllerSourceTypePhotoLibrary];
    
    //Note: we are fine with the default media type, which is just
    //a still image to take.
    
    [[self imagePicker] setAllowsEditing:YES];
    [[self imagePicker] setDelegate:self];
    
    [self presentModalViewController:[self imagePicker] animated:YES];    
    
}

- (IBAction)presentActionSheet:(id)sender {
    [actionSheetPerform_ showFromBarButtonItem:[self uiActionSheetBarButtonItem] 
                                     animated:YES];
}

- (IBAction)doneWithPlayMode:(id)sender {
    //TODO: probably show some stats as well...
    [self switchCurrentModeTo:kMatchBoxCreatorsMode];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
    // TODO: decide what to here...
}

- (void)viewDidUnload
{
    NSLog(@"Unloading view...");
    [self setUiCreatorsToolbar:nil];
    [self setUiPlayersToolbar:nil];
    [self setUiImageView:nil];
    [self setImagePicker:nil];
    [self setUiActionSheetBarButtonItem:nil];
    [self setUiPlayersControlDoneButton:nil];
    [self setUiNavigationBar:nil];
    [self setUiMainToolbar:nil];
    [self setUiMyFilePlayButton:nil];
    [self setUiMyFileRecordButton:nil];
    [self setUiCorrectnessIndicatorView:nil];
    [self setUiNumberOfTriesLeftLabel:nil];
    [self setUiPlaySolutionButton:nil];
    [self setUiDescribeAndCompareButton:nil];
    
    [self setStartImage:nil];
    
    [self setLastReceiveOperation:nil];
    
    [imagePicker_ release];    
    imagePicker_ = nil;
    [actionSheetPerform_ release];    
    actionSheetPerform_ = nil;
    [audioPlayer_ release];
    audioPlayer_ = nil;
    [voiceRecorder_ release];    
    voiceRecorder_ = nil;
    [actionSheetConfirmSolution_ release];    
    actionSheetConfirmSolution_ = nil;
    [alertWon_ release];
    alertWon_ = nil;
    [alertLost_ release];
    alertLost_ = nil;
//    [alertChallengeReceived_ release];
//    alertChallengeReceived_ = nil;
    [alertSayItAgain_ release];    
    alertSayItAgain_ = nil;
    [fileManager_ release];
    fileManager_ = nil;
    [myAudioFileURL_ release];
    myAudioFileURL_ = nil;
    [myAudioFilePropertiesURL_ release];
    myAudioFilePropertiesURL_ = nil;
    [myImageFileURL_ release];
    myImageFileURL_ = nil;
    [othersAudioFileURL_ release];
    othersAudioFileURL_ = nil;
    [othersImageFileURL_ release];    
    othersImageFileURL_ = nil;
    
    [self setUiCorrectnessCircle:nil];
    [super viewDidUnload];

}

- (void)dealloc
{
    [uiCreatorsToolbar release];
    [uiPlayersToolbar release];
    [uiImageView release];
    [uiActionSheetBarButtonItem release];
    [uiPlayersControlDoneButton release];
    [uiNavigationBar release];
    [uiMainToolbar release];
    [uiMyFilePlayButton release];
    [uiMyFileRecordButton release];
    [uiCorrectnessIndicatorView release];
    [uiNumberOfTriesLeftLabel release];
    [uiPlaySolutionButton release];
    [uiDescribeAndCompareButton release];
    
    [imagePicker_ release];    
    [actionSheetPerform_ release];    
    [audioPlayer_ release];
    [voiceRecorder_ release];    
    [actionSheetConfirmSolution_ release];    
    [alertWon_ release];
    [alertLost_ release];
    [alertSayItAgain_ release]; 
//    [alertChallengeReceived_ release];
    [fileManager_ release];    
    [myAudioFileURL_ release];
    [myAudioFilePropertiesURL_ release];
    [myImageFileURL_ release];
    [othersAudioFileURL_ release];
    [othersImageFileURL_ release];
    
    [startImage_ release];
    
    [lastReceiveOperation release];
    
    [uiCorrectnessCircle release];
    [super dealloc];
}

#pragma mark Image Picker Delegate Methods

- (void)imagePickerController:(UIImagePickerController *)picker 
    didFinishPickingMediaWithInfo:(NSDictionary *)info
{
    
    UIImage *image = [[info valueForKey:UIImagePickerControllerEditedImage] retain];    
    [self dismissModalViewControllerAnimated:YES];
    
    [[self uiImageView] setImage:image];
    
    //Also save the image as mine.jpeg
    NSData * data = UIImageJPEGRepresentation(image, 0.7f);
    
    [image release];
    
    BOOL success = [data writeToURL:myImageFileURL_ atomically:YES];
    if (!success)
        NSLog(@"Could not write to %@", myImageFileURL_);
  
    [self checkForCompletenessAndEnableActionSheet];
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [self dismissModalViewControllerAnimated:YES];
}

#pragma mark UIActionSheet Delegate Methods

- (void)actionSheet:(UIActionSheet *)actionSheetIn 
    clickedButtonAtIndex:(NSInteger)buttonIndex
{
    NSString * buttonTitle = [actionSheetIn buttonTitleAtIndex:buttonIndex];
    
    if (actionSheetIn == actionSheetPerform_) {
    
        if ([buttonTitle isEqualToString:@"Test"])
        {
            
            NSError* error = nil;        
            
            //Be sure to remove destination files, if they exist
            NSString * path = [othersAudioFileURL_ path];
            
            if ([fileManager_ fileExistsAtPath:path]) {
                BOOL s = [fileManager_ removeItemAtURL:othersAudioFileURL_ 
                                                error:&error];
                if (!s || (error != nil)) {
                    NSLog(@"Error removing file: %@", 
                          [error localizedDescription]);
                    return;
                }            
                
            }
            
            path = [othersImageFileURL_ path];
            
            if ([fileManager_ fileExistsAtPath:path]) {
                BOOL s = [fileManager_ removeItemAtURL:othersImageFileURL_ 
                                                error:&error];
                if (!s || (error != nil)) {
                    NSLog(@"Error removing file: %@", [error localizedDescription]);
                    return;
                }            
                
            }        
            
            //Copy mine to others... 
            BOOL success = [fileManager_ copyItemAtURL:myAudioFileURL_ 
                                                toURL:othersAudioFileURL_ 
                                                error:&error];
        
            
            if (!success || (error != nil)) {
                NSLog(@"Error copying audio: %@", [error localizedDescription]);
                return;
            }
            
            success = [fileManager_ copyItemAtURL:myImageFileURL_      
                                           toURL:othersImageFileURL_ 
                                           error:&error];
            
            if (!success || (error != nil)) {
                NSLog(@"Error copying image: %@", [error localizedDescription]);
                return;
            }    
                        
            //copy my metadata to the others' metadata
            othersFileProcessInfo_ = myFileProcessInfo_;
            
            if (!success)
                NSLog(@"Could not extract info data for other audio file.");

            isTesting_ = YES;            
            
            [self switchCurrentModeTo:kMatchBoxPlayersMode];
            
        } else if ([buttonTitle isEqualToString:@"Send"]) {
            
            [self prepareAndSendGameStatus];
            
        }
        
    } else if (actionSheetIn == actionSheetConfirmSolution_) {
        
        if ([buttonTitle isEqualToString:@"Yes"]) {
            
            [self playbackOthersFile];
            
        }
    }
}

- (void)playbackOthersFile
{
    if (![[[self audioPlayer] url] isEqual:othersAudioFileURL_])
        [self prepareAudioPlayerForURL:othersAudioFileURL_];
    
    NSTimeInterval start_time = othersFileProcessInfo_.threshold_start_time;
    
    NSTimeInterval duration = othersFileProcessInfo_.threshold_end_time - 
                              othersFileProcessInfo_.threshold_start_time;
    
    if (duration <= 0)
        return;
    
    [[self audioPlayer] setCurrentTime:start_time];
    
    NSTimer* timer = [NSTimer timerWithTimeInterval:duration 
                                             target:self 
                                           selector:@selector(timerStopPlayingOthersFile:)
                                           userInfo:nil 
                                            repeats:NO];
    
    [[self audioPlayer] play];
    [[NSRunLoop currentRunLoop] addTimer:timer 
                                 forMode:NSDefaultRunLoopMode];    
}

#pragma mark UIAlertView delegate methods

- (void)alertView:(UIAlertView *)alertView 
    clickedButtonAtIndex:(NSInteger)buttonIndex
{

    NSString * buttonTitle = [alertView buttonTitleAtIndex:buttonIndex];
    
    if ( (alertView == alertLost_) || 
         (alertView == alertWon_) )
    {
        if ([buttonTitle isEqualToString:@"Yes"]) {
            [self playbackOthersFile];
            [NSThread sleepForTimeInterval:[self.audioPlayer duration]];
        }
        
        //In any case, we switch back to creators mode
        [self switchCurrentModeTo:kMatchBoxCreatorsMode];        
    }        
//    } else if (alertView == alertChallengeReceived_) {
//        
//        if ([buttonTitle isEqualToString:@"Yes"]) {
//            
//            //We are making sure that we are in a safe state, were we could
//            //overwrite files and switch to the players mode without any 
//            //troubles
//            
//            [self dismissModalViewControllerAnimated:YES];
//            
////            [self copyReceivedGameStatusToOthers];
//            
//  //          isTesting_ = NO;
//            
////            [self switchCurrentModeTo:kMatchBoxPlayersMode];
//
//        }
//        
//    }
    
}

- (void)loadSafeState
{
    //If we are currently recording, we need to stop
    if ([voiceRecorder_ isRecording])
        [self recordIntoMyFile:[self uiMyFileRecordButton]];
    
    //If we are currently playing back our file, we need to stop
    if ([audioPlayer_ isPlaying])
        [audioPlayer_ stop];     
}

//TODO: implement other usefull delegates of AVAudioPlayer

#pragma mark AVAudioPlayer Delegate Methods
- (void) audioPlayerDidFinishPlaying: (AVAudioPlayer *) player
                        successfully: (BOOL) completed 
    
{
                            
    NSLog(@"Finished Playing");

    if ([[player url] isEqual:othersAudioFileURL_])
        return;
    
    if (completed == YES) {
        [self playMyFile:nil];
    } else {
        NSLog(@"Something happened during decoding.");
    }
}

#pragma mark Stubs for Sending/Receiving Game States

- (void)prepareAndSendGameStatus
{
    //myAudioFile and myFileProcessInfo_ should already be in sync, 
    //we just need to generate the plist file and broadcast them all together
    
    NSDictionary * dict = [MainViewController createDictionaryWithProcessInfo:myFileProcessInfo_];
    
    //TODO: does this overwrite old files automatically?
    BOOL s = [dict writeToURL:myAudioFilePropertiesURL_ atomically:NO];
    
    if (!s) {
        [NSException raise:@"error" format:@"Could not write to %@", myAudioFilePropertiesURL_];
    }
    
    MessagingViewController *controller = 
    [[MessagingViewController alloc] initWithNibName:@"MessagingView" 
                                             bundle:nil];
    
    controller.delegate = self;
    
    NetworkManager* m = [NetworkManager sharedNetworkManager];
    m.delegate = controller;
    
    controller.mode = kMatchBoxMessagingModeSend;    
    controller.modalTransitionStyle = UIModalTransitionStylePartialCurl;    
    
    [self presentModalViewController:controller animated:YES];
    
    [controller release];    
    
    //create the properties file
    
    NSArray* files = [NSArray arrayWithObjects:[myAudioFileURL_ path], 
                                               [myImageFileURL_ path],
                                               [myAudioFilePropertiesURL_ path],
                                                nil];
    
    [controller startSendingFiles:files];    
}

#pragma mark Utility methods

+ (NSDictionary*)createDictionaryWithProcessInfo:(WMAudioFilePreProcessInfo)info
{
    
    NSNumber * peak = [NSNumber numberWithFloat:info.max_peak];
    NSNumber * norm_factor = [NSNumber numberWithFloat:info.normalization_factor];
    NSNumber * begin_t = [NSNumber numberWithFloat:info.threshold_start_time];
    NSNumber * end_t = [NSNumber numberWithFloat:info.threshold_end_time];
    
    NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:
                          peak, kMatchBoxInfoDictKeyPeak, 
                          norm_factor, kMatchBoxInfoDictKeyNormalizationFactor,
                          begin_t, kMatchBoxInfoDictKeyThresholdBeginTime,
                          end_t, kMatchBoxInfoDictKeyThresholdEndTime,
                          nil];
    
    return dict;                        
}

+ (WMAudioFilePreProcessInfo)getProcessInfoFromDictionary:(NSDictionary*)dict
{
    
    WMAudioFilePreProcessInfo info;
    
    id obj = [dict objectForKey:kMatchBoxInfoDictKeyPeak];
    
    if ( (obj == nil) ||
         ![obj isKindOfClass:[NSNumber class]])
    {
        [NSException raise:@"error" format:@"Wrong value for key %@.", 
         kMatchBoxInfoDictKeyPeak];
    }
    
    info.max_peak = [(NSNumber*)obj floatValue];
    
    obj = [dict objectForKey:kMatchBoxInfoDictKeyNormalizationFactor];
    
    if ( (obj == nil) ||
        ![obj isKindOfClass:[NSNumber class]])
    {
        [NSException raise:@"error" format:@"Wrong value for key %@.", 
         kMatchBoxInfoDictKeyNormalizationFactor];
    }
    
    info.normalization_factor = [(NSNumber*)obj floatValue]; 
    
    obj = [dict objectForKey:kMatchBoxInfoDictKeyThresholdBeginTime];
    
    if ( (obj == nil) ||
        ![obj isKindOfClass:[NSNumber class]])
    {
        [NSException raise:@"error" format:@"Wrong value for key %@.", 
         kMatchBoxInfoDictKeyThresholdBeginTime];
    }
    
    info.threshold_start_time = [(NSNumber*)obj floatValue];    
    
    obj = [dict objectForKey:kMatchBoxInfoDictKeyThresholdEndTime];
    
    if ( (obj == nil) ||
        ![obj isKindOfClass:[NSNumber class]])
    {
        [NSException raise:@"error" format:@"Wrong value for key %@.", 
         kMatchBoxInfoDictKeyThresholdEndTime];
    }
    
    info.threshold_end_time = [(NSNumber*)obj floatValue];    
    
    return info;
}

#pragma mark Network Manager delegate methods

- (void)sendDidStart 
{
    NSLog(@"@MainView: Send did start.");
}

- (void)sendDidStopWithStatus:(NSString *)status // status==nil means success
{

    NSLog(@"@MainView: Send did stop with %@.", status);
    
}

- (void)receiveDidStart
{    
    NSLog(@"MainView: Receive did start");
    
    if ([self.lastReceiveOperation timeIntervalSinceNow] > -10) {
        NSLog(@"Rejecting receive did start.");
        return;
    }
    
    NetworkManager* m = [NetworkManager sharedNetworkManager];
    
    if ([m isSending]) {
        NSLog(@"Warning: got a receive notification while sending, will ignore it.");
        return;
    }
    
    [self loadSafeState];
    
    MessagingViewController *controller = 
    [[MessagingViewController alloc] initWithNibName:@"MessagingView" 
                                              bundle:nil];
    
    controller.delegate = self;
    
    controller.uiMessageLabel.text = @"Receiving Game Status...";

    //switching the delegate, but note that the MessagingViewController
    //also forwards any notification to us.
    m.delegate = controller;    
    
    controller.mode = kMatchBoxMessagingModeReceive;
    
    controller.modalTransitionStyle = UIModalTransitionStylePartialCurl;    
    
    [self presentModalViewController:controller animated:YES];
    
    [controller release];    
    
    //Always use the same order: audio file, image file, audio description file.
    [controller setNumberOfExceptedFiles:3];
    
    //TODO: usually, we shall ask the user if he wants to accept it
    //for that we will need a receivedWillStart notification which allows
    //us to reject the reception of a message
}

- (void)receiveDidStopWithStatus:(NSString *)status // status==nil means success
                     andFilePath:(NSString *)path // only non-nil on success
{
    NSLog(@"@MainView: MainView Receive did stop with %@", status);
}

- (void)serverDidStartOnPort:(int)port
{
    NSLog(@"@MainView: server did start");
}

- (void)serverDidStopWithReason:(NSString *)reason
{
    NSLog(@"@MainView: server did stop");
}

- (void)didStartNetworking
{
    UIApplication * app = [UIApplication sharedApplication];
    [app setNetworkActivityIndicatorVisible:YES];
    NSLog(@"@MainView: did start netwrok");
}

- (void)didStopNetworking
{
    UIApplication * app = [UIApplication sharedApplication];
    [app setNetworkActivityIndicatorVisible:NO];    
    NSLog(@"@MainView: did stop network.");
}

#pragma mark Fancy animation of correctness slider
- (void)setCorrectnessLevel:(CGFloat) level 
              withAnimation:(BOOL) animate
{
    
    CGRect new_frame;
    CGRect corrFrame = [self uiCorrectnessIndicatorView].frame;
    new_frame.origin.x = corrFrame.size.width / 2 - 15;
    new_frame.origin.y = 60 + level*(corrFrame.size.height - 60*2) - 15;
    new_frame.size.width = 30;
    new_frame.size.height = 30;        
 
    UIViewAnimationOptions options;
    
    if (level < 0.5)
        options = UIViewAnimationOptionCurveEaseIn;
    else
        options = UIViewAnimationOptionCurveEaseOut;
    
    if (!animate) {
        self.uiCorrectnessCircle.frame = new_frame;
        [self.uiCorrectnessCircle setNeedsDisplay];        
    } else {
        
        [UIView animateWithDuration:0.5 
                              delay:0 
                            options:options 
                         animations:^{
                             
                             CGSize offset;
                             offset.width = 0;
                             offset.height = (1.0f-level)*3 + level*-3;
                             self.uiCorrectnessCircle.layer.shadowOffset = offset;                             
                             
                             float r = (1.0f-level)*rgb_match[0] + level*rgb_wrong[0];
                             float g = (1.0f-level)*rgb_match[1] + level* rgb_wrong[1];                             
                             float b = (1.0f-level)*rgb_match[2] + level* rgb_wrong[2];                                                          
                             
                             UIColor * color = [UIColor colorWithRed:r
                                                               green:g 
                                                                blue:b 
                                                               alpha:1.0];
                             
                             self.uiCorrectnessCircle.layer.backgroundColor = 
                                [color CGColor];
                             
                             self.uiCorrectnessCircle.frame = new_frame;
                         }
                         completion:^(BOOL finished){
                         
                             if (finished) {
                                 if (level == 0) {
                                     [alertWon_ show];
                                     return;
                                 }
                                 
                                 numberOfTriesLeft_--;

                                 if (numberOfTriesLeft_ >= 0) {
                                     
                                     NSString * str = [NSString stringWithFormat:@"# Tries left: %d", 
                                                       numberOfTriesLeft_];
                                     
                                     [[self uiNumberOfTriesLeftLabel] setText:str];        
                                     
                                 } else {
                                     
                                     // loose scenario
                                     
                                     [alertLost_ show];
                                     
                                 }
                             }

                         }];
    }
}
@end
